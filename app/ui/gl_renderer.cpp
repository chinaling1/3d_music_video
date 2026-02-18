/**
 * @file gl_renderer.cpp
 * @brief OpenGL渲染器实现 - 使用OpenGL 1.1基本功能
 */

#include "gl_renderer.h"
#include <fstream>
#include <sstream>

namespace v3d {
namespace app {

glm::mat4 GLCamera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 GLCamera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

GLRenderer::GLRenderer()
    : hWnd_(nullptr)
    , hDC_(nullptr)
    , hGLRC_(nullptr)
    , initialized_(false)
    , width_(0)
    , height_(0)
    , wireframe_(false)
{
    camera_.position = glm::vec3(5, 5, 5);
    camera_.target = glm::vec3(0, 0, 0);
    camera_.up = glm::vec3(0, 1, 0);
    camera_.fov = 45.0f;
    camera_.aspectRatio = 16.0f / 9.0f;
    camera_.nearPlane = 0.1f;
    camera_.farPlane = 1000.0f;
    
    clearColor_ = glm::vec4(0.15f, 0.15f, 0.18f, 1.0f);
}

GLRenderer::~GLRenderer() {
    shutdown();
}

bool GLRenderer::initialize(HWND hWnd) {
    hWnd_ = hWnd;
    
    if (!createGLContext()) {
        return false;
    }
    
    setupDefaultState();
    
    initialized_ = true;
    return true;
}

void GLRenderer::shutdown() {
    if (!initialized_) return;
    destroyGLContext();
    initialized_ = false;
}

bool GLRenderer::createGLContext() {
    hDC_ = GetDC(hWnd_);
    if (!hDC_) return false;
    
    if (!setupPixelFormat()) {
        ReleaseDC(hWnd_, hDC_);
        return false;
    }
    
    hGLRC_ = wglCreateContext(hDC_);
    if (!hGLRC_) {
        ReleaseDC(hWnd_, hDC_);
        return false;
    }
    
    if (!wglMakeCurrent(hDC_, hGLRC_)) {
        wglDeleteContext(hGLRC_);
        ReleaseDC(hWnd_, hDC_);
        return false;
    }
    
    return true;
}

void GLRenderer::destroyGLContext() {
    if (hGLRC_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(hGLRC_);
        hGLRC_ = nullptr;
    }
    if (hDC_) {
        ReleaseDC(hWnd_, hDC_);
        hDC_ = nullptr;
    }
}

bool GLRenderer::setupPixelFormat() {
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int pixelFormat = ChoosePixelFormat(hDC_, &pfd);
    if (pixelFormat == 0) return false;
    
    if (!SetPixelFormat(hDC_, pixelFormat, &pfd)) return false;
    
    return true;
}

void GLRenderer::setupDefaultState() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    GLfloat lightPos[] = {5.0f, 10.0f, 5.0f, 1.0f};
    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
}

void GLRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    camera_.aspectRatio = (height > 0) ? (float)width / height : 1.0f;
    glViewport(0, 0, width, height);
}

void GLRenderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glm::mat4 proj = camera_.getProjectionMatrix();
    glLoadMatrixf(&proj[0][0]);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glm::mat4 view = camera_.getViewMatrix();
    glLoadMatrixf(&view[0][0]);
}

void GLRenderer::endFrame() {
    SwapBuffers(hDC_);
}

void GLRenderer::render() {
    beginFrame();
    drawGrid(20.0f, 20, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
    drawAxes(2.0f);
    endFrame();
}

void GLRenderer::setClearColor(const glm::vec4& color) {
    clearColor_ = color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void GLRenderer::setCamera(const GLCamera& camera) {
    camera_ = camera;
}

void GLRenderer::addLight(const GLLight& light) {
    lights_.push_back(light);
}

void GLRenderer::clearLights() {
    lights_.clear();
}

GLuint GLRenderer::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
    return 0;
}

void GLRenderer::deleteShader(GLuint program) {
}

GLMesh GLRenderer::createMesh(const std::vector<float>& vertices,
                               const std::vector<float>& normals,
                               const std::vector<float>& uvs,
                               const std::vector<unsigned int>& indices) {
    GLMesh mesh;
    mesh.indexCount = (int)indices.size();
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.uvs = uvs;
    mesh.indices = indices;
    mesh.minBounds = glm::vec3(0);
    mesh.maxBounds = glm::vec3(0);
    
    return mesh;
}

void GLRenderer::deleteMesh(GLMesh& mesh) {
    mesh.vertices.clear();
    mesh.normals.clear();
    mesh.uvs.clear();
    mesh.indices.clear();
    mesh.indexCount = 0;
}

GLuint GLRenderer::createTexture(int width, int height, const unsigned char* data, bool hasAlpha) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return texture;
}

void GLRenderer::deleteTexture(GLuint texture) {
    if (texture) glDeleteTextures(1, &texture);
}

void GLRenderer::drawMesh(const GLMesh& mesh, const GLMaterial& material, const glm::mat4& transform) {
    glPushMatrix();
    glMultMatrixf(&transform[0][0]);
    
    GLfloat matDiffuse[] = {material.diffuse.r, material.diffuse.g, material.diffuse.b, 1.0f};
    GLfloat matSpecular[] = {material.specular.r, material.specular.g, material.specular.b, 1.0f};
    GLfloat matAmbient[] = {material.ambient.r, material.ambient.g, material.ambient.b, 1.0f};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);
    
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < mesh.indices.size(); i++) {
        unsigned int idx = mesh.indices[i];
        
        if (!mesh.normals.empty() && idx * 3 + 2 < mesh.normals.size()) {
            glNormal3f(mesh.normals[idx * 3], mesh.normals[idx * 3 + 1], mesh.normals[idx * 3 + 2]);
        }
        
        if (!mesh.uvs.empty() && idx * 2 + 1 < mesh.uvs.size()) {
            glTexCoord2f(mesh.uvs[idx * 2], mesh.uvs[idx * 2 + 1]);
        }
        
        if (idx * 3 + 2 < mesh.vertices.size()) {
            glVertex3f(mesh.vertices[idx * 3], mesh.vertices[idx * 3 + 1], mesh.vertices[idx * 3 + 2]);
        }
    }
    glEnd();
    
    glPopMatrix();
}

void GLRenderer::drawGrid(float size, int divisions, const glm::vec4& color) {
    glDisable(GL_LIGHTING);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINES);
    float step = size / divisions;
    float half = size / 2.0f;
    
    for (int i = 0; i <= divisions; i++) {
        float pos = -half + i * step;
        glVertex3f(pos, 0, -half);
        glVertex3f(pos, 0, half);
        glVertex3f(-half, 0, pos);
        glVertex3f(half, 0, pos);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void GLRenderer::drawAxes(float length) {
    glDisable(GL_LIGHTING);
    
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(length, 0, 0);
    
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, length, 0);
    
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, length);
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void GLRenderer::drawBoundingBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color) {
    glDisable(GL_LIGHTING);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINES);
    glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z);
    glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z);
    glVertex3f(max.x, max.y, min.z); glVertex3f(min.x, max.y, min.z);
    glVertex3f(min.x, max.y, min.z); glVertex3f(min.x, min.y, min.z);
    
    glVertex3f(min.x, min.y, max.z); glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z);
    glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z);
    glVertex3f(min.x, max.y, max.z); glVertex3f(min.x, min.y, max.z);
    
    glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, min.y, max.z);
    glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z);
    glVertex3f(min.x, max.y, min.z); glVertex3f(min.x, max.y, max.z);
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void GLRenderer::enableWireframe(bool enable) {
    wireframe_ = enable;
    glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
}

void GLRenderer::enableDepthTest(bool enable) {
    if (enable) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}

void GLRenderer::enableBackfaceCulling(bool enable) {
    if (enable) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

}
}
