#include "mesh.h"
#include <algorithm>
#include <unordered_set>
#include <cmath>

namespace v3d {
namespace modeling {

bool Vertex::operator==(const Vertex& other) const {
    return position == other.position &&
           normal == other.normal &&
           texCoord == other.texCoord &&
           tangent == other.tangent &&
           bitangent == other.bitangent &&
           color == other.color;
}

size_t Vertex::hash() const {
    size_t h = 0;
    h ^= std::hash<float>()(position.x) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>()(position.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>()(position.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

bool Edge::operator==(const Edge& other) const {
    return (vertex1 == other.vertex1 && vertex2 == other.vertex2) ||
           (vertex1 == other.vertex2 && vertex2 == other.vertex1);
}

size_t Edge::hash() const {
    int minV = std::min(vertex1, vertex2);
    int maxV = std::max(vertex1, vertex2);
    size_t h = std::hash<int>()(minV);
    h ^= std::hash<int>()(maxV) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

void Face::calculateNormal(const std::vector<Vertex>& vertexData) {
    if (vertices.size() < 3) {
        normal = glm::vec3(0.0f);
        return;
    }

    const Vertex& v0 = vertexData[vertices[0]];
    const Vertex& v1 = vertexData[vertices[1]];
    const Vertex& v2 = vertexData[vertices[2]];

    glm::vec3 edge1 = v1.position - v0.position;
    glm::vec3 edge2 = v2.position - v0.position;

    normal = glm::normalize(glm::cross(edge1, edge2));
}

void Face::calculateCentroid(const std::vector<Vertex>& vertexData) {
    if (vertices.empty()) {
        centroid = glm::vec3(0.0f);
        return;
    }

    glm::vec3 sum(0.0f);
    for (int vi : vertices) {
        sum += vertexData[vi].position;
    }
    centroid = sum / static_cast<float>(vertices.size());
}

void Face::calculateArea(const std::vector<Vertex>& vertexData) {
    if (vertices.size() < 3) {
        area = 0.0f;
        return;
    }

    const Vertex& v0 = vertexData[vertices[0]];
    const Vertex& v1 = vertexData[vertices[1]];
    const Vertex& v2 = vertexData[vertices[2]];

    glm::vec3 edge1 = v1.position - v0.position;
    glm::vec3 edge2 = v2.position - v0.position;

    area = glm::length(glm::cross(edge1, edge2)) * 0.5f;
}

bool Face::containsVertex(int vertexIndex) const {
    return std::find(vertices.begin(), vertices.end(), vertexIndex) != vertices.end();
}

bool Face::containsEdge(int edgeIndex) const {
    return std::find(edges.begin(), edges.end(), edgeIndex) != edges.end();
}

Mesh::Mesh()
    : materialId_(-1)
    , smoothShading_(true)
    , needsUpdate_(true) {
    minBounds_ = glm::vec3(std::numeric_limits<float>::max());
    maxBounds_ = glm::vec3(std::numeric_limits<float>::lowest());
}

Mesh::~Mesh() {
}

void Mesh::clear() {
    vertices_.clear();
    faces_.clear();
    edges_.clear();
    vertexToEdges_.clear();
    vertexToFaces_.clear();
    needsUpdate_ = true;
}

int Mesh::addVertex(const Vertex& vertex) {
    vertices_.push_back(vertex);
    needsUpdate_ = true;
    return static_cast<int>(vertices_.size()) - 1;
}

int Mesh::addVertex(const glm::vec3& position) {
    return addVertex(Vertex(position));
}

Vertex& Mesh::getVertex(int index) {
    return vertices_[index];
}

const Vertex& Mesh::getVertex(int index) const {
    return vertices_[index];
}

void Mesh::setVertex(int index, const Vertex& vertex) {
    vertices_[index] = vertex;
    needsUpdate_ = true;
}

void Mesh::removeVertex(int index) {
    if (index < 0 || index >= static_cast<int>(vertices_.size())) {
        return;
    }

    vertices_.erase(vertices_.begin() + index);

    for (auto& face : faces_) {
        auto it = std::find(face.vertices.begin(), face.vertices.end(), index);
        if (it != face.vertices.end()) {
            face.vertices.erase(it);
        }
        for (int& vi : face.vertices) {
            if (vi > index) {
                vi--;
            }
        }
    }

    vertexToEdges_.erase(index);
    vertexToFaces_.erase(index);

    needsUpdate_ = true;
}

int Mesh::addFace(const std::vector<int>& vertexIndices) {
    Face face(vertexIndices);
    face.calculateNormal(vertices_);
    face.calculateCentroid(vertices_);
    face.calculateArea(vertices_);

    int faceIndex = static_cast<int>(faces_.size());
    faces_.push_back(face);

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        int v1 = vertexIndices[i];
        int v2 = vertexIndices[(i + 1) % vertexIndices.size()];

        int edgeIndex = addEdge(v1, v2);
        face.edges.push_back(edgeIndex);

        Edge& edge = edges_[edgeIndex];
        if (edge.face1 == -1) {
            edge.face1 = faceIndex;
        } else if (edge.face2 == -1) {
            edge.face2 = faceIndex;
        }

        vertexToFaces_[v1].push_back(faceIndex);
    }

    needsUpdate_ = true;
    return faceIndex;
}

int Mesh::addTriangle(int v0, int v1, int v2) {
    return addFace({v0, v1, v2});
}

int Mesh::addQuad(int v0, int v1, int v2, int v3) {
    return addFace({v0, v1, v2, v3});
}

Face& Mesh::getFace(int index) {
    return faces_[index];
}

const Face& Mesh::getFace(int index) const {
    return faces_[index];
}

void Mesh::removeFace(int index) {
    if (index < 0 || index >= static_cast<int>(faces_.size())) {
        return;
    }

    Face& face = faces_[index];

    for (int edgeIndex : face.edges) {
        Edge& edge = edges_[edgeIndex];
        if (edge.face1 == index) {
            edge.face1 = edge.face2;
            edge.face2 = -1;
        } else if (edge.face2 == index) {
            edge.face2 = -1;
        }
    }

    for (int vertexIndex : face.vertices) {
        auto& faces = vertexToFaces_[vertexIndex];
        faces.erase(std::remove(faces.begin(), faces.end(), index), faces.end());
    }

    faces_.erase(faces_.begin() + index);
    needsUpdate_ = true;
}

int Mesh::addEdge(int v1, int v2) {
    Edge newEdge(v1, v2);

    for (size_t i = 0; i < edges_.size(); ++i) {
        if (edges_[i] == newEdge) {
            vertexToEdges_[v1].push_back(static_cast<int>(i));
            vertexToEdges_[v2].push_back(static_cast<int>(i));
            return static_cast<int>(i);
        }
    }

    int edgeIndex = static_cast<int>(edges_.size());
    edges_.push_back(newEdge);
    vertexToEdges_[v1].push_back(edgeIndex);
    vertexToEdges_[v2].push_back(edgeIndex);

    return edgeIndex;
}

Edge& Mesh::getEdge(int index) {
    return edges_[index];
}

const Edge& Mesh::getEdge(int index) const {
    return edges_[index];
}

void Mesh::removeEdge(int index) {
    if (index < 0 || index >= static_cast<int>(edges_.size())) {
        return;
    }

    Edge& edge = edges_[index];

    for (int vi : {edge.vertex1, edge.vertex2}) {
        auto& edges = vertexToEdges_[vi];
        edges.erase(std::remove(edges.begin(), edges.end(), index), edges.end());
    }

    edges_.erase(edges_.begin() + index);
    needsUpdate_ = true;
}

int Mesh::getVertexCount() const {
    return static_cast<int>(vertices_.size());
}

int Mesh::getFaceCount() const {
    return static_cast<int>(faces_.size());
}

int Mesh::getEdgeCount() const {
    return static_cast<int>(edges_.size());
}

const std::vector<Vertex>& Mesh::getVertices() const {
    return vertices_;
}

const std::vector<Face>& Mesh::getFaces() const {
    return faces_;
}

const std::vector<Edge>& Mesh::getEdges() const {
    return edges_;
}

void Mesh::calculateNormals() {
    for (auto& vertex : vertices_) {
        vertex.normal = glm::vec3(0.0f);
    }

    for (const auto& face : faces_) {
        glm::vec3 faceNormal = face.normal;
        float faceArea = face.area;

        for (int vertexIndex : face.vertices) {
            vertices_[vertexIndex].normal += faceNormal * faceArea;
        }
    }

    for (auto& vertex : vertices_) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

void Mesh::calculateTangents() {
    for (auto& vertex : vertices_) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }

    for (const auto& face : faces_) {
        if (face.vertices.size() < 3) continue;

        const Vertex& v0 = vertices_[face.vertices[0]];
        const Vertex& v1 = vertices_[face.vertices[1]];
        const Vertex& v2 = vertices_[face.vertices[2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        for (int vertexIndex : face.vertices) {
            vertices_[vertexIndex].tangent += tangent;
            vertices_[vertexIndex].bitangent += bitangent;
        }
    }

    for (auto& vertex : vertices_) {
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::normalize(vertex.bitangent);
    }
}

void Mesh::calculateBoundingBox() {
    if (vertices_.empty()) {
        minBounds_ = glm::vec3(0.0f);
        maxBounds_ = glm::vec3(0.0f);
        center_ = glm::vec3(0.0f);
        size_ = glm::vec3(0.0f);
        return;
    }

    minBounds_ = vertices_[0].position;
    maxBounds_ = vertices_[0].position;

    for (const auto& vertex : vertices_) {
        minBounds_ = glm::min(minBounds_, vertex.position);
        maxBounds_ = glm::max(maxBounds_, vertex.position);
    }

    size_ = maxBounds_ - minBounds_;
    center_ = minBounds_ + size_ * 0.5f;
}

void Mesh::recalculateAll() {
    calculateNormals();
    calculateTangents();
    calculateBoundingBox();
    needsUpdate_ = false;
}

glm::vec3 Mesh::getCenter() const {
    if (needsUpdate_) {
        const_cast<Mesh*>(this)->calculateBoundingBox();
    }
    return center_;
}

glm::vec3 Mesh::getSize() const {
    if (needsUpdate_) {
        const_cast<Mesh*>(this)->calculateBoundingBox();
    }
    return size_;
}

glm::vec3 Mesh::getMin() const {
    if (needsUpdate_) {
        const_cast<Mesh*>(this)->calculateBoundingBox();
    }
    return minBounds_;
}

glm::vec3 Mesh::getMax() const {
    if (needsUpdate_) {
        const_cast<Mesh*>(this)->calculateBoundingBox();
    }
    return maxBounds_;
}

void Mesh::translate(const glm::vec3& offset) {
    for (auto& vertex : vertices_) {
        vertex.position += offset;
    }
    needsUpdate_ = true;
}

void Mesh::rotate(float angle, const glm::vec3& axis) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

    for (auto& vertex : vertices_) {
        glm::vec4 pos = rotation * glm::vec4(vertex.position, 1.0f);
        vertex.position = glm::vec3(pos);

        glm::vec4 norm = rotation * glm::vec4(vertex.normal, 0.0f);
        vertex.normal = glm::normalize(glm::vec3(norm));
    }
    needsUpdate_ = true;
}

void Mesh::scale(const glm::vec3& scale) {
    for (auto& vertex : vertices_) {
        vertex.position *= scale;
    }
    needsUpdate_ = true;
}

void Mesh::transform(const glm::mat4& matrix) {
    for (auto& vertex : vertices_) {
        glm::vec4 pos = matrix * glm::vec4(vertex.position, 1.0f);
        vertex.position = glm::vec3(pos);

        glm::vec4 norm = glm::inverse(glm::transpose(matrix)) * glm::vec4(vertex.normal, 0.0f);
        vertex.normal = glm::normalize(glm::vec3(norm));
    }
    needsUpdate_ = true;
}

void Mesh::merge(const Mesh& other) {
    int vertexOffset = static_cast<int>(vertices_.size());

    for (const auto& vertex : other.vertices_) {
        vertices_.push_back(vertex);
    }

    for (const auto& face : other.faces_) {
        std::vector<int> newVertices;
        for (int vi : face.vertices) {
            newVertices.push_back(vi + vertexOffset);
        }
        addFace(newVertices);
    }

    needsUpdate_ = true;
}

Mesh Mesh::split() {
    Mesh result;
    return result;
}

bool Mesh::isValid() const {
    if (vertices_.empty() || faces_.empty()) {
        return false;
    }

    for (const auto& face : faces_) {
        if (face.vertices.size() < 3) {
            return false;
        }

        for (int vi : face.vertices) {
            if (vi < 0 || vi >= static_cast<int>(vertices_.size())) {
                return false;
            }
        }
    }

    return true;
}

void Mesh::validate() {
    std::unordered_set<int> validVertices;

    for (const auto& face : faces_) {
        for (int vi : face.vertices) {
            validVertices.insert(vi);
        }
    }

    std::vector<Vertex> newVertices;
    std::unordered_map<int, int> vertexRemap;

    for (int vi : validVertices) {
        vertexRemap[vi] = static_cast<int>(newVertices.size());
        newVertices.push_back(vertices_[vi]);
    }

    vertices_ = std::move(newVertices);

    for (auto& face : faces_) {
        for (int& vi : face.vertices) {
            vi = vertexRemap[vi];
        }
    }

    needsUpdate_ = true;
}

std::string Mesh::getName() const {
    return name_;
}

void Mesh::setName(const std::string& name) {
    name_ = name;
}

void Mesh::setMaterialId(int materialId) {
    materialId_ = materialId;
}

int Mesh::getMaterialId() const {
    return materialId_;
}

void Mesh::setSmoothShading(bool smooth) {
    smoothShading_ = smooth;
}

bool Mesh::isSmoothShading() const {
    return smoothShading_;
}

std::vector<int> Mesh::getAdjacentVertices(int vertexIndex) const {
    std::vector<int> adjacent;

    auto it = vertexToEdges_.find(vertexIndex);
    if (it != vertexToEdges_.end()) {
        for (int edgeIndex : it->second) {
            const Edge& edge = edges_[edgeIndex];
            if (edge.vertex1 == vertexIndex) {
                adjacent.push_back(edge.vertex2);
            } else {
                adjacent.push_back(edge.vertex1);
            }
        }
    }

    return adjacent;
}

std::vector<int> Mesh::getAdjacentFaces(int vertexIndex) const {
    auto it = vertexToFaces_.find(vertexIndex);
    return it != vertexToFaces_.end() ? it->second : std::vector<int>();
}

std::vector<int> Mesh::getAdjacentEdges(int vertexIndex) const {
    auto it = vertexToEdges_.find(vertexIndex);
    return it != vertexToEdges_.end() ? it->second : std::vector<int>();
}

void Mesh::optimize() {
    validate();
    clean();
}

void Mesh::clean() {
    std::vector<int> verticesToRemove;

    for (int i = 0; i < static_cast<int>(vertices_.size()); ++i) {
        if (vertexToFaces_.find(i) == vertexToFaces_.end() || vertexToFaces_[i].empty()) {
            verticesToRemove.push_back(i);
        }
    }

    for (int i = static_cast<int>(verticesToRemove.size()) - 1; i >= 0; --i) {
        removeVertex(verticesToRemove[i]);
    }

    std::vector<int> edgesToRemove;

    for (int i = 0; i < static_cast<int>(edges_.size()); ++i) {
        if (edges_[i].face1 == -1 && edges_[i].face2 == -1) {
            edgesToRemove.push_back(i);
        }
    }

    for (int i = static_cast<int>(edgesToRemove.size()) - 1; i >= 0; --i) {
        removeEdge(edgesToRemove[i]);
    }
}

void Mesh::invertNormals() {
    for (auto& vertex : vertices_) {
        vertex.normal = -vertex.normal;
    }

    for (auto& face : faces_) {
        face.normal = -face.normal;
        std::reverse(face.vertices.begin(), face.vertices.end());
    }
}

void Mesh::flipFaces() {
    for (auto& face : faces_) {
        std::reverse(face.vertices.begin(), face.vertices.end());
        face.calculateNormal(vertices_);
    }
}

void Mesh::flipUVs() {
    for (auto& vertex : vertices_) {
        vertex.texCoord.y = 1.0f - vertex.texCoord.y;
    }
}

void Mesh::updateEdgeFaceConnections() {
}

void Mesh::buildEdgeList() {
}

}
}