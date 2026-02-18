/**
 * @file mesh.h
 * @brief 3D网格系统 - 提供网格数据的存储和操作功能
 * 
 * 该模块实现了完整的3D网格处理系统，包括：
 * - Vertex: 顶点结构
 * - Edge: 边结构
 * - Face: 面结构
 * - Mesh: 网格类
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace v3d {
namespace modeling {

/**
 * @struct Vertex
 * @brief 顶点结构 - 存储顶点的所有属性
 * 
 * 包含位置、法线、纹理坐标、切线、副切线、颜色和骨骼权重。
 * 
 * @example
 * @code
 * Vertex v;
 * v.position = glm::vec3(0, 1, 0);
 * v.normal = glm::vec3(0, 0, 1);
 * v.texCoord = glm::vec2(0.5f, 0.5f);
 * v.color = glm::vec4(1, 0, 0, 1);
 * @endcode
 */
struct Vertex {
    glm::vec3 position{0.0f};      ///< 位置
    glm::vec3 normal{0.0f, 0.0f, 1.0f};  ///< 法线
    glm::vec2 texCoord{0.0f};      ///< 纹理坐标
    glm::vec3 tangent{0.0f};       ///< 切线
    glm::vec3 bitangent{0.0f};    ///< 副切线
    glm::vec4 color{1.0f};        ///< 顶点颜色
    std::vector<int> boneIndices;   ///< 骨骼索引
    std::vector<float> boneWeights; ///< 骨骼权重

    Vertex() = default;
    
    /**
     * @brief 从位置构造顶点
     * @param pos 位置
     */
    Vertex(const glm::vec3& pos) : position(pos) {}
    
    /**
     * @brief 从位置、法线和纹理坐标构造顶点
     */
    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
        : position(pos), normal(norm), texCoord(uv) {}

    /**
     * @brief 比较两个顶点是否相等
     */
    bool operator==(const Vertex& other) const;
    
    /**
     * @brief 计算顶点的哈希值
     * @return 哈希值
     */
    size_t hash() const;
};

/**
 * @struct Edge
 * @brief 边结构 - 存储边的连接信息
 * 
 * 包含两个顶点索引和相邻面索引。
 */
struct Edge {
    int vertex1; ///< 第一个顶点索引
    int vertex2; ///< 第二个顶点索引
    int face1;   ///< 第一个相邻面索引
    int face2;   ///< 第二个相邻面索引（-1表示边界）

    Edge() : vertex1(-1), vertex2(-1), face1(-1), face2(-1) {}
    
    /**
     * @brief 从两个顶点构造边
     */
    Edge(int v1, int v2) : vertex1(v1), vertex2(v2), face1(-1), face2(-1) {}

    bool operator==(const Edge& other) const;
    
    /**
     * @brief 检查是否为边界边
     * @return 如果是边界边返回true
     */
    bool isBoundary() const { return face2 == -1; }
    
    size_t hash() const;
};

/**
 * @struct Face
 * @brief 面结构 - 存储面的顶点和属性
 * 
 * 支持任意多边形面。
 */
struct Face {
    std::vector<int> vertices; ///< 顶点索引列表
    std::vector<int> edges;    ///< 边索引列表
    glm::vec3 normal;          ///< 面法线
    glm::vec3 centroid;        ///< 面中心
    float area;                ///< 面面积

    Face() = default;
    
    /**
     * @brief 从顶点列表构造面
     */
    explicit Face(const std::vector<int>& verts) : vertices(verts) {}

    /**
     * @brief 计算面法线
     * @param vertexData 顶点数据
     */
    void calculateNormal(const std::vector<Vertex>& vertexData);
    
    /**
     * @brief 计算面中心
     */
    void calculateCentroid(const std::vector<Vertex>& vertexData);
    
    /**
     * @brief 计算面面积
     */
    void calculateArea(const std::vector<Vertex>& vertexData);
    
    /**
     * @brief 检查是否包含指定顶点
     */
    bool containsVertex(int vertexIndex) const;
    
    /**
     * @brief 检查是否包含指定边
     */
    bool containsEdge(int edgeIndex) const;
};

/**
 * @class Mesh
 * @brief 网格类 - 管理3D网格数据
 * 
 * 提供完整的网格操作功能，包括：
 * - 顶点、边、面的增删改查
 * - 法线和切线计算
 * - 变换操作
 * - 网格优化和清理
 * 
 * @example
 * @code
 * Mesh mesh;
 * 
 * // 创建一个三角形
 * int v0 = mesh.addVertex(glm::vec3(0, 0, 0));
 * int v1 = mesh.addVertex(glm::vec3(1, 0, 0));
 * int v2 = mesh.addVertex(glm::vec3(0, 1, 0));
 * 
 * mesh.addTriangle(v0, v1, v2);
 * 
 * // 计算法线
 * mesh.calculateNormals();
 * 
 * // 变换
 * mesh.translate(glm::vec3(0, 0, 1));
 * mesh.scale(glm::vec3(2.0f));
 * 
 * // 获取边界框
 * glm::vec3 min = mesh.getMin();
 * glm::vec3 max = mesh.getMax();
 * @endcode
 */
class Mesh {
public:
    Mesh();
    ~Mesh();

    /**
     * @brief 清空网格
     */
    void clear();

    /**
     * @brief 添加顶点
     * @param vertex 顶点数据
     * @return 顶点索引
     */
    int addVertex(const Vertex& vertex);
    
    /**
     * @brief 添加顶点（仅位置）
     */
    int addVertex(const glm::vec3& position);
    
    /**
     * @brief 获取顶点
     */
    Vertex& getVertex(int index);
    const Vertex& getVertex(int index) const;
    
    /**
     * @brief 设置顶点
     */
    void setVertex(int index, const Vertex& vertex);
    
    /**
     * @brief 移除顶点
     */
    void removeVertex(int index);

    /**
     * @brief 添加面
     * @param vertexIndices 顶点索引列表
     * @return 面索引
     */
    int addFace(const std::vector<int>& vertexIndices);
    
    /**
     * @brief 添加三角形
     */
    int addTriangle(int v0, int v1, int v2);
    
    /**
     * @brief 添加四边形
     */
    int addQuad(int v0, int v1, int v2, int v3);
    
    Face& getFace(int index);
    const Face& getFace(int index) const;
    void removeFace(int index);

    /**
     * @brief 添加边
     */
    int addEdge(int v1, int v2);
    Edge& getEdge(int index);
    const Edge& getEdge(int index) const;
    void removeEdge(int index);

    /**
     * @brief 获取元素数量
     */
    int getVertexCount() const;
    int getFaceCount() const;
    int getEdgeCount() const;

    /**
     * @brief 获取元素列表
     */
    const std::vector<Vertex>& getVertices() const;
    const std::vector<Face>& getFaces() const;
    const std::vector<Edge>& getEdges() const;

    /**
     * @brief 计算法线
     */
    void calculateNormals();
    
    /**
     * @brief 计算切线
     */
    void calculateTangents();
    
    /**
     * @brief 计算边界框
     */
    void calculateBoundingBox();
    
    /**
     * @brief 重新计算所有属性
     */
    void recalculateAll();

    /**
     * @brief 获取边界框信息
     */
    glm::vec3 getCenter() const;
    glm::vec3 getSize() const;
    glm::vec3 getMin() const;
    glm::vec3 getMax() const;

    /**
     * @brief 变换操作
     */
    void translate(const glm::vec3& offset);
    void rotate(float angle, const glm::vec3& axis);
    void scale(const glm::vec3& scale);
    void transform(const glm::mat4& matrix);

    /**
     * @brief 合并另一个网格
     */
    void merge(const Mesh& other);
    
    /**
     * @brief 分离网格
     */
    Mesh split();

    /**
     * @brief 检查网格是否有效
     */
    bool isValid() const;
    
    /**
     * @brief 验证并修复网格
     */
    void validate();

    /**
     * @brief 获取/设置名称
     */
    std::string getName() const;
    void setName(const std::string& name);

    /**
     * @brief 获取/设置材质ID
     */
    void setMaterialId(int materialId);
    int getMaterialId() const;

    /**
     * @brief 设置平滑着色
     */
    void setSmoothShading(bool smooth);
    bool isSmoothShading() const;

    /**
     * @brief 获取相邻元素
     */
    std::vector<int> getAdjacentVertices(int vertexIndex) const;
    std::vector<int> getAdjacentFaces(int vertexIndex) const;
    std::vector<int> getAdjacentEdges(int vertexIndex) const;

    /**
     * @brief 优化网格
     */
    void optimize();
    
    /**
     * @brief 清理网格
     */
    void clean();

    /**
     * @brief 反转法线
     */
    void invertNormals();
    
    /**
     * @brief 翻转面
     */
    void flipFaces();
    
    /**
     * @brief 翻转UV
     */
    void flipUVs();

private:
    void updateEdgeFaceConnections();
    void buildEdgeList();

    std::string name_;                              ///< 网格名称
    std::vector<Vertex> vertices_;                  ///< 顶点列表
    std::vector<Face> faces_;                       ///< 面列表
    std::vector<Edge> edges_;                       ///< 边列表

    std::unordered_map<int, std::vector<int>> vertexToEdges_; ///< 顶点到边的映射
    std::unordered_map<int, std::vector<int>> vertexToFaces_; ///< 顶点到面的映射

    glm::vec3 minBounds_;  ///< 最小边界
    glm::vec3 maxBounds_;  ///< 最大边界
    glm::vec3 center_;     ///< 中心
    glm::vec3 size_;       ///< 尺寸

    int materialId_;       ///< 材质ID
    bool smoothShading_;   ///< 平滑着色标志
    bool needsUpdate_;     ///< 需要更新标志
};

}
}
