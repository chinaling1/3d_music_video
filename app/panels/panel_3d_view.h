/**
 * @file panel_3d_view.h
 * @brief 3D视图面板 - 显示3D场景和模型，支持OpenGL渲染和鼠标交互
 */

#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "ui/gl_renderer.h"

namespace v3d {
namespace app {

class Panel3DView {
public:
    Panel3DView(HWND hParent, HINSTANCE hInstance);
    ~Panel3DView();
    
    HWND getHandle() const { return hWnd_; }
    
    void render();
    void setGridVisible(bool visible);
    void setAxisVisible(bool visible);
    
    void loadModel(const std::string& filePath);
    void clearScene();
    
    void resetCamera();
    void setCameraPosition(const glm::vec3& pos);
    void setCameraTarget(const glm::vec3& target);
    
    void setWireframe(bool enabled);
    void setBackfaceCulling(bool enabled);
    
    GLRenderer* getRenderer() { return renderer_.get(); }

private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onPaint();
    void onSize(int width, int height);
    void onMouseDown(int button, int x, int y);
    void onMouseUp(int button, int x, int y);
    void onMouseMove(int x, int y);
    void onMouseWheel(int delta);
    void onKeyDown(int key);
    
    void updateCamera();
    void orbitCamera(float dx, float dy);
    void panCamera(float dx, float dy);
    void zoomCamera(float delta);
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    
    std::unique_ptr<GLRenderer> renderer_;
    
    bool gridVisible_;
    bool axisVisible_;
    
    bool mouseDown_[3];
    int lastMouseX_;
    int lastMouseY_;
    
    float cameraDistance_;
    float cameraYaw_;
    float cameraPitch_;
    glm::vec3 cameraTarget_;
    
    bool wireframe_;
    bool backfaceCulling_;
    
    std::vector<GLMesh> meshes_;
    std::vector<GLMaterial> materials_;
    std::vector<glm::mat4> transforms_;
    
    static const wchar_t* CLASS_NAME;
};

}
}
