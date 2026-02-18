/**
 * @file panel_3d_view.cpp
 * @brief 3D视图面板实现 - OpenGL渲染和鼠标交互
 */

#include "panel_3d_view.h"
#include "../../src/io/file_loader.h"
#include <windowsx.h>
#include <cmath>
#include <sstream>

namespace v3d {
namespace app {

const wchar_t* Panel3DView::CLASS_NAME = L"Panel3DView";

Panel3DView::Panel3DView(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , gridVisible_(true)
    , axisVisible_(true)
    , lastMouseX_(0)
    , lastMouseY_(0)
    , cameraDistance_(10.0f)
    , cameraYaw_(45.0f)
    , cameraPitch_(30.0f)
    , cameraTarget_(0.0f)
    , wireframe_(false)
    , backfaceCulling_(true)
{
    mouseDown_[0] = mouseDown_[1] = mouseDown_[2] = false;
    
    registerClass();
    createWindow();
    
    renderer_ = std::make_unique<GLRenderer>();
    if (renderer_->initialize(hWnd_)) {
        renderer_->addLight({glm::vec3(5, 10, 5), glm::vec3(1, 1, 1), 1.0f, 0});
        updateCamera();
    }
}

Panel3DView::~Panel3DView() {
    meshes_.clear();
    materials_.clear();
    renderer_.reset();
    
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

void Panel3DView::registerClass() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = panelProc;
    wcex.hInstance = hInstance_;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);
}

bool Panel3DView::createWindow() {
    hWnd_ = CreateWindowExW(
        0,
        CLASS_NAME,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 100, 100,
        hParent_,
        nullptr,
        hInstance_,
        this
    );
    
    return hWnd_ != nullptr;
}

void Panel3DView::render() {
    if (renderer_ && renderer_->isInitialized()) {
        renderer_->beginFrame();
        
        if (gridVisible_) {
            renderer_->drawGrid(20.0f, 20, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        }
        
        if (axisVisible_) {
            renderer_->drawAxes(2.0f);
        }
        
        for (size_t i = 0; i < meshes_.size(); i++) {
            if (i < materials_.size() && i < transforms_.size()) {
                renderer_->drawMesh(meshes_[i], materials_[i], transforms_[i]);
            }
        }
        
        renderer_->endFrame();
    } else {
        InvalidateRect(hWnd_, nullptr, FALSE);
    }
}

void Panel3DView::setGridVisible(bool visible) {
    gridVisible_ = visible;
    render();
}

void Panel3DView::setAxisVisible(bool visible) {
    axisVisible_ = visible;
    render();
}

void Panel3DView::loadModel(const std::string& filePath) {
    if (!renderer_ || !renderer_->isInitialized()) return;
    
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> indices;
    
    glm::vec3 minBounds(0), maxBounds(0);
    
    if (filePath.find(".obj") != std::string::npos) {
        std::string content = v3d::io::FileLoader::readTextFile(filePath);
        
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> norm;
        std::vector<glm::vec2> tex;
        
        std::istringstream stream(content);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            
            if (type == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                positions.push_back(glm::vec3(x, y, z));
            } else if (type == "vn") {
                float x, y, z;
                iss >> x >> y >> z;
                norm.push_back(glm::vec3(x, y, z));
            } else if (type == "vt") {
                float u, v;
                iss >> u >> v;
                tex.push_back(glm::vec2(u, v));
            } else if (type == "f") {
                std::string vertex;
                int faceIndices[3] = {0};
                int normalIndices[3] = {0};
                int uvIndices[3] = {0};
                int count = 0;
                
                while (iss >> vertex && count < 3) {
                    size_t pos1 = vertex.find('/');
                    size_t pos2 = vertex.find('/', pos1 + 1);
                    
                    faceIndices[count] = std::stoi(vertex.substr(0, pos1)) - 1;
                    
                    if (pos1 != std::string::npos && pos1 + 1 < vertex.size()) {
                        if (pos2 != std::string::npos) {
                            if (pos2 > pos1 + 1) {
                                uvIndices[count] = std::stoi(vertex.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                            }
                            normalIndices[count] = std::stoi(vertex.substr(pos2 + 1)) - 1;
                        }
                    }
                    count++;
                }
                
                for (int i = 0; i < 3; i++) {
                    if (faceIndices[i] >= 0 && faceIndices[i] < (int)positions.size()) {
                        vertices.push_back(positions[faceIndices[i]].x);
                        vertices.push_back(positions[faceIndices[i]].y);
                        vertices.push_back(positions[faceIndices[i]].z);
                    }
                    if (normalIndices[i] >= 0 && normalIndices[i] < (int)norm.size()) {
                        normals.push_back(norm[normalIndices[i]].x);
                        normals.push_back(norm[normalIndices[i]].y);
                        normals.push_back(norm[normalIndices[i]].z);
                    }
                    if (uvIndices[i] >= 0 && uvIndices[i] < (int)tex.size()) {
                        uvs.push_back(tex[uvIndices[i]].x);
                        uvs.push_back(tex[uvIndices[i]].y);
                    }
                    indices.push_back(static_cast<unsigned int>(indices.size()));
                }
            }
        }
    }
    
    if (!vertices.empty()) {
        GLMesh mesh = renderer_->createMesh(vertices, normals, uvs, indices);
        meshes_.push_back(mesh);
        
        GLMaterial material;
        material.diffuse = glm::vec3(0.7f, 0.7f, 0.8f);
        material.specular = glm::vec3(0.3f, 0.3f, 0.3f);
        material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
        material.shininess = 32.0f;
        materials_.push_back(material);
        
        transforms_.push_back(glm::mat4(1.0f));
        
        render();
    }
}

void Panel3DView::clearScene() {
    for (auto& mesh : meshes_) {
        renderer_->deleteMesh(mesh);
    }
    meshes_.clear();
    materials_.clear();
    transforms_.clear();
    render();
}

void Panel3DView::resetCamera() {
    cameraDistance_ = 10.0f;
    cameraYaw_ = 45.0f;
    cameraPitch_ = 30.0f;
    cameraTarget_ = glm::vec3(0.0f);
    updateCamera();
}

void Panel3DView::setCameraPosition(const glm::vec3& pos) {
    glm::vec3 dir = pos - cameraTarget_;
    cameraDistance_ = glm::length(dir);
    cameraYaw_ = glm::degrees(atan2(dir.x, dir.z));
    cameraPitch_ = glm::degrees(asin(dir.y / cameraDistance_));
    updateCamera();
}

void Panel3DView::setCameraTarget(const glm::vec3& target) {
    cameraTarget_ = target;
    updateCamera();
}

void Panel3DView::setWireframe(bool enabled) {
    wireframe_ = enabled;
    if (renderer_) {
        renderer_->enableWireframe(enabled);
    }
    render();
}

void Panel3DView::setBackfaceCulling(bool enabled) {
    backfaceCulling_ = enabled;
    if (renderer_) {
        renderer_->enableBackfaceCulling(enabled);
    }
    render();
}

void Panel3DView::updateCamera() {
    if (!renderer_) return;
    
    float yawRad = glm::radians(cameraYaw_);
    float pitchRad = glm::radians(cameraPitch_);
    
    glm::vec3 position;
    position.x = cameraTarget_.x + cameraDistance_ * cos(pitchRad) * sin(yawRad);
    position.y = cameraTarget_.y + cameraDistance_ * sin(pitchRad);
    position.z = cameraTarget_.z + cameraDistance_ * cos(pitchRad) * cos(yawRad);
    
    GLCamera camera;
    camera.position = position;
    camera.target = cameraTarget_;
    camera.up = glm::vec3(0, 1, 0);
    camera.fov = 45.0f;
    camera.aspectRatio = renderer_->getWidth() > 0 ? 
        (float)renderer_->getWidth() / renderer_->getHeight() : 16.0f / 9.0f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;
    
    renderer_->setCamera(camera);
}

void Panel3DView::orbitCamera(float dx, float dy) {
    cameraYaw_ += dx * 0.5f;
    cameraPitch_ -= dy * 0.5f;
    
    if (cameraPitch_ > 89.0f) cameraPitch_ = 89.0f;
    if (cameraPitch_ < -89.0f) cameraPitch_ = -89.0f;
    
    updateCamera();
    render();
}

void Panel3DView::panCamera(float dx, float dy) {
    float yawRad = glm::radians(cameraYaw_);
    
    glm::vec3 right(sin(yawRad + 1.57f), 0, cos(yawRad + 1.57f));
    glm::vec3 forward(sin(yawRad), 0, cos(yawRad));
    
    float panSpeed = cameraDistance_ * 0.002f;
    cameraTarget_ -= right * dx * panSpeed;
    cameraTarget_ += forward * dy * panSpeed;
    
    updateCamera();
    render();
}

void Panel3DView::zoomCamera(float delta) {
    cameraDistance_ -= delta * 0.01f * cameraDistance_;
    
    if (cameraDistance_ < 0.1f) cameraDistance_ = 0.1f;
    if (cameraDistance_ > 1000.0f) cameraDistance_ = 1000.0f;
    
    updateCamera();
    render();
}

void Panel3DView::onPaint() {
    if (renderer_ && renderer_->isInitialized()) {
        render();
    } else {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd_, &ps);
        
        RECT rc;
        GetClientRect(hWnd_, &rc);
        PatBlt(hdc, 0, 0, rc.right, rc.bottom, BLACKNESS);
        
        SetTextColor(hdc, RGB(200, 200, 200));
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, 10, 10, L"3D视图 (OpenGL未初始化)", 22);
        
        EndPaint(hWnd_, &ps);
    }
}

void Panel3DView::onSize(int width, int height) {
    if (renderer_ && renderer_->isInitialized()) {
        renderer_->resize(width, height);
        updateCamera();
        render();
    }
}

void Panel3DView::onMouseDown(int button, int x, int y) {
    if (button >= 0 && button < 3) {
        mouseDown_[button] = true;
    }
    lastMouseX_ = x;
    lastMouseY_ = y;
    SetCapture(hWnd_);
    SetFocus(hWnd_);
}

void Panel3DView::onMouseUp(int button, int x, int y) {
    if (button >= 0 && button < 3) {
        mouseDown_[button] = false;
    }
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    if (!mouseDown_[0] && !mouseDown_[1] && !mouseDown_[2]) {
        ReleaseCapture();
    }
}

void Panel3DView::onMouseMove(int x, int y) {
    int dx = x - lastMouseX_;
    int dy = y - lastMouseY_;
    
    if (mouseDown_[0]) {
        orbitCamera(static_cast<float>(dx), static_cast<float>(dy));
    } else if (mouseDown_[1]) {
        panCamera(static_cast<float>(-dx), static_cast<float>(-dy));
    } else if (mouseDown_[2]) {
        zoomCamera(static_cast<float>(dy));
    }
    
    lastMouseX_ = x;
    lastMouseY_ = y;
}

void Panel3DView::onMouseWheel(int delta) {
    zoomCamera(static_cast<float>(-delta / 120.0f));
}

void Panel3DView::onKeyDown(int key) {
    switch (key) {
        case 'R':
            resetCamera();
            break;
        case 'G':
            setGridVisible(!gridVisible_);
            break;
        case 'A':
            setAxisVisible(!axisVisible_);
            break;
        case 'W':
            setWireframe(!wireframe_);
            break;
    }
}

LRESULT CALLBACK Panel3DView::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Panel3DView* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<Panel3DView*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Panel3DView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    switch (message) {
        case WM_PAINT:
            if (pThis) pThis->onPaint();
            return 0;
            
        case WM_SIZE:
            if (pThis) pThis->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_LBUTTONDOWN:
            if (pThis) pThis->onMouseDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_MBUTTONDOWN:
            if (pThis) pThis->onMouseDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_RBUTTONDOWN:
            if (pThis) pThis->onMouseDown(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_LBUTTONUP:
            if (pThis) pThis->onMouseUp(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_MBUTTONUP:
            if (pThis) pThis->onMouseUp(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_RBUTTONUP:
            if (pThis) pThis->onMouseUp(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_MOUSEMOVE:
            if (pThis) pThis->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_MOUSEWHEEL:
            if (pThis) pThis->onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
            
        case WM_KEYDOWN:
            if (pThis) pThis->onKeyDown((int)wParam);
            return 0;
            
        case WM_ERASEBKGND:
            return 1;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

}
}
