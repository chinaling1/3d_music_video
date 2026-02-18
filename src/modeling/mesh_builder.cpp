#include "mesh_builder.h"
#include <cmath>
#include <algorithm>

namespace v3d {
namespace modeling {

MeshBuilder::MeshBuilder()
    : generateNormals_(true)
    , generateTangents_(true)
    , generateUVs_(true) {
}

MeshBuilder::~MeshBuilder() {
}

void MeshBuilder::clear() {
}

Mesh MeshBuilder::createBox(const glm::vec3& size) {
    Mesh mesh;

    glm::vec3 halfSize = size * 0.5f;

    int v0 = mesh.addVertex(glm::vec3(-halfSize.x, -halfSize.y, halfSize.z));
    int v1 = mesh.addVertex(glm::vec3(halfSize.x, -halfSize.y, halfSize.z));
    int v2 = mesh.addVertex(glm::vec3(halfSize.x, halfSize.y, halfSize.z));
    int v3 = mesh.addVertex(glm::vec3(-halfSize.x, halfSize.y, halfSize.z));
    int v4 = mesh.addVertex(glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z));
    int v5 = mesh.addVertex(glm::vec3(halfSize.x, -halfSize.y, -halfSize.z));
    int v6 = mesh.addVertex(glm::vec3(halfSize.x, halfSize.y, -halfSize.z));
    int v7 = mesh.addVertex(glm::vec3(-halfSize.x, halfSize.y, -halfSize.z));

    mesh.addQuad(v3, v2, v1, v0);
    mesh.addQuad(v6, v7, v4, v5);
    mesh.addQuad(v7, v3, v0, v4);
    mesh.addQuad(v2, v6, v5, v1);
    mesh.addQuad(v7, v6, v2, v3);
    mesh.addQuad(v0, v1, v5, v4);

    if (generateNormals_) {
        mesh.calculateNormals();
    }

    if (generateTangents_) {
        mesh.calculateTangents();
    }

    if (generateUVs_) {
        auto& vertices = const_cast<std::vector<Vertex>&>(mesh.getVertices());
        vertices[v0].texCoord = glm::vec2(0.0f, 0.0f);
        vertices[v1].texCoord = glm::vec2(1.0f, 0.0f);
        vertices[v2].texCoord = glm::vec2(1.0f, 1.0f);
        vertices[v3].texCoord = glm::vec2(0.0f, 1.0f);
        vertices[v4].texCoord = glm::vec2(0.0f, 0.0f);
        vertices[v5].texCoord = glm::vec2(1.0f, 0.0f);
        vertices[v6].texCoord = glm::vec2(1.0f, 1.0f);
        vertices[v7].texCoord = glm::vec2(0.0f, 1.0f);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createSphere(float radius, int segments, int rings) {
    Mesh mesh;

    for (int ring = 0; ring <= rings; ++ring) {
        float theta = glm::pi<float>() * ring / rings;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int seg = 0; seg <= segments; ++seg) {
            float phi = 2.0f * glm::pi<float>() * seg / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 position(
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi
            );

            Vertex vertex(position);
            vertex.normal = glm::normalize(position);

            float u = static_cast<float>(seg) / segments;
            float v = static_cast<float>(ring) / rings;
            vertex.texCoord = glm::vec2(u, v);

            mesh.addVertex(vertex);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int first = ring * (segments + 1) + seg;
            int second = first + segments + 1;

            mesh.addTriangle(first, second, first + 1);
            mesh.addTriangle(second, second + 1, first + 1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createCylinder(float radius, float height, int segments) {
    Mesh mesh;

    float halfHeight = height * 0.5f;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        int v0 = mesh.addVertex(glm::vec3(x, halfHeight, z));
        int v1 = mesh.addVertex(glm::vec3(x, -halfHeight, z));

        auto& vertices = const_cast<std::vector<Vertex>&>(mesh.getVertices());
        vertices[v0].texCoord = glm::vec2(static_cast<float>(i) / segments, 1.0f);
        vertices[v1].texCoord = glm::vec2(static_cast<float>(i) / segments, 0.0f);
    }

    for (int i = 0; i < segments; ++i) {
        int v0 = i * 2;
        int v1 = v0 + 1;
        int v2 = ((i + 1) % (segments + 1)) * 2;
        int v3 = v2 + 1;

        mesh.addQuad(v0, v2, v3, v1);
    }

    int centerTop = mesh.addVertex(glm::vec3(0.0f, halfHeight, 0.0f));
    int centerBottom = mesh.addVertex(glm::vec3(0.0f, -halfHeight, 0.0f));

    for (int i = 0; i < segments; ++i) {
        int v0 = i * 2;
        int v1 = ((i + 1) % (segments + 1)) * 2;
        mesh.addTriangle(centerTop, v1, v0);

        int v2 = i * 2 + 1;
        int v3 = ((i + 1) % (segments + 1)) * 2 + 1;
        mesh.addTriangle(centerBottom, v2, v3);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createCone(float radius, float height, int segments) {
    Mesh mesh;

    float halfHeight = height * 0.5f;

    int tip = mesh.addVertex(glm::vec3(0.0f, halfHeight, 0.0f));
    int baseCenter = mesh.addVertex(glm::vec3(0.0f, -halfHeight, 0.0f));

    std::vector<int> baseVertices;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        int v = mesh.addVertex(glm::vec3(x, -halfHeight, z));
        baseVertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        mesh.addTriangle(tip, baseVertices[i + 1], baseVertices[i]);
        mesh.addTriangle(baseCenter, baseVertices[i], baseVertices[i + 1]);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments) {
    Mesh mesh;

    for (int i = 0; i <= majorSegments; ++i) {
        float u = 2.0f * glm::pi<float>() * i / majorSegments;
        float cosU = std::cos(u);
        float sinU = std::sin(u);

        for (int j = 0; j <= minorSegments; ++j) {
            float v = 2.0f * glm::pi<float>() * j / minorSegments;
            float cosV = std::cos(v);
            float sinV = std::sin(v);

            glm::vec3 position(
                (majorRadius + minorRadius * cosV) * cosU,
                minorRadius * sinV,
                (majorRadius + minorRadius * cosV) * sinU
            );

            Vertex vertex(position);
            vertex.normal = glm::normalize(glm::vec3(cosV * cosU, sinV, cosV * sinU));
            vertex.texCoord = glm::vec2(static_cast<float>(i) / majorSegments, static_cast<float>(j) / minorSegments);

            mesh.addVertex(vertex);
        }
    }

    for (int i = 0; i < majorSegments; ++i) {
        for (int j = 0; j < minorSegments; ++j) {
            int first = i * (minorSegments + 1) + j;
            int second = first + minorSegments + 1;

            mesh.addTriangle(first, second, first + 1);
            mesh.addTriangle(second, second + 1, first + 1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createPlane(const glm::vec2& size, int segmentsX, int segmentsY) {
    Mesh mesh;

    glm::vec2 halfSize = size * 0.5f;

    for (int y = 0; y <= segmentsY; ++y) {
        for (int x = 0; x <= segmentsX; ++x) {
            float px = (static_cast<float>(x) / segmentsX - 0.5f) * size.x;
            float py = (static_cast<float>(y) / segmentsY - 0.5f) * size.y;

            Vertex vertex(glm::vec3(px, 0.0f, py));
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.texCoord = glm::vec2(static_cast<float>(x) / segmentsX, static_cast<float>(y) / segmentsY);

            mesh.addVertex(vertex);
        }
    }

    for (int y = 0; y < segmentsY; ++y) {
        for (int x = 0; x < segmentsX; ++x) {
            int v0 = y * (segmentsX + 1) + x;
            int v1 = v0 + 1;
            int v2 = (y + 1) * (segmentsX + 1) + x;
            int v3 = v2 + 1;

            mesh.addQuad(v0, v2, v3, v1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createGrid(const glm::vec2& size, int divisionsX, int divisionsY) {
    return createPlane(size, divisionsX, divisionsY);
}

Mesh MeshBuilder::createCircle(float radius, int segments) {
    Mesh mesh;

    int center = mesh.addVertex(glm::vec3(0.0f, 0.0f, 0.0f));

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        mesh.addVertex(glm::vec3(x, 0.0f, z));
    }

    for (int i = 0; i < segments; ++i) {
        mesh.addTriangle(center, i + 1, i + 2);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createRing(float innerRadius, float outerRadius, int segments) {
    Mesh mesh;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);

        glm::vec3 innerPos(innerRadius * cosAngle, 0.0f, innerRadius * sinAngle);
        glm::vec3 outerPos(outerRadius * cosAngle, 0.0f, outerRadius * sinAngle);

        mesh.addVertex(innerPos);
        mesh.addVertex(outerPos);
    }

    for (int i = 0; i < segments; ++i) {
        int v0 = i * 2;
        int v1 = v0 + 1;
        int v2 = ((i + 1) % (segments + 1)) * 2;
        int v3 = v2 + 1;

        mesh.addQuad(v0, v2, v3, v1);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createCapsule(float radius, float height, int segments, int rings) {
    Mesh mesh;

    float cylinderHeight = height - 2.0f * radius;
    float halfCylinderHeight = cylinderHeight * 0.5f;

    for (int ring = 0; ring <= rings; ++ring) {
        float theta = glm::pi<float>() * ring / rings;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int seg = 0; seg <= segments; ++seg) {
            float phi = 2.0f * glm::pi<float>() * seg / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 position(
                radius * sinTheta * cosPhi,
                halfCylinderHeight + radius * cosTheta,
                radius * sinTheta * sinPhi
            );

            Vertex vertex(position);
            vertex.normal = glm::normalize(position - glm::vec3(0.0f, halfCylinderHeight, 0.0f));
            vertex.texCoord = glm::vec2(static_cast<float>(seg) / segments, static_cast<float>(ring) / rings);

            mesh.addVertex(vertex);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int first = ring * (segments + 1) + seg;
            int second = first + segments + 1;

            mesh.addTriangle(first, second, first + 1);
            mesh.addTriangle(second, second + 1, first + 1);
        }
    }

    int cylinderStart = mesh.getVertexCount();

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        int v0 = mesh.addVertex(glm::vec3(x, halfCylinderHeight, z));
        int v1 = mesh.addVertex(glm::vec3(x, -halfCylinderHeight, z));

        auto& vertices = const_cast<std::vector<Vertex>&>(mesh.getVertices());
        vertices[v0].texCoord = glm::vec2(static_cast<float>(i) / segments, 1.0f);
        vertices[v1].texCoord = glm::vec2(static_cast<float>(i) / segments, 0.0f);
    }

    for (int i = 0; i < segments; ++i) {
        int v0 = cylinderStart + i * 2;
        int v1 = v0 + 1;
        int v2 = cylinderStart + ((i + 1) % (segments + 1)) * 2;
        int v3 = v2 + 1;

        mesh.addQuad(v0, v2, v3, v1);
    }

    int bottomSphereStart = mesh.getVertexCount();

    for (int ring = 0; ring <= rings; ++ring) {
        float theta = glm::pi<float>() * ring / rings;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int seg = 0; seg <= segments; ++seg) {
            float phi = 2.0f * glm::pi<float>() * seg / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 position(
                radius * sinTheta * cosPhi,
                -halfCylinderHeight - radius * cosTheta,
                radius * sinTheta * sinPhi
            );

            Vertex vertex(position);
            vertex.normal = glm::normalize(position - glm::vec3(0.0f, -halfCylinderHeight, 0.0f));
            vertex.texCoord = glm::vec2(static_cast<float>(seg) / segments, static_cast<float>(ring) / rings);

            mesh.addVertex(vertex);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int first = bottomSphereStart + ring * (segments + 1) + seg;
            int second = first + segments + 1;

            mesh.addTriangle(first, second, first + 1);
            mesh.addTriangle(second, second + 1, first + 1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createIcosahedron(float radius) {
    Mesh mesh;

    float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    std::vector<glm::vec3> positions = {
        glm::vec3(-1, t, 0), glm::vec3(1, t, 0), glm::vec3(-1, -t, 0), glm::vec3(1, -t, 0),
        glm::vec3(0, -1, t), glm::vec3(0, 1, t), glm::vec3(0, -1, -t), glm::vec3(0, 1, -t),
        glm::vec3(t, 0, -1), glm::vec3(t, 0, 1), glm::vec3(-t, 0, -1), glm::vec3(-t, 0, 1)
    };

    for (const auto& pos : positions) {
        mesh.addVertex(glm::normalize(pos) * radius);
    }

    std::vector<std::vector<int>> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    for (const auto& face : faces) {
        mesh.addTriangle(face[0], face[1], face[2]);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createDodecahedron(float radius) {
    Mesh mesh;

    float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
    float invPhi = 1.0f / phi;

    std::vector<glm::vec3> positions = {
        glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1),
        glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1),
        glm::vec3(0, invPhi, phi), glm::vec3(0, invPhi, -phi), glm::vec3(0, -invPhi, phi), glm::vec3(0, -invPhi, -phi),
        glm::vec3(invPhi, phi, 0), glm::vec3(invPhi, -phi, 0), glm::vec3(-invPhi, phi, 0), glm::vec3(-invPhi, -phi, 0),
        glm::vec3(phi, 0, invPhi), glm::vec3(phi, 0, -invPhi), glm::vec3(-phi, 0, invPhi), glm::vec3(-phi, 0, -invPhi)
    };

    for (const auto& pos : positions) {
        mesh.addVertex(glm::normalize(pos) * radius);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createOctahedron(float radius) {
    Mesh mesh;

    std::vector<glm::vec3> positions = {
        glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
        glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
        glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
    };

    for (const auto& pos : positions) {
        mesh.addVertex(pos * radius);
    }

    std::vector<std::vector<int>> faces = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {1, 4, 2}, {1, 3, 4}, {1, 5, 3}, {1, 2, 5}
    };

    for (const auto& face : faces) {
        mesh.addTriangle(face[0], face[1], face[2]);
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createTetrahedron(float radius) {
    Mesh mesh;

    float a = 1.0f / std::sqrt(3.0f);

    std::vector<glm::vec3> positions = {
        glm::vec3(a, a, a),
        glm::vec3(-a, -a, a),
        glm::vec3(-a, a, -a),
        glm::vec3(a, -a, -a)
    };

    for (const auto& pos : positions) {
        mesh.addVertex(pos * radius);
    }

    mesh.addTriangle(0, 1, 2);
    mesh.addTriangle(0, 3, 1);
    mesh.addTriangle(0, 2, 3);
    mesh.addTriangle(1, 3, 2);

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createExtrusion(const std::vector<glm::vec2>& profile, float depth, int segments) {
    Mesh mesh;

    if (profile.size() < 3) {
        return mesh;
    }

    float halfDepth = depth * 0.5f;

    for (int seg = 0; seg <= segments; ++seg) {
        float z = (static_cast<float>(seg) / segments - 0.5f) * depth;

        for (const auto& point : profile) {
            Vertex vertex(glm::vec3(point.x, point.y, z));
            vertex.texCoord = glm::vec2(point.x, point.y);
            mesh.addVertex(vertex);
        }
    }

    int profileSize = static_cast<int>(profile.size());

    for (int seg = 0; seg < segments; ++seg) {
        for (int i = 0; i < profileSize; ++i) {
            int v0 = seg * profileSize + i;
            int v1 = v0 + profileSize;
            int v2 = ((i + 1) % profileSize) + seg * profileSize;
            int v3 = v2 + profileSize;

            mesh.addQuad(v0, v2, v3, v1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createLathe(const std::vector<glm::vec2>& profile, int segments, float angle) {
    Mesh mesh;

    if (profile.size() < 2) {
        return mesh;
    }

    float angleRad = glm::radians(angle);

    for (int seg = 0; seg <= segments; ++seg) {
        float theta = angleRad * seg / segments;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        for (const auto& point : profile) {
            glm::vec3 position(
                point.x * cosTheta,
                point.y,
                point.x * sinTheta
            );

            Vertex vertex(position);
            vertex.normal = glm::normalize(glm::vec3(cosTheta, 0.0f, sinTheta));
            vertex.texCoord = glm::vec2(static_cast<float>(seg) / segments, point.y);

            mesh.addVertex(vertex);
        }
    }

    int profileSize = static_cast<int>(profile.size());

    for (int seg = 0; seg < segments; ++seg) {
        for (int i = 0; i < profileSize - 1; ++i) {
            int v0 = seg * profileSize + i;
            int v1 = v0 + profileSize;
            int v2 = seg * profileSize + i + 1;
            int v3 = v2 + profileSize;

            mesh.addQuad(v0, v2, v3, v1);
        }
    }

    mesh.recalculateAll();
    return mesh;
}

Mesh MeshBuilder::createRevolution(const std::vector<glm::vec2>& profile, int segments) {
    return createLathe(profile, segments, 360.0f);
}

Mesh MeshBuilder::createText(const std::string& text, float size, float depth) {
    Mesh mesh;

    return mesh;
}

void MeshBuilder::setGenerateNormals(bool generate) {
    generateNormals_ = generate;
}

void MeshBuilder::setGenerateTangents(bool generate) {
    generateTangents_ = generate;
}

void MeshBuilder::setGenerateUVs(bool generate) {
    generateUVs_ = generate;
}

void MeshBuilder::generateNormals(Mesh& mesh) {
    mesh.calculateNormals();
}

void MeshBuilder::generateTangents(Mesh& mesh) {
    mesh.calculateTangents();
}

void MeshBuilder::generateUVs(Mesh& mesh) {
}

}
}