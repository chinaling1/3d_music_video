#pragma once

#include "mesh.h"
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

class MeshBuilder {
public:
    MeshBuilder();
    ~MeshBuilder();

    void clear();

    Mesh createBox(const glm::vec3& size = glm::vec3(1.0f));
    Mesh createSphere(float radius = 1.0f, int segments = 32, int rings = 16);
    Mesh createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);
    Mesh createCone(float radius = 1.0f, float height = 2.0f, int segments = 32);
    Mesh createTorus(float majorRadius = 1.0f, float minorRadius = 0.3f, int majorSegments = 32, int minorSegments = 16);
    Mesh createPlane(const glm::vec2& size = glm::vec2(1.0f), int segmentsX = 1, int segmentsY = 1);
    Mesh createGrid(const glm::vec2& size = glm::vec2(10.0f), int divisionsX = 10, int divisionsY = 10);
    Mesh createCircle(float radius = 1.0f, int segments = 32);
    Mesh createRing(float innerRadius = 0.5f, float outerRadius = 1.0f, int segments = 32);
    Mesh createCapsule(float radius = 1.0f, float height = 2.0f, int segments = 32, int rings = 8);

    Mesh createIcosahedron(float radius = 1.0f);
    Mesh createDodecahedron(float radius = 1.0f);
    Mesh createOctahedron(float radius = 1.0f);
    Mesh createTetrahedron(float radius = 1.0f);

    Mesh createExtrusion(const std::vector<glm::vec2>& profile, float depth, int segments = 1);
    Mesh createLathe(const std::vector<glm::vec2>& profile, int segments = 32, float angle = 360.0f);
    Mesh createRevolution(const std::vector<glm::vec2>& profile, int segments = 32);

    Mesh createText(const std::string& text, float size = 1.0f, float depth = 0.2f);

    void setGenerateNormals(bool generate);
    void setGenerateTangents(bool generate);
    void setGenerateUVs(bool generate);

private:
    void generateNormals(Mesh& mesh);
    void generateTangents(Mesh& mesh);
    void generateUVs(Mesh& mesh);

    bool generateNormals_;
    bool generateTangents_;
    bool generateUVs_;
};

}
}