#pragma once

#include "mesh.h"
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

enum class SubdivisionMethod {
    CatmullClark,
    Loop,
    DooSabin,
    Midpoint,
    Linear
};

class Subdivision {
public:
    static void subdivide(Mesh& mesh, SubdivisionMethod method = SubdivisionMethod::CatmullClark, int levels = 1);
    static void catmullClark(Mesh& mesh, int levels = 1);
    static void loop(Mesh& mesh, int levels = 1);
    static void dooSabin(Mesh& mesh, int levels = 1);
    static void midpoint(Mesh& mesh, int levels = 1);
    static void linear(Mesh& mesh, int levels = 1);

    static void smooth(Mesh& mesh, int iterations = 1);
    static void sharpen(Mesh& mesh, int iterations = 1);

    static void tessellate(Mesh& mesh, int iterations = 1);

private:
    static void catmullClarkStep(Mesh& mesh);
    static void loopStep(Mesh& mesh);
    static void dooSabinStep(Mesh& mesh);
    static void midpointStep(Mesh& mesh);
    static void linearStep(Mesh& mesh);

    static glm::vec3 calculateFacePoint(const Face& face, const std::vector<Vertex>& vertices);
    static glm::vec3 calculateEdgePoint(const Edge& edge, const std::vector<Vertex>& vertices, const std::vector<Face>& faces);
    static glm::vec3 calculateVertexPoint(int vertexIndex, const Mesh& mesh);
};

}
}