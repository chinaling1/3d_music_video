/**
 * @file gl_renderer.h
 * @brief OpenGL渲染器 - 提供硬件加速的3D渲染功能
 */

#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace v3d {
namespace app {

struct GLMesh {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> indices;
    int indexCount;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
};

struct GLMaterial {
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float shininess;
    GLuint diffuseTexture;
    GLuint normalTexture;
};

struct GLCamera {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
};

struct GLLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    int type;
};

class GLRenderer {
public:
    GLRenderer();
    ~GLRenderer();
    
    bool initialize(HWND hWnd);
    void shutdown();
    void resize(int width, int height);
    
    void beginFrame();
    void endFrame();
    void render();
    
    void setClearColor(const glm::vec4& color);
    void setCamera(const GLCamera& camera);
    void addLight(const GLLight& light);
    void clearLights();
    
    GLuint createShader(const std::string& vertexSource, const std::string& fragmentSource);
    void deleteShader(GLuint program);
    
    GLMesh createMesh(const std::vector<float>& vertices, 
                      const std::vector<float>& normals,
                      const std::vector<float>& uvs,
                      const std::vector<unsigned int>& indices);
    void deleteMesh(GLMesh& mesh);
    
    GLuint createTexture(int width, int height, const unsigned char* data, bool hasAlpha);
    void deleteTexture(GLuint texture);
    
    void drawMesh(const GLMesh& mesh, const GLMaterial& material, const glm::mat4& transform);
    
    void drawGrid(float size, int divisions, const glm::vec4& color);
    void drawAxes(float length);
    void drawBoundingBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color);
    
    void enableWireframe(bool enable);
    void enableDepthTest(bool enable);
    void enableBackfaceCulling(bool enable);
    
    HDC getDC() const { return hDC_; }
    HGLRC getGLRC() const { return hGLRC_; }
    
    bool isInitialized() const { return initialized_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    bool createGLContext();
    void destroyGLContext();
    bool setupPixelFormat();
    void setupDefaultState();
    
    HWND hWnd_;
    HDC hDC_;
    HGLRC hGLRC_;
    
    bool initialized_;
    int width_;
    int height_;
    
    GLCamera camera_;
    std::vector<GLLight> lights_;
    glm::vec4 clearColor_;
    
    bool wireframe_;
};

}
}
