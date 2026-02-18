#include "subdivision.h"
#include <unordered_map>

namespace v3d {
namespace modeling {

void Subdivision::subdivide(Mesh& mesh, SubdivisionMethod method, int levels) {
    for (int i = 0; i < levels; ++i) {
        switch (method) {
            case SubdivisionMethod::CatmullClark:
                catmullClark(mesh);
                break;
            case SubdivisionMethod::Loop:
                loop(mesh);
                break;
            case SubdivisionMethod::DooSabin:
                dooSabin(mesh);
                break;
            case SubdivisionMethod::Midpoint:
                midpoint(mesh);
                break;
            case SubdivisionMethod::Linear:
                linear(mesh);
                break;
        }
    }
}

void Subdivision::catmullClark(Mesh& mesh, int levels) {
    for (int i = 0; i < levels; ++i) {
        catmullClarkStep(mesh);
    }
}

void Subdivision::loop(Mesh& mesh, int levels) {
    for (int i = 0; i < levels; ++i) {
        loopStep(mesh);
    }
}

void Subdivision::dooSabin(Mesh& mesh, int levels) {
    for (int i = 0; i < levels; ++i) {
        dooSabinStep(mesh);
    }
}

void Subdivision::midpoint(Mesh& mesh, int levels) {
    for (int i = 0; i < levels; ++i) {
        midpointStep(mesh);
    }
}

void Subdivision::linear(Mesh& mesh, int levels) {
    for (int i = 0; i < levels; ++i) {
        linearStep(mesh);
    }
}

void Subdivision::smooth(Mesh& mesh, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        catmullClarkStep(mesh);
    }
}

void Subdivision::sharpen(Mesh& mesh, int iterations) {
}

void Subdivision::tessellate(Mesh& mesh, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        linearStep(mesh);
    }
}

void Subdivision::catmullClarkStep(Mesh& mesh) {
    std::unordered_map<int, int> facePointMap;
    std::unordered_map<int, int> edgePointMap;

    const auto& vertices = mesh.getVertices();
    const auto& faces = mesh.getFaces();
    const auto& edges = mesh.getEdges();

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];
        glm::vec3 facePoint = calculateFacePoint(face, vertices);

        Vertex newVertex;
        newVertex.position = facePoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        facePointMap[i] = newVertexIndex;
    }

    for (int i = 0; i < mesh.getEdgeCount(); ++i) {
        const Edge& edge = edges[i];
        glm::vec3 edgePoint = calculateEdgePoint(edge, vertices, faces);

        Vertex newVertex;
        newVertex.position = edgePoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        edgePointMap[i] = newVertexIndex;
    }

    std::unordered_map<int, int> vertexPointMap;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        glm::vec3 vertexPoint = calculateVertexPoint(i, mesh);

        Vertex newVertex;
        newVertex.position = vertexPoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        vertexPointMap[i] = newVertexIndex;
    }

    std::vector<std::vector<int>> newFaces;

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];
        int facePointIndex = facePointMap[i];

        for (size_t j = 0; j < face.vertices.size(); ++j) {
            int v0 = face.vertices[j];
            int v1 = face.vertices[(j + 1) % face.vertices.size()];

            int edgeIndex = -1;
            for (int ei = 0; ei < mesh.getEdgeCount(); ++ei) {
                const Edge& edge = edges[ei];
                if ((edge.vertex1 == v0 && edge.vertex2 == v1) ||
                    (edge.vertex1 == v1 && edge.vertex2 == v0)) {
                    edgeIndex = ei;
                    break;
                }
            }

            int edgePointIndex = edgePointMap[edgeIndex];
            int vertexPointIndex = vertexPointMap[v0];

            int nextEdgeIndex = -1;
            int nextV0 = v1;
            int nextV1 = face.vertices[(j + 2) % face.vertices.size()];
            for (int ei = 0; ei < mesh.getEdgeCount(); ++ei) {
                const Edge& edge = edges[ei];
                if ((edge.vertex1 == nextV0 && edge.vertex2 == nextV1) ||
                    (edge.vertex1 == nextV1 && edge.vertex2 == nextV0)) {
                    nextEdgeIndex = ei;
                    break;
                }
            }
            int nextEdgePointIndex = edgePointMap[nextEdgeIndex];

            newFaces.push_back({vertexPointIndex, edgePointIndex, facePointIndex, nextEdgePointIndex});
        }
    }

    mesh.clear();

    for (const auto& pair : vertexPointMap) {
        mesh.addVertex(vertices[pair.first]);
    }

    for (const auto& pair : facePointMap) {
        const Face& face = faces[pair.first];
        glm::vec3 facePoint = calculateFacePoint(face, vertices);
        mesh.addVertex(Vertex(facePoint));
    }

    for (const auto& pair : edgePointMap) {
        const Edge& edge = edges[pair.first];
        glm::vec3 edgePoint = calculateEdgePoint(edge, vertices, faces);
        mesh.addVertex(Vertex(edgePoint));
    }

    for (const auto& newFace : newFaces) {
        mesh.addFace(newFace);
    }

    mesh.calculateNormals();
}

void Subdivision::loopStep(Mesh& mesh) {
    const auto& vertices = mesh.getVertices();
    const auto& faces = mesh.getFaces();
    const auto& edges = mesh.getEdges();

    std::unordered_map<int, int> edgePointMap;

    for (int i = 0; i < mesh.getEdgeCount(); ++i) {
        const Edge& edge = edges[i];

        if (edge.isBoundary()) {
            const Vertex& v1 = vertices[edge.vertex1];
            const Vertex& v2 = vertices[edge.vertex2];

            glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;

            Vertex newVertex;
            newVertex.position = edgePoint;
            int newVertexIndex = mesh.addVertex(newVertex);
            edgePointMap[i] = newVertexIndex;
        } else {
            const Face& face1 = faces[edge.face1];
            const Face& face2 = faces[edge.face2];

            int oppositeVertex1 = -1;
            int oppositeVertex2 = -1;

            for (int fv : face1.vertices) {
                if (fv != edge.vertex1 && fv != edge.vertex2) {
                    oppositeVertex1 = fv;
                    break;
                }
            }

            for (int fv : face2.vertices) {
                if (fv != edge.vertex1 && fv != edge.vertex2) {
                    oppositeVertex2 = fv;
                    break;
                }
            }

            const Vertex& v1 = vertices[edge.vertex1];
            const Vertex& v2 = vertices[edge.vertex2];
            const Vertex& v3 = vertices[oppositeVertex1];
            const Vertex& v4 = vertices[oppositeVertex2];

            glm::vec3 edgePoint = (v1.position + v2.position) * 0.375f +
                                 (v3.position + v4.position) * 0.125f;

            Vertex newVertex;
            newVertex.position = edgePoint;
            int newVertexIndex = mesh.addVertex(newVertex);
            edgePointMap[i] = newVertexIndex;
        }
    }

    std::unordered_map<int, int> vertexPointMap;
    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const Vertex& vertex = vertices[i];
        std::vector<int> adjacentVertices = mesh.getAdjacentVertices(i);

        float beta = 0.0f;
        if (adjacentVertices.size() == 3) {
            beta = 3.0f / 16.0f;
        } else {
            beta = 3.0f / (8.0f * adjacentVertices.size());
        }

        glm::vec3 sum(0.0f);
        for (int av : adjacentVertices) {
            sum += vertices[av].position;
        }

        glm::vec3 vertexPoint = vertex.position * (1.0f - adjacentVertices.size() * beta) +
                                sum * beta;

        Vertex newVertex;
        newVertex.position = vertexPoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        vertexPointMap[i] = newVertexIndex;
    }

    std::vector<std::vector<int>> newFaces;

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];

        for (size_t j = 0; j < face.vertices.size(); ++j) {
            int v0 = face.vertices[j];
            int v1 = face.vertices[(j + 1) % face.vertices.size()];
            int v2 = face.vertices[(j + 2) % face.vertices.size()];

            int edgeIndex1 = -1;
            int edgeIndex2 = -1;

            for (int ei = 0; ei < mesh.getEdgeCount(); ++ei) {
                const Edge& edge = edges[ei];
                if ((edge.vertex1 == v0 && edge.vertex2 == v1) ||
                    (edge.vertex1 == v1 && edge.vertex2 == v0)) {
                    edgeIndex1 = ei;
                }
                if ((edge.vertex1 == v1 && edge.vertex2 == v2) ||
                    (edge.vertex1 == v2 && edge.vertex2 == v1)) {
                    edgeIndex2 = ei;
                }
            }

            int vertexPoint0 = vertexPointMap[v0];
            int edgePoint1 = edgePointMap[edgeIndex1];
            int edgePoint2 = edgePointMap[edgeIndex2];

            newFaces.push_back({vertexPoint0, edgePoint1, edgePoint2});
        }
    }

    mesh.clear();

    for (const auto& pair : vertexPointMap) {
        mesh.addVertex(vertices[pair.first]);
    }

    for (const auto& pair : edgePointMap) {
        const Edge& edge = edges[pair.first];

        if (edge.isBoundary()) {
            const Vertex& v1 = vertices[edge.vertex1];
            const Vertex& v2 = vertices[edge.vertex2];
            glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;
            mesh.addVertex(Vertex(edgePoint));
        } else {
            const Face& face1 = faces[edge.face1];
            const Face& face2 = faces[edge.face2];

            int oppositeVertex1 = -1;
            int oppositeVertex2 = -1;

            for (int fv : face1.vertices) {
                if (fv != edge.vertex1 && fv != edge.vertex2) {
                    oppositeVertex1 = fv;
                    break;
                }
            }

            for (int fv : face2.vertices) {
                if (fv != edge.vertex1 && fv != edge.vertex2) {
                    oppositeVertex2 = fv;
                    break;
                }
            }

            const Vertex& v1 = vertices[edge.vertex1];
            const Vertex& v2 = vertices[edge.vertex2];
            const Vertex& v3 = vertices[oppositeVertex1];
            const Vertex& v4 = vertices[oppositeVertex2];

            glm::vec3 edgePoint = (v1.position + v2.position) * 0.375f +
                                 (v3.position + v4.position) * 0.125f;
            mesh.addVertex(Vertex(edgePoint));
        }
    }

    for (const auto& newFace : newFaces) {
        mesh.addFace(newFace);
    }

    mesh.calculateNormals();
}

void Subdivision::dooSabinStep(Mesh& mesh) {
}

void Subdivision::midpointStep(Mesh& mesh) {
    const auto& vertices = mesh.getVertices();
    const auto& faces = mesh.getFaces();
    const auto& edges = mesh.getEdges();

    std::unordered_map<int, int> edgePointMap;

    for (int i = 0; i < mesh.getEdgeCount(); ++i) {
        const Edge& edge = edges[i];
        const Vertex& v1 = vertices[edge.vertex1];
        const Vertex& v2 = vertices[edge.vertex2];

        glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;

        Vertex newVertex;
        newVertex.position = edgePoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        edgePointMap[i] = newVertexIndex;
    }

    std::unordered_map<int, int> facePointMap;
    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];
        glm::vec3 facePoint = calculateFacePoint(face, vertices);

        Vertex newVertex;
        newVertex.position = facePoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        facePointMap[i] = newVertexIndex;
    }

    std::vector<std::vector<int>> newFaces;

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];
        int facePointIndex = facePointMap[i];

        for (size_t j = 0; j < face.vertices.size(); ++j) {
            int v0 = face.vertices[j];
            int v1 = face.vertices[(j + 1) % face.vertices.size()];

            int edgeIndex = -1;
            for (int ei = 0; ei < mesh.getEdgeCount(); ++ei) {
                const Edge& edge = edges[ei];
                if ((edge.vertex1 == v0 && edge.vertex2 == v1) ||
                    (edge.vertex1 == v1 && edge.vertex2 == v0)) {
                    edgeIndex = ei;
                    break;
                }
            }

            int edgePointIndex = edgePointMap[edgeIndex];

            newFaces.push_back({v0, edgePointIndex, facePointIndex});
        }
    }

    mesh.clear();

    for (const auto& vertex : vertices) {
        mesh.addVertex(vertex);
    }

    for (const auto& pair : edgePointMap) {
        const Edge& edge = edges[pair.first];
        const Vertex& v1 = vertices[edge.vertex1];
        const Vertex& v2 = vertices[edge.vertex2];
        glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;
        mesh.addVertex(Vertex(edgePoint));
    }

    for (const auto& pair : facePointMap) {
        const Face& face = faces[pair.first];
        glm::vec3 facePoint = calculateFacePoint(face, vertices);
        mesh.addVertex(Vertex(facePoint));
    }

    for (const auto& newFace : newFaces) {
        mesh.addFace(newFace);
    }

    mesh.calculateNormals();
}

void Subdivision::linearStep(Mesh& mesh) {
    const auto& vertices = mesh.getVertices();
    const auto& faces = mesh.getFaces();
    const auto& edges = mesh.getEdges();

    std::unordered_map<int, int> edgePointMap;

    for (int i = 0; i < mesh.getEdgeCount(); ++i) {
        const Edge& edge = edges[i];
        const Vertex& v1 = vertices[edge.vertex1];
        const Vertex& v2 = vertices[edge.vertex2];

        glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;

        Vertex newVertex;
        newVertex.position = edgePoint;
        int newVertexIndex = mesh.addVertex(newVertex);
        edgePointMap[i] = newVertexIndex;
    }

    std::vector<std::vector<int>> newFaces;

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const Face& face = faces[i];

        for (size_t j = 0; j < face.vertices.size(); ++j) {
            int v0 = face.vertices[j];
            int v1 = face.vertices[(j + 1) % face.vertices.size()];
            int v2 = face.vertices[(j + 2) % face.vertices.size()];

            int edgeIndex1 = -1;
            int edgeIndex2 = -1;

            for (int ei = 0; ei < mesh.getEdgeCount(); ++ei) {
                const Edge& edge = edges[ei];
                if ((edge.vertex1 == v0 && edge.vertex2 == v1) ||
                    (edge.vertex1 == v1 && edge.vertex2 == v0)) {
                    edgeIndex1 = ei;
                }
                if ((edge.vertex1 == v1 && edge.vertex2 == v2) ||
                    (edge.vertex1 == v2 && edge.vertex2 == v1)) {
                    edgeIndex2 = ei;
                }
            }

            int edgePoint1 = edgePointMap[edgeIndex1];
            int edgePoint2 = edgePointMap[edgeIndex2];

            newFaces.push_back({v0, edgePoint1, edgePoint2});
        }
    }

    mesh.clear();

    for (const auto& vertex : vertices) {
        mesh.addVertex(vertex);
    }

    for (const auto& pair : edgePointMap) {
        const Edge& edge = edges[pair.first];
        const Vertex& v1 = vertices[edge.vertex1];
        const Vertex& v2 = vertices[edge.vertex2];
        glm::vec3 edgePoint = (v1.position + v2.position) * 0.5f;
        mesh.addVertex(Vertex(edgePoint));
    }

    for (const auto& newFace : newFaces) {
        mesh.addFace(newFace);
    }

    mesh.calculateNormals();
}

glm::vec3 Subdivision::calculateFacePoint(const Face& face, const std::vector<Vertex>& vertices) {
    glm::vec3 sum(0.0f);
    for (int vi : face.vertices) {
        sum += vertices[vi].position;
    }
    return sum / static_cast<float>(face.vertices.size());
}

glm::vec3 Subdivision::calculateEdgePoint(const Edge& edge, const std::vector<Vertex>& vertices, const std::vector<Face>& faces) {
    const Vertex& v1 = vertices[edge.vertex1];
    const Vertex& v2 = vertices[edge.vertex2];

    if (edge.isBoundary()) {
        return (v1.position + v2.position) * 0.5f;
    }

    const Face& face1 = faces[edge.face1];
    const Face& face2 = faces[edge.face2];

    glm::vec3 facePoint1 = calculateFacePoint(face1, vertices);
    glm::vec3 facePoint2 = calculateFacePoint(face2, vertices);

    return (v1.position + v2.position + facePoint1 + facePoint2) * 0.25f;
}

glm::vec3 Subdivision::calculateVertexPoint(int vertexIndex, const Mesh& mesh) {
    const Vertex& vertex = mesh.getVertex(vertexIndex);
    std::vector<int> adjacentFaces = mesh.getAdjacentFaces(vertexIndex);

    glm::vec3 facePointSum(0.0f);
    for (int fi : adjacentFaces) {
        const Face& face = mesh.getFace(fi);
        facePointSum += calculateFacePoint(face, mesh.getVertices());
    }

    std::vector<int> adjacentEdges = mesh.getAdjacentEdges(vertexIndex);
    glm::vec3 edgePointSum(0.0f);
    for (int ei : adjacentEdges) {
        const Edge& edge = mesh.getEdge(ei);
        edgePointSum += calculateEdgePoint(edge, mesh.getVertices(), mesh.getFaces());
    }

    float n = static_cast<float>(adjacentFaces.size());

    glm::vec3 Q = facePointSum / n;
    glm::vec3 R = edgePointSum / n;
    glm::vec3 S = vertex.position;

    return (Q / n) + (2.0f * R / n) + (S * (n - 3.0f) / n);
}

}
}