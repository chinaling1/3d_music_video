/**
 * @file geometry_factory.cpp
 * @brief 几何体工厂实现
 */

#include "geometry_factory.h"
#include <cmath>
#include <algorithm>

namespace v3d {
namespace modeling {

void MeshData::calculateNormals() {
    for (auto& v : vertices) {
        v.normal = glm::vec3(0);
    }
    
    for (const auto& face : faces) {
        if (face.vertices.size() >= 3) {
            const glm::vec3& v0 = vertices[face.vertices[0]].position;
            const glm::vec3& v1 = vertices[face.vertices[1]].position;
            const glm::vec3& v2 = vertices[face.vertices[2]].position;
            
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
            
            for (int idx : face.vertices) {
                vertices[idx].normal += normal;
            }
        }
    }
    
    for (auto& v : vertices) {
        if (glm::length(v.normal) > 0.0001f) {
            v.normal = glm::normalize(v.normal);
        }
    }
}

void MeshData::calculateTangents() {
    for (auto& v : vertices) {
        v.tangent = glm::vec3(1, 0, 0);
        v.bitangent = glm::vec3(0, 0, 1);
    }
}

void MeshData::calculateBounds() {
    if (vertices.empty()) {
        minBounds = maxBounds = center = glm::vec3(0);
        radius = 0;
        return;
    }
    
    minBounds = maxBounds = vertices[0].position;
    
    for (const auto& v : vertices) {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }
    
    center = (minBounds + maxBounds) * 0.5f;
    
    radius = 0;
    for (const auto& v : vertices) {
        float dist = glm::length(v.position - center);
        radius = std::max(radius, dist);
    }
}

void MeshData::calculateFaceData() {
    for (auto& face : faces) {
        if (face.vertices.size() >= 3) {
            const glm::vec3& v0 = vertices[face.vertices[0]].position;
            const glm::vec3& v1 = vertices[face.vertices[1]].position;
            const glm::vec3& v2 = vertices[face.vertices[2]].position;
            
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            face.normal = glm::normalize(glm::cross(edge1, edge2));
            
            face.centroid = glm::vec3(0);
            for (int idx : face.vertices) {
                face.centroid += vertices[idx].position;
            }
            face.centroid /= static_cast<float>(face.vertices.size());
            
            if (face.vertices.size() == 3) {
                face.area = glm::length(glm::cross(edge1, edge2)) * 0.5f;
            } else if (face.vertices.size() == 4) {
                const glm::vec3& v3 = vertices[face.vertices[3]].position;
                glm::vec3 edge3 = v3 - v0;
                face.area = glm::length(glm::cross(edge1, edge2)) * 0.5f + 
                           glm::length(glm::cross(edge2, edge3)) * 0.5f;
            }
        }
    }
}

void MeshData::buildEdgeList() {
    edges.clear();
    
    for (size_t fi = 0; fi < faces.size(); fi++) {
        const auto& face = faces[fi];
        for (size_t i = 0; i < face.vertices.size(); i++) {
            int v0 = face.vertices[i];
            int v1 = face.vertices[(i + 1) % face.vertices.size()];
            
            bool found = false;
            for (auto& edge : edges) {
                if ((edge.v1 == v0 && edge.v2 == v1) || (edge.v1 == v1 && edge.v2 == v0)) {
                    edge.adjacentFaces.push_back(static_cast<int>(fi));
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                Edge edge(v0, v1);
                edge.adjacentFaces.push_back(static_cast<int>(fi));
                edges.push_back(edge);
            }
        }
    }
}

void MeshData::translate(const glm::vec3& offset) {
    for (auto& v : vertices) {
        v.position += offset;
    }
    calculateBounds();
}

void MeshData::rotate(const glm::quat& rotation) {
    for (auto& v : vertices) {
        v.position = rotation * v.position;
        v.normal = rotation * v.normal;
        v.tangent = rotation * v.tangent;
        v.bitangent = rotation * v.bitangent;
    }
    calculateBounds();
}

void MeshData::scale(const glm::vec3& s) {
    for (auto& v : vertices) {
        v.position *= s;
    }
    calculateBounds();
}

void MeshData::transform(const glm::mat4& matrix) {
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
    for (auto& v : vertices) {
        v.position = glm::vec3(matrix * glm::vec4(v.position, 1.0f));
        v.normal = normalMatrix * v.normal;
    }
    calculateBounds();
}

void MeshData::merge(const MeshData& other) {
    int vertexOffset = static_cast<int>(vertices.size());
    
    for (const auto& v : other.vertices) {
        vertices.push_back(v);
    }
    
    for (auto face : other.faces) {
        for (auto& idx : face.vertices) {
            idx += vertexOffset;
        }
        faces.push_back(face);
    }
    
    buildEdgeList();
    calculateBounds();
}

MeshData MeshData::clone() const {
    MeshData result = *this;
    return result;
}

void MeshData::flipNormals() {
    for (auto& v : vertices) {
        v.normal = -v.normal;
    }
    for (auto& face : faces) {
        face.normal = -face.normal;
    }
}

void MeshData::flipWinding() {
    for (auto& face : faces) {
        std::reverse(face.vertices.begin(), face.vertices.end());
    }
    calculateNormals();
}

void MeshData::subdivide(int levels) {
}

void MeshData::simplify(float targetRatio) {
}

MeshData GeometryFactory::createCube(const GeometryParams& params) {
    MeshData mesh;
    
    float hw = params.width * 0.5f;
    float hh = params.height * 0.5f;
    float hd = params.depth * 0.5f;
    
    glm::vec3 positions[8] = {
        {-hw, -hh, -hd}, {hw, -hh, -hd}, {hw, hh, -hd}, {-hw, hh, -hd},
        {-hw, -hh, hd}, {hw, -hh, hd}, {hw, hh, hd}, {-hw, hh, hd}
    };
    
    glm::vec3 normals[6] = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}
    };
    
    glm::vec2 uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    
    int faceVerts[6][4] = {
        {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
        {1, 5, 6, 2}, {4, 5, 1, 0}, {3, 2, 6, 7}
    };
    
    for (int f = 0; f < 6; f++) {
        int baseIndex = static_cast<int>(mesh.vertices.size());
        
        for (int v = 0; v < 4; v++) {
            Vertex vertex;
            vertex.position = positions[faceVerts[f][v]];
            vertex.normal = normals[f];
            vertex.texCoord = uvs[v];
            vertex.color = glm::vec4(1);
            mesh.vertices.push_back(vertex);
        }
        
        Face face;
        face.vertices = {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3};
        face.normal = normals[f];
        face.materialIndex = 0;
        mesh.faces.push_back(face);
    }
    
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createSphere(const GeometryParams& params) {
    return createUVSphere(params);
}

MeshData GeometryFactory::createUVSphere(const GeometryParams& params) {
    MeshData mesh;
    
    float radius = params.radius;
    int segments = params.segments;
    int rings = params.rings;
    
    for (int ring = 0; ring <= rings; ring++) {
        float phi = glm::pi<float>() * static_cast<float>(ring) / static_cast<float>(rings);
        float y = radius * std::cos(phi);
        float ringRadius = radius * std::sin(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(seg) / static_cast<float>(segments);
            float x = ringRadius * std::cos(theta);
            float z = ringRadius * std::sin(theta);
            
            Vertex v;
            v.position = glm::vec3(x, y, z);
            v.normal = glm::normalize(v.position);
            v.texCoord = glm::vec2(static_cast<float>(seg) / segments, static_cast<float>(ring) / rings);
            v.color = glm::vec4(1);
            mesh.vertices.push_back(v);
        }
    }
    
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int i0 = ring * (segments + 1) + seg;
            int i1 = i0 + 1;
            int i2 = i0 + segments + 1;
            int i3 = i2 + 1;
            
            Face f1, f2;
            f1.vertices = {i0, i2, i1};
            f2.vertices = {i1, i2, i3};
            f1.materialIndex = f2.materialIndex = 0;
            mesh.faces.push_back(f1);
            mesh.faces.push_back(f2);
        }
    }
    
    mesh.calculateNormals();
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createCylinder(const GeometryParams& params) {
    MeshData mesh;
    
    float radius = params.radius;
    float height = params.height;
    int segments = params.segments;
    float halfHeight = height * 0.5f;
    
    int centerTop = static_cast<int>(mesh.vertices.size());
    Vertex vTop;
    vTop.position = glm::vec3(0, halfHeight, 0);
    vTop.normal = glm::vec3(0, 1, 0);
    vTop.texCoord = glm::vec2(0.5f, 0.5f);
    vTop.color = glm::vec4(1);
    mesh.vertices.push_back(vTop);
    
    int centerBottom = static_cast<int>(mesh.vertices.size());
    Vertex vBottom;
    vBottom.position = glm::vec3(0, -halfHeight, 0);
    vBottom.normal = glm::vec3(0, -1, 0);
    vBottom.texCoord = glm::vec2(0.5f, 0.5f);
    vBottom.color = glm::vec4(1);
    mesh.vertices.push_back(vBottom);
    
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        float u = static_cast<float>(i) / segments;
        
        Vertex vTopRing;
        vTopRing.position = glm::vec3(x, halfHeight, z);
        vTopRing.normal = glm::vec3(0, 1, 0);
        vTopRing.texCoord = glm::vec2(u, 1);
        vTopRing.color = glm::vec4(1);
        mesh.vertices.push_back(vTopRing);
        
        Vertex vBottomRing;
        vBottomRing.position = glm::vec3(x, -halfHeight, z);
        vBottomRing.normal = glm::vec3(0, -1, 0);
        vBottomRing.texCoord = glm::vec2(u, 0);
        vBottomRing.color = glm::vec4(1);
        mesh.vertices.push_back(vBottomRing);
    }
    
    for (int i = 0; i < segments; i++) {
        int topIdx = 2 + i * 2;
        int bottomIdx = 3 + i * 2;
        int nextTopIdx = 2 + (i + 1) * 2;
        int nextBottomIdx = 3 + (i + 1) * 2;
        
        Face topFace;
        topFace.vertices = {centerTop, topIdx, nextTopIdx};
        topFace.normal = glm::vec3(0, 1, 0);
        mesh.faces.push_back(topFace);
        
        Face bottomFace;
        bottomFace.vertices = {centerBottom, nextBottomIdx, bottomIdx};
        bottomFace.normal = glm::vec3(0, -1, 0);
        mesh.faces.push_back(bottomFace);
        
        Face sideFace;
        sideFace.vertices = {topIdx, bottomIdx, nextBottomIdx, nextTopIdx};
        float theta2 = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        glm::vec3 sideNormal = glm::normalize(glm::vec3(std::cos(theta2), 0, std::sin(theta2)));
        sideFace.normal = sideNormal;
        mesh.faces.push_back(sideFace);
    }
    
    mesh.calculateNormals();
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createCone(const GeometryParams& params) {
    MeshData mesh;
    
    float radius = params.radius;
    float height = params.height;
    int segments = params.segments;
    float halfHeight = height * 0.5f;
    
    int apexIdx = static_cast<int>(mesh.vertices.size());
    Vertex apex;
    apex.position = glm::vec3(0, halfHeight, 0);
    apex.color = glm::vec4(1);
    mesh.vertices.push_back(apex);
    
    int centerBottom = static_cast<int>(mesh.vertices.size());
    Vertex center;
    center.position = glm::vec3(0, -halfHeight, 0);
    center.normal = glm::vec3(0, -1, 0);
    center.texCoord = glm::vec2(0.5f, 0.5f);
    center.color = glm::vec4(1);
    mesh.vertices.push_back(center);
    
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        
        Vertex v;
        v.position = glm::vec3(x, -halfHeight, z);
        v.normal = glm::vec3(0, -1, 0);
        v.texCoord = glm::vec2(static_cast<float>(i) / segments, 0);
        v.color = glm::vec4(1);
        mesh.vertices.push_back(v);
    }
    
    for (int i = 0; i < segments; i++) {
        int baseIdx = 2 + i;
        int nextBaseIdx = 2 + i + 1;
        
        Face bottomFace;
        bottomFace.vertices = {centerBottom, nextBaseIdx, baseIdx};
        bottomFace.normal = glm::vec3(0, -1, 0);
        mesh.faces.push_back(bottomFace);
        
        Face sideFace;
        sideFace.vertices = {apexIdx, baseIdx, nextBaseIdx};
        mesh.faces.push_back(sideFace);
    }
    
    mesh.calculateNormals();
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createTorus(const GeometryParams& params) {
    MeshData mesh;
    
    float majorRadius = params.outerRadius;
    float minorRadius = params.innerRadius;
    int majorSegments = params.segments;
    int minorSegments = params.rings;
    
    for (int i = 0; i <= majorSegments; i++) {
        float u = static_cast<float>(i) / majorSegments;
        float theta = u * 2.0f * glm::pi<float>();
        
        for (int j = 0; j <= minorSegments; j++) {
            float v = static_cast<float>(j) / minorSegments;
            float phi = v * 2.0f * glm::pi<float>();
            
            float x = (majorRadius + minorRadius * std::cos(phi)) * std::cos(theta);
            float y = minorRadius * std::sin(phi);
            float z = (majorRadius + minorRadius * std::cos(phi)) * std::sin(theta);
            
            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(glm::vec3(
                std::cos(phi) * std::cos(theta),
                std::sin(phi),
                std::cos(phi) * std::sin(theta)
            ));
            vertex.texCoord = glm::vec2(u, v);
            vertex.color = glm::vec4(1);
            mesh.vertices.push_back(vertex);
        }
    }
    
    for (int i = 0; i < majorSegments; i++) {
        for (int j = 0; j < minorSegments; j++) {
            int i0 = i * (minorSegments + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + minorSegments + 1;
            int i3 = i2 + 1;
            
            Face f;
            f.vertices = {i0, i2, i3, i1};
            mesh.faces.push_back(f);
        }
    }
    
    mesh.calculateNormals();
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createPlane(const GeometryParams& params) {
    MeshData mesh;
    
    float hw = params.width * 0.5f;
    float hd = params.depth * 0.5f;
    int segX = params.segments;
    int segZ = params.rings > 0 ? params.rings : params.segments;
    
    for (int z = 0; z <= segZ; z++) {
        for (int x = 0; x <= segX; x++) {
            Vertex v;
            v.position = glm::vec3(
                -hw + (2.0f * hw * x / segX),
                0,
                -hd + (2.0f * hd * z / segZ)
            );
            v.normal = glm::vec3(0, 1, 0);
            v.texCoord = glm::vec2(
                static_cast<float>(x) / segX,
                static_cast<float>(z) / segZ
            );
            v.color = glm::vec4(1);
            mesh.vertices.push_back(v);
        }
    }
    
    for (int z = 0; z < segZ; z++) {
        for (int x = 0; x < segX; x++) {
            int i0 = z * (segX + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + segX + 1;
            int i3 = i2 + 1;
            
            Face f;
            f.vertices = {i0, i2, i3, i1};
            f.normal = glm::vec3(0, 1, 0);
            mesh.faces.push_back(f);
        }
    }
    
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createCircle(const GeometryParams& params) {
    MeshData mesh;
    
    float radius = params.radius;
    int segments = params.segments;
    
    Vertex center;
    center.position = glm::vec3(0);
    center.normal = glm::vec3(0, 1, 0);
    center.texCoord = glm::vec2(0.5f);
    center.color = glm::vec4(1);
    mesh.vertices.push_back(center);
    
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * i / segments;
        
        Vertex v;
        v.position = glm::vec3(radius * std::cos(theta), 0, radius * std::sin(theta));
        v.normal = glm::vec3(0, 1, 0);
        v.texCoord = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        v.color = glm::vec4(1);
        mesh.vertices.push_back(v);
    }
    
    for (int i = 0; i < segments; i++) {
        Face f;
        f.vertices = {0, i + 1, i + 2};
        f.normal = glm::vec3(0, 1, 0);
        mesh.faces.push_back(f);
    }
    
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

MeshData GeometryFactory::createIcoSphere(const GeometryParams& params) {
    MeshData mesh = createSphere(params);
    return mesh;
}

MeshData GeometryFactory::createCapsule(const GeometryParams& params) {
    MeshData mesh = createSphere(params);
    return mesh;
}

MeshData GeometryFactory::createTorusKnot(const GeometryParams& params) {
    return createTorus(params);
}

MeshData GeometryFactory::createGrid(const GeometryParams& params) {
    return createPlane(params);
}

MeshData GeometryFactory::createMonkey(const GeometryParams& params) {
    return createSphere(params);
}

MeshData GeometryFactory::createFromFunction(
    std::function<glm::vec3(float, float)> positionFunc,
    std::function<glm::vec3(float, float)> normalFunc,
    std::function<glm::vec2(float, float)> uvFunc,
    int uSegments, int vSegments) {
    
    MeshData mesh;
    
    for (int v = 0; v <= vSegments; v++) {
        for (int u = 0; u <= uSegments; u++) {
            float uf = static_cast<float>(u) / uSegments;
            float vf = static_cast<float>(v) / vSegments;
            
            Vertex vertex;
            vertex.position = positionFunc(uf, vf);
            vertex.normal = normalFunc ? normalFunc(uf, vf) : glm::vec3(0, 1, 0);
            vertex.texCoord = uvFunc ? uvFunc(uf, vf) : glm::vec2(uf, vf);
            vertex.color = glm::vec4(1);
            mesh.vertices.push_back(vertex);
        }
    }
    
    for (int v = 0; v < vSegments; v++) {
        for (int u = 0; u < uSegments; u++) {
            int i0 = v * (uSegments + 1) + u;
            int i1 = i0 + 1;
            int i2 = i0 + uSegments + 1;
            int i3 = i2 + 1;
            
            Face f;
            f.vertices = {i0, i2, i3, i1};
            mesh.faces.push_back(f);
        }
    }
    
    mesh.calculateNormals();
    mesh.buildEdgeList();
    mesh.calculateBounds();
    
    return mesh;
}

void GeometryFactory::addQuad(MeshData& mesh, int v0, int v1, int v2, int v3) {
    Face f;
    f.vertices = {v0, v1, v2, v3};
    mesh.faces.push_back(f);
}

void GeometryFactory::addTriangle(MeshData& mesh, int v0, int v1, int v2) {
    Face f;
    f.vertices = {v0, v1, v2};
    mesh.faces.push_back(f);
}

void GeometryFactory::addVertex(MeshData& mesh, const Vertex& v) {
    mesh.vertices.push_back(v);
}

}
}
