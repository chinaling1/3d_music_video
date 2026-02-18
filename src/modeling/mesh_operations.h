#pragma once

#include "mesh.h"
#include <glm/glm.hpp>
#include <functional>

namespace v3d {
namespace modeling {

class MeshOperations {
public:
    static void extrude(Mesh& mesh, const std::vector<int>& faceIndices, float distance);
    static void extrude(Mesh& mesh, const std::vector<int>& vertexIndices, const glm::vec3& direction, float distance);

    static void bevelEdges(Mesh& mesh, const std::vector<int>& edgeIndices, float amount);
    static void bevelVertices(Mesh& mesh, const std::vector<int>& vertexIndices, float amount);

    static void inset(Mesh& mesh, const std::vector<int>& faceIndices, float amount);

    static void bridge(Mesh& mesh, const std::vector<int>& edgeLoop1, const std::vector<int>& edgeLoop2);

    static void cut(Mesh& mesh, const glm::vec3& point, const glm::vec3& normal);
    static void knife(Mesh& mesh, const glm::vec3& start, const glm::vec3& end);

    static void booleanUnion(Mesh& result, const Mesh& a, const Mesh& b);
    static void booleanIntersection(Mesh& result, const Mesh& a, const Mesh& b);
    static void booleanDifference(Mesh& result, const Mesh& a, const Mesh& b);

    static void mergeVertices(Mesh& mesh, float threshold = 0.0001f);
    static void mergeVertices(Mesh& mesh, const std::vector<int>& vertices);

    static void splitEdge(Mesh& mesh, int edgeIndex);
    static void splitFace(Mesh& mesh, int faceIndex, const glm::vec3& point);

    static void collapseEdge(Mesh& mesh, int edgeIndex);
    static void collapseVertex(Mesh& mesh, int vertexIndex);

    static void dissolveEdge(Mesh& mesh, int edgeIndex);
    static void dissolveFace(Mesh& mesh, int faceIndex);

    static void weld(Mesh& mesh, const std::vector<int>& vertices);
    static void bridgeLoops(Mesh& mesh, const std::vector<int>& loop1, const std::vector<int>& loop2);

    static void smooth(Mesh& mesh, int iterations = 1, float factor = 0.5f);
    static void sharpen(Mesh& mesh, int iterations = 1, float factor = 0.5f);

    static void deform(Mesh& mesh, std::function<void(glm::vec3&)> deformer);
    static void twist(Mesh& mesh, const glm::vec3& axis, float angle, float falloff = 0.0f);
    static void bend(Mesh& mesh, const glm::vec3& axis, float angle, float falloff = 0.0f);
    static void taper(Mesh& mesh, const glm::vec3& axis, float factor);
    static void bulge(Mesh& mesh, const glm::vec3& center, float radius, float strength);
    static void noise(Mesh& mesh, float strength, int seed = 0);

    static void mirror(Mesh& mesh, const glm::vec3& axis, bool merge = true);
    static void array(Mesh& mesh, const glm::vec3& offset, int count);
    static void radialArray(Mesh& mesh, const glm::vec3& axis, int count, float angle = 360.0f);

    static void projectToPlane(Mesh& mesh, const glm::vec3& point, const glm::vec3& normal);
    static void projectToSphere(Mesh& mesh, const glm::vec3& center, float radius);
    static void projectToCylinder(Mesh& mesh, const glm::vec3& axis, float radius);

    static void triangulate(Mesh& mesh);
    static void quadrangulate(Mesh& mesh);

    static void calculateConvexHull(Mesh& mesh);

    static void relax(Mesh& mesh, int iterations = 10);
    static void uniformRelax(Mesh& mesh, int iterations = 10);

    static void shrink(Mesh& mesh, float amount);
    static void expand(Mesh& mesh, float amount);

private:
    static glm::vec3 calculateFaceNormal(const std::vector<glm::vec3>& vertices);
    static glm::vec3 calculateFaceCentroid(const std::vector<glm::vec3>& vertices);
};

}
}