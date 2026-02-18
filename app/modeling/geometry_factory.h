/**
 * @file geometry_factory.h
 * @brief 几何体工厂 - 创建基础3D几何体
 * 
 * 提供类似Blender的基础几何体创建功能：
 * - 立方体、球体、圆柱体、圆锥体、圆环等
 * - 可配置的细分级别和参数
 */

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <memory>

namespace v3d {
namespace modeling {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec4 color;
    std::vector<int> boneIndices;
    std::vector<float> boneWeights;
    
    Vertex() : position(0), normal(0, 1, 0), texCoord(0), tangent(1, 0, 0), bitangent(0, 0, 1), color(1) {}
};

struct Edge {
    int v1, v2;
    std::vector<int> adjacentFaces;
    
    Edge(int a, int b) : v1(a), v2(b) {}
};

struct Face {
    std::vector<int> vertices;
    std::vector<int> edges;
    glm::vec3 normal;
    glm::vec3 centroid;
    float area;
    int materialIndex;
    
    Face() : normal(0, 1, 0), area(0), materialIndex(0) {}
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<Face> faces;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    glm::vec3 center;
    float radius;
    
    void calculateNormals();
    void calculateTangents();
    void calculateBounds();
    void calculateFaceData();
    void buildEdgeList();
    
    void translate(const glm::vec3& offset);
    void rotate(const glm::quat& rotation);
    void scale(const glm::vec3& scale);
    void transform(const glm::mat4& matrix);
    
    void merge(const MeshData& other);
    MeshData clone() const;
    
    void flipNormals();
    void flipWinding();
    
    void subdivide(int levels = 1);
    void simplify(float targetRatio);
    
    size_t getVertexCount() const { return vertices.size(); }
    size_t getFaceCount() const { return faces.size(); }
    size_t getEdgeCount() const { return edges.size(); }
};

struct GeometryParams {
    int segments = 32;
    int rings = 16;
    float radius = 1.0f;
    float radius2 = 0.5f;
    float height = 2.0f;
    float width = 2.0f;
    float depth = 2.0f;
    float innerRadius = 0.5f;
    float outerRadius = 1.0f;
    int subdivisions = 1;
    bool generateUVs = true;
    bool generateNormals = true;
    bool generateTangents = true;
};

class GeometryFactory {
public:
    static MeshData createCube(const GeometryParams& params = GeometryParams());
    static MeshData createSphere(const GeometryParams& params = GeometryParams());
    static MeshData createCylinder(const GeometryParams& params = GeometryParams());
    static MeshData createCone(const GeometryParams& params = GeometryParams());
    static MeshData createTorus(const GeometryParams& params = GeometryParams());
    static MeshData createPlane(const GeometryParams& params = GeometryParams());
    static MeshData createCircle(const GeometryParams& params = GeometryParams());
    static MeshData createIcoSphere(const GeometryParams& params = GeometryParams());
    static MeshData createUVSphere(const GeometryParams& params = GeometryParams());
    static MeshData createCapsule(const GeometryParams& params = GeometryParams());
    static MeshData createTorusKnot(const GeometryParams& params = GeometryParams());
    static MeshData createGrid(const GeometryParams& params = GeometryParams());
    static MeshData createMonkey(const GeometryParams& params = GeometryParams());
    
    static MeshData createFromFunction(
        std::function<glm::vec3(float, float)> positionFunc,
        std::function<glm::vec3(float, float)> normalFunc,
        std::function<glm::vec2(float, float)> uvFunc,
        int uSegments, int vSegments
    );

private:
    static void addQuad(MeshData& mesh, int v0, int v1, int v2, int v3);
    static void addTriangle(MeshData& mesh, int v0, int v1, int v2);
    static void addVertex(MeshData& mesh, const Vertex& v);
};

}
}
