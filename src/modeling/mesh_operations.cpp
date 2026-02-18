#include "mesh_operations.h"
#include <algorithm>
#include <unordered_set>
#include <random>

namespace v3d {
namespace modeling {

void MeshOperations::extrude(Mesh& mesh, const std::vector<int>& faceIndices, float distance) {
    std::unordered_set<int> verticesToExtrude;

    for (int faceIndex : faceIndices) {
        const Face& face = mesh.getFace(faceIndex);
        for (int vertexIndex : face.vertices) {
            verticesToExtrude.insert(vertexIndex);
        }
    }

    std::unordered_map<int, int> vertexMap;
    for (int vertexIndex : verticesToExtrude) {
        const Vertex& vertex = mesh.getVertex(vertexIndex);
        Vertex newVertex = vertex;
        newVertex.position += vertex.normal * distance;
        int newIndex = mesh.addVertex(newVertex);
        vertexMap[vertexIndex] = newIndex;
    }

    for (int faceIndex : faceIndices) {
        const Face& face = mesh.getFace(faceIndex);
        std::vector<int> newVertices;
        for (int vertexIndex : face.vertices) {
            newVertices.push_back(vertexMap[vertexIndex]);
        }
        mesh.addFace(newVertices);
    }
}

void MeshOperations::extrude(Mesh& mesh, const std::vector<int>& vertexIndices, const glm::vec3& direction, float distance) {
    std::unordered_map<int, int> vertexMap;
    for (int vertexIndex : vertexIndices) {
        const Vertex& vertex = mesh.getVertex(vertexIndex);
        Vertex newVertex = vertex;
        newVertex.position += glm::normalize(direction) * distance;
        int newIndex = mesh.addVertex(newVertex);
        vertexMap[vertexIndex] = newIndex;
    }

    std::unordered_set<int> vertexSet(vertexIndices.begin(), vertexIndices.end());

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = mesh.getFace(i);
        bool allVerticesInSet = true;
        for (int vertexIndex : face.vertices) {
            if (vertexSet.find(vertexIndex) == vertexSet.end()) {
                allVerticesInSet = false;
                break;
            }
        }

        if (allVerticesInSet) {
            std::vector<int> newVertices;
            for (int vertexIndex : face.vertices) {
                newVertices.push_back(vertexMap[vertexIndex]);
            }
            mesh.addFace(newVertices);
        }
    }
}

void MeshOperations::bevelEdges(Mesh& mesh, const std::vector<int>& edgeIndices, float amount) {
    std::unordered_set<int> verticesToBevel;

    for (int edgeIndex : edgeIndices) {
        const Edge& edge = mesh.getEdge(edgeIndex);
        verticesToBevel.insert(edge.vertex1);
        verticesToBevel.insert(edge.vertex2);
    }

    std::unordered_map<int, int> vertexMap;
    for (int vertexIndex : verticesToBevel) {
        const Vertex& vertex = mesh.getVertex(vertexIndex);
        Vertex newVertex = vertex;
        newVertex.position += vertex.normal * amount;
        int newIndex = mesh.addVertex(newVertex);
        vertexMap[vertexIndex] = newIndex;
    }

    for (int edgeIndex : edgeIndices) {
        const Edge& edge = mesh.getEdge(edgeIndex);
        int v1 = edge.vertex1;
        int v2 = edge.vertex2;

        int newV1 = vertexMap[v1];
        int newV2 = vertexMap[v2];

        mesh.addQuad(v1, v2, newV2, newV1);
    }
}

void MeshOperations::bevelVertices(Mesh& mesh, const std::vector<int>& vertexIndices, float amount) {
    for (int vertexIndex : vertexIndices) {
        const Vertex& vertex = mesh.getVertex(vertexIndex);
        glm::vec3 offset = vertex.normal * amount;

        std::vector<int> adjacentVertices = mesh.getAdjacentVertices(vertexIndex);
        std::vector<int> adjacentFaces = mesh.getAdjacentFaces(vertexIndex);

        std::unordered_set<int> adjacentVertexSet(adjacentVertices.begin(), adjacentVertices.end());

        for (int faceIndex : adjacentFaces) {
            const Face& face = mesh.getFace(faceIndex);
            std::vector<int> newFaceVertices;

            for (int fv : face.vertices) {
                if (fv == vertexIndex) {
                    Vertex newVertex = mesh.getVertex(fv);
                    newVertex.position += offset;
                    int newIndex = mesh.addVertex(newVertex);
                    newFaceVertices.push_back(newIndex);
                } else {
                    newFaceVertices.push_back(fv);
                }
            }

            mesh.addFace(newFaceVertices);
        }
    }
}

void MeshOperations::inset(Mesh& mesh, const std::vector<int>& faceIndices, float amount) {
    for (int faceIndex : faceIndices) {
        const Face& face = mesh.getFace(faceIndex);
        glm::vec3 centroid = face.centroid;
        glm::vec3 normal = face.normal;

        std::vector<int> newVertices;
        for (int vertexIndex : face.vertices) {
            const Vertex& vertex = mesh.getVertex(vertexIndex);
            glm::vec3 direction = glm::normalize(vertex.position - centroid);
            Vertex newVertex = vertex;
            newVertex.position = centroid + direction * (glm::length(vertex.position - centroid) * (1.0f - amount));
            newVertex.position += normal * amount * 0.1f;
            newVertices.push_back(mesh.addVertex(newVertex));
        }

        mesh.addFace(newVertices);
    }
}

void MeshOperations::bridge(Mesh& mesh, const std::vector<int>& edgeLoop1, const std::vector<int>& edgeLoop2) {
    if (edgeLoop1.size() != edgeLoop2.size()) {
        return;
    }

    size_t loopSize = edgeLoop1.size();

    for (size_t i = 0; i < loopSize; ++i) {
        int v1 = edgeLoop1[i];
        int v2 = edgeLoop1[(i + 1) % loopSize];
        int v3 = edgeLoop2[(i + 1) % loopSize];
        int v4 = edgeLoop2[i];

        mesh.addQuad(v1, v2, v3, v4);
    }
}

void MeshOperations::cut(Mesh& mesh, const glm::vec3& point, const glm::vec3& normal) {
}

void MeshOperations::knife(Mesh& mesh, const glm::vec3& start, const glm::vec3& end) {
}

void MeshOperations::booleanUnion(Mesh& result, const Mesh& a, const Mesh& b) {
    result = a;
    result.merge(b);
}

void MeshOperations::booleanIntersection(Mesh& result, const Mesh& a, const Mesh& b) {
}

void MeshOperations::booleanDifference(Mesh& result, const Mesh& a, const Mesh& b) {
}

void MeshOperations::mergeVertices(Mesh& mesh, float threshold) {
    std::vector<int> verticesToRemove;

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        for (int j = i + 1; j < mesh.getVertexCount(); ++j) {
            const Vertex& v1 = mesh.getVertex(i);
            const Vertex& v2 = mesh.getVertex(j);

            if (glm::length(v1.position - v2.position) < threshold) {
                verticesToRemove.push_back(j);
            }
        }
    }

    std::sort(verticesToRemove.begin(), verticesToRemove.end());
    verticesToRemove.erase(std::unique(verticesToRemove.begin(), verticesToRemove.end()), verticesToRemove.end());

    for (int i = static_cast<int>(verticesToRemove.size()) - 1; i >= 0; --i) {
        mesh.removeVertex(verticesToRemove[i]);
    }
}

void MeshOperations::mergeVertices(Mesh& mesh, const std::vector<int>& vertices) {
    if (vertices.empty()) {
        return;
    }

    int targetVertex = vertices[0];
    glm::vec3 mergedPosition(0.0f);

    for (int vertexIndex : vertices) {
        mergedPosition += mesh.getVertex(vertexIndex).position;
    }
    mergedPosition /= static_cast<float>(vertices.size());

    Vertex target = mesh.getVertex(targetVertex);
    target.position = mergedPosition;
    mesh.setVertex(targetVertex, target);

    for (size_t i = 1; i < vertices.size(); ++i) {
        int vertexIndex = vertices[i];
        std::vector<int> adjacentFaces = mesh.getAdjacentFaces(vertexIndex);

        for (int faceIndex : adjacentFaces) {
            Face& face = mesh.getFace(faceIndex);
            for (int& fv : face.vertices) {
                if (fv == vertexIndex) {
                    fv = targetVertex;
                }
            }
        }
    }

    for (size_t i = 1; i < vertices.size(); ++i) {
        mesh.removeVertex(vertices[i]);
    }
}

void MeshOperations::splitEdge(Mesh& mesh, int edgeIndex) {
    const Edge& edge = mesh.getEdge(edgeIndex);
    const Vertex& v1 = mesh.getVertex(edge.vertex1);
    const Vertex& v2 = mesh.getVertex(edge.vertex2);

    Vertex newVertex;
    newVertex.position = (v1.position + v2.position) * 0.5f;
    newVertex.normal = glm::normalize(v1.normal + v2.normal);
    newVertex.texCoord = (v1.texCoord + v2.texCoord) * 0.5f;

    int newVertexIndex = mesh.addVertex(newVertex);

    std::vector<int> facesToUpdate;
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = mesh.getFace(i);
        if (face.containsEdge(edgeIndex)) {
            facesToUpdate.push_back(i);
        }
    }

    for (int faceIndex : facesToUpdate) {
        const Face& face = mesh.getFace(faceIndex);
        std::vector<int> newVertices;

        for (int fv : face.vertices) {
            newVertices.push_back(fv);
            if (fv == edge.vertex1 || fv == edge.vertex2) {
                newVertices.push_back(newVertexIndex);
            }
        }

        mesh.removeFace(faceIndex);
        mesh.addFace(newVertices);
    }
}

void MeshOperations::splitFace(Mesh& mesh, int faceIndex, const glm::vec3& point) {
    const Face& face = mesh.getFace(faceIndex);

    Vertex newVertex;
    newVertex.position = point;
    newVertex.normal = face.normal;
    int newVertexIndex = mesh.addVertex(newVertex);

    std::vector<int> vertices = face.vertices;
    mesh.removeFace(faceIndex);

    for (size_t i = 0; i < vertices.size(); ++i) {
        int v0 = vertices[i];
        int v1 = vertices[(i + 1) % vertices.size()];
        mesh.addTriangle(v0, v1, newVertexIndex);
    }
}

void MeshOperations::collapseEdge(Mesh& mesh, int edgeIndex) {
    const Edge& edge = mesh.getEdge(edgeIndex);
    int v1 = edge.vertex1;
    int v2 = edge.vertex2;

    const Vertex& vertex1 = mesh.getVertex(v1);
    const Vertex& vertex2 = mesh.getVertex(v2);

    glm::vec3 newPosition = (vertex1.position + vertex2.position) * 0.5f;
    glm::vec3 newNormal = glm::normalize(vertex1.normal + vertex2.normal);
    glm::vec2 newTexCoord = (vertex1.texCoord + vertex2.texCoord) * 0.5f;

    Vertex mergedVertex = vertex1;
    mergedVertex.position = newPosition;
    mergedVertex.normal = newNormal;
    mergedVertex.texCoord = newTexCoord;
    mesh.setVertex(v1, mergedVertex);

    std::vector<int> facesToUpdate;
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = mesh.getFace(i);
        if (face.containsVertex(v2)) {
            facesToUpdate.push_back(i);
        }
    }

    for (int faceIndex : facesToUpdate) {
        Face& face = mesh.getFace(faceIndex);
        for (int& fv : face.vertices) {
            if (fv == v2) {
                fv = v1;
            }
        }
    }

    mesh.removeVertex(v2);
}

void MeshOperations::collapseVertex(Mesh& mesh, int vertexIndex) {
    std::vector<int> adjacentVertices = mesh.getAdjacentVertices(vertexIndex);

    if (adjacentVertices.empty()) {
        return;
    }

    const Vertex& vertex = mesh.getVertex(vertexIndex);
    glm::vec3 newPosition(0.0f);
    glm::vec3 newNormal(0.0f);

    for (int av : adjacentVertices) {
        newPosition += mesh.getVertex(av).position;
        newNormal += mesh.getVertex(av).normal;
    }

    newPosition /= static_cast<float>(adjacentVertices.size());
    newNormal = glm::normalize(newNormal);

    Vertex newVertex = vertex;
    newVertex.position = newPosition;
    newVertex.normal = newNormal;
    mesh.setVertex(vertexIndex, newVertex);
}

void MeshOperations::dissolveEdge(Mesh& mesh, int edgeIndex) {
    const Edge& edge = mesh.getEdge(edgeIndex);

    if (!edge.isBoundary()) {
        return;
    }

    std::vector<int> facesToRemove;
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = mesh.getFace(i);
        if (face.containsEdge(edgeIndex)) {
            facesToRemove.push_back(i);
        }
    }

    for (int faceIndex : facesToRemove) {
        mesh.removeFace(faceIndex);
    }
}

void MeshOperations::dissolveFace(Mesh& mesh, int faceIndex) {
    mesh.removeFace(faceIndex);
}

void MeshOperations::weld(Mesh& mesh, const std::vector<int>& vertices) {
    mergeVertices(mesh, vertices);
}

void MeshOperations::bridgeLoops(Mesh& mesh, const std::vector<int>& loop1, const std::vector<int>& loop2) {
    bridge(mesh, loop1, loop2);
}

void MeshOperations::smooth(Mesh& mesh, int iterations, float factor) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<glm::vec3> newPositions(mesh.getVertexCount());

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            const Vertex& vertex = mesh.getVertex(i);
            std::vector<int> adjacentVertices = mesh.getAdjacentVertices(i);

            if (adjacentVertices.empty()) {
                newPositions[i] = vertex.position;
                continue;
            }

            glm::vec3 average(0.0f);
            for (int av : adjacentVertices) {
                average += mesh.getVertex(av).position;
            }
            average /= static_cast<float>(adjacentVertices.size());

            newPositions[i] = glm::mix(vertex.position, average, factor);
        }

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            Vertex vertex = mesh.getVertex(i);
            vertex.position = newPositions[i];
            mesh.setVertex(i, vertex);
        }

        mesh.calculateNormals();
    }
}

void MeshOperations::sharpen(Mesh& mesh, int iterations, float factor) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<glm::vec3> newPositions(mesh.getVertexCount());

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            const Vertex& vertex = mesh.getVertex(i);
            std::vector<int> adjacentVertices = mesh.getAdjacentVertices(i);

            if (adjacentVertices.empty()) {
                newPositions[i] = vertex.position;
                continue;
            }

            glm::vec3 average(0.0f);
            for (int av : adjacentVertices) {
                average += mesh.getVertex(av).position;
            }
            average /= static_cast<float>(adjacentVertices.size());

            glm::vec3 direction = vertex.position - average;
            newPositions[i] = vertex.position + direction * factor;
        }

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            Vertex vertex = mesh.getVertex(i);
            vertex.position = newPositions[i];
            mesh.setVertex(i, vertex);
        }

        mesh.calculateNormals();
    }
}

void MeshOperations::deform(Mesh& mesh, std::function<void(glm::vec3&)> deformer) {
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);
        deformer(vertex.position);
        mesh.setVertex(i, vertex);
    }
    mesh.calculateNormals();
}

void MeshOperations::twist(Mesh& mesh, const glm::vec3& axis, float angle, float falloff) {
    glm::vec3 normalizedAxis = glm::normalize(axis);
    glm::vec3 center = mesh.getCenter();

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        float distance = glm::length(toVertex);
        glm::vec3 projection = glm::dot(toVertex, normalizedAxis) * normalizedAxis;
        glm::vec3 perpendicular = toVertex - projection;

        float twistAngle = angle;
        if (falloff > 0.0f) {
            twistAngle *= std::exp(-distance * falloff);
        }

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), twistAngle, normalizedAxis);
        glm::vec4 rotatedPerp = rotation * glm::vec4(perpendicular, 1.0f);

        vertex.position = center + projection + glm::vec3(rotatedPerp);
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::bend(Mesh& mesh, const glm::vec3& axis, float angle, float falloff) {
    glm::vec3 normalizedAxis = glm::normalize(axis);
    glm::vec3 center = mesh.getCenter();

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        float distance = glm::length(toVertex);
        glm::vec3 projection = glm::dot(toVertex, normalizedAxis) * normalizedAxis;
        glm::vec3 perpendicular = toVertex - projection;

        float bendAngle = angle;
        if (falloff > 0.0f) {
            bendAngle *= std::exp(-distance * falloff);
        }

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), bendAngle, normalizedAxis);
        glm::vec4 rotatedPerp = rotation * glm::vec4(perpendicular, 1.0f);

        vertex.position = center + projection + glm::vec3(rotatedPerp);
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::taper(Mesh& mesh, const glm::vec3& axis, float factor) {
    glm::vec3 normalizedAxis = glm::normalize(axis);
    glm::vec3 center = mesh.getCenter();

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        float distance = glm::dot(toVertex, normalizedAxis);
        glm::vec3 projection = distance * normalizedAxis;
        glm::vec3 perpendicular = toVertex - projection;

        float scale = 1.0f + factor * distance;

        vertex.position = center + projection + perpendicular * scale;
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::bulge(Mesh& mesh, const glm::vec3& center, float radius, float strength) {
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        float distance = glm::length(toVertex);

        if (distance < radius) {
            float falloff = 1.0f - (distance / radius);
            falloff = falloff * falloff * (3.0f - 2.0f * falloff);

            glm::vec3 direction = glm::normalize(toVertex);
            vertex.position += direction * strength * falloff;
            mesh.setVertex(i, vertex);
        }
    }

    mesh.calculateNormals();
}

void MeshOperations::noise(Mesh& mesh, float strength, int seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 noise(
            dist(gen),
            dist(gen),
            dist(gen)
        );

        vertex.position += noise * strength;
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::mirror(Mesh& mesh, const glm::vec3& axis, bool merge) {
    glm::vec3 normalizedAxis = glm::normalize(axis);

    std::vector<int> originalVertexIndices;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        originalVertexIndices.push_back(i);
    }

    std::unordered_map<int, int> vertexMap;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const Vertex& vertex = mesh.getVertex(i);

        glm::vec3 reflectedPosition = vertex.position - 2.0f * glm::dot(vertex.position, normalizedAxis) * normalizedAxis;
        glm::vec3 reflectedNormal = vertex.normal - 2.0f * glm::dot(vertex.normal, normalizedAxis) * normalizedAxis;

        Vertex newVertex;
        newVertex.position = reflectedPosition;
        newVertex.normal = reflectedNormal;
        newVertex.texCoord = vertex.texCoord;
        newVertex.tangent = vertex.tangent;
        newVertex.bitangent = vertex.bitangent;
        newVertex.color = vertex.color;

        int newIndex = mesh.addVertex(newVertex);
        vertexMap[i] = newIndex;
    }

    int originalFaceCount = mesh.getFaceCount();
    for (int i = 0; i < originalFaceCount; ++i) {
        const Face& face = mesh.getFace(i);
        std::vector<int> newVertices;

        for (int fv : face.vertices) {
            newVertices.push_back(vertexMap[fv]);
        }

        std::reverse(newVertices.begin(), newVertices.end());
        mesh.addFace(newVertices);
    }

    if (merge) {
        mergeVertices(mesh, 0.0001f);
    }
}

void MeshOperations::array(Mesh& mesh, const glm::vec3& offset, int count) {
    if (count <= 1) {
        return;
    }

    Mesh originalMesh;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        originalMesh.addVertex(mesh.getVertex(i));
    }
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        originalMesh.addFace(mesh.getFace(i).vertices);
    }

    for (int i = 1; i < count; ++i) {
        glm::vec3 currentOffset = offset * static_cast<float>(i);

        for (int j = 0; j < originalMesh.getVertexCount(); ++j) {
            Vertex vertex = originalMesh.getVertex(j);
            vertex.position += currentOffset;
            mesh.addVertex(vertex);
        }

        int vertexOffset = i * originalMesh.getVertexCount();
        for (int j = 0; j < originalMesh.getFaceCount(); ++j) {
            const Face& face = originalMesh.getFace(j);
            std::vector<int> newVertices;
            for (int fv : face.vertices) {
                newVertices.push_back(fv + vertexOffset);
            }
            mesh.addFace(newVertices);
        }
    }
}

void MeshOperations::radialArray(Mesh& mesh, const glm::vec3& axis, int count, float angle) {
    if (count <= 1) {
        return;
    }

    glm::vec3 normalizedAxis = glm::normalize(axis);
    float angleStep = glm::radians(angle) / static_cast<float>(count);

    Mesh originalMesh;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        originalMesh.addVertex(mesh.getVertex(i));
    }
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        originalMesh.addFace(mesh.getFace(i).vertices);
    }

    for (int i = 1; i < count; ++i) {
        float currentAngle = angleStep * static_cast<float>(i);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), currentAngle, normalizedAxis);

        for (int j = 0; j < originalMesh.getVertexCount(); ++j) {
            Vertex vertex = originalMesh.getVertex(j);
            glm::vec4 rotatedPos = rotation * glm::vec4(vertex.position, 1.0f);
            glm::vec4 rotatedNorm = rotation * glm::vec4(vertex.normal, 0.0f);
            vertex.position = glm::vec3(rotatedPos);
            vertex.normal = glm::normalize(glm::vec3(rotatedNorm));
            mesh.addVertex(vertex);
        }

        int vertexOffset = i * originalMesh.getVertexCount();
        for (int j = 0; j < originalMesh.getFaceCount(); ++j) {
            const Face& face = originalMesh.getFace(j);
            std::vector<int> newVertices;
            for (int fv : face.vertices) {
                newVertices.push_back(fv + vertexOffset);
            }
            mesh.addFace(newVertices);
        }
    }
}

void MeshOperations::projectToPlane(Mesh& mesh, const glm::vec3& point, const glm::vec3& normal) {
    glm::vec3 normalizedNormal = glm::normalize(normal);

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - point;
        float distance = glm::dot(toVertex, normalizedNormal);

        vertex.position -= normalizedNormal * distance;
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::projectToSphere(Mesh& mesh, const glm::vec3& center, float radius) {
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        float distance = glm::length(toVertex);

        if (distance > 0.0001f) {
            vertex.position = center + glm::normalize(toVertex) * radius;
            mesh.setVertex(i, vertex);
        }
    }

    mesh.calculateNormals();
}

void MeshOperations::projectToCylinder(Mesh& mesh, const glm::vec3& axis, float radius) {
    glm::vec3 normalizedAxis = glm::normalize(axis);

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position;
        float projection = glm::dot(toVertex, normalizedAxis);
        glm::vec3 projectionVec = projection * normalizedAxis;
        glm::vec3 perpendicular = toVertex - projectionVec;

        float perpendicularLength = glm::length(perpendicular);

        if (perpendicularLength > 0.0001f) {
            vertex.position = projectionVec + glm::normalize(perpendicular) * radius;
            mesh.setVertex(i, vertex);
        }
    }

    mesh.calculateNormals();
}

void MeshOperations::triangulate(Mesh& mesh) {
    std::vector<int> facesToRemove;

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = mesh.getFace(i);

        if (face.vertices.size() > 3) {
            facesToRemove.push_back(i);

            for (size_t j = 1; j < face.vertices.size() - 1; ++j) {
                mesh.addTriangle(face.vertices[0], face.vertices[j], face.vertices[j + 1]);
            }
        }
    }

    for (int i = static_cast<int>(facesToRemove.size()) - 1; i >= 0; --i) {
        mesh.removeFace(facesToRemove[i]);
    }
}

void MeshOperations::quadrangulate(Mesh& mesh) {
}

void MeshOperations::calculateConvexHull(Mesh& mesh) {
}

void MeshOperations::relax(Mesh& mesh, int iterations) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<glm::vec3> newPositions(mesh.getVertexCount());

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            const Vertex& vertex = mesh.getVertex(i);
            std::vector<int> adjacentVertices = mesh.getAdjacentVertices(i);

            if (adjacentVertices.empty()) {
                newPositions[i] = vertex.position;
                continue;
            }

            glm::vec3 average(0.0f);
            for (int av : adjacentVertices) {
                average += mesh.getVertex(av).position;
            }
            average /= static_cast<float>(adjacentVertices.size());

            newPositions[i] = average;
        }

        for (int i = 0; i < mesh.getVertexCount(); ++i) {
            Vertex vertex = mesh.getVertex(i);
            vertex.position = newPositions[i];
            mesh.setVertex(i, vertex);
        }

        mesh.calculateNormals();
    }
}

void MeshOperations::uniformRelax(Mesh& mesh, int iterations) {
    relax(mesh, iterations);
}

void MeshOperations::shrink(Mesh& mesh, float amount) {
    glm::vec3 center = mesh.getCenter();

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        Vertex vertex = mesh.getVertex(i);

        glm::vec3 toVertex = vertex.position - center;
        glm::vec3 direction = glm::normalize(toVertex);

        vertex.position = center + direction * (glm::length(toVertex) - amount);
        mesh.setVertex(i, vertex);
    }

    mesh.calculateNormals();
}

void MeshOperations::expand(Mesh& mesh, float amount) {
    shrink(mesh, -amount);
}

glm::vec3 MeshOperations::calculateFaceNormal(const std::vector<glm::vec3>& vertices) {
    if (vertices.size() < 3) {
        return glm::vec3(0.0f);
    }

    glm::vec3 edge1 = vertices[1] - vertices[0];
    glm::vec3 edge2 = vertices[2] - vertices[0];

    return glm::normalize(glm::cross(edge1, edge2));
}

glm::vec3 MeshOperations::calculateFaceCentroid(const std::vector<glm::vec3>& vertices) {
    if (vertices.empty()) {
        return glm::vec3(0.0f);
    }

    glm::vec3 sum(0.0f);
    for (const auto& vertex : vertices) {
        sum += vertex;
    }

    return sum / static_cast<float>(vertices.size());
}

}
}