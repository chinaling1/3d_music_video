#include <gtest/gtest.h>
#include "../../src/modeling/mesh.h"
#include "../utils/test_data_generator.h"
#include "../utils/test_utils.h"
#include <glm/glm.hpp>

using namespace v3d::modeling;
using namespace v3d::test;

class MeshTest : public ::testing::Test {
protected:
    void SetUp() override {
        mesh_ = std::make_shared<Mesh>();
    }

    void TearDown() override {
    }

    std::shared_ptr<Mesh> mesh_;
};

TEST_F(MeshTest, AddVertex) {
    Vertex vertex;
    vertex.position = glm::vec3(1.0f, 2.0f, 3.0f);
    
    int index = mesh_->addVertex(vertex);
    
    EXPECT_EQ(index, 0);
    EXPECT_EQ(mesh_->getVertexCount(), 1);
    
    const Vertex& result = mesh_->getVertex(0);
    EXPECT_EQ(result.position, vertex.position);
}

TEST_F(MeshTest, AddVertexPositionOnly) {
    int index = mesh_->addVertex(glm::vec3(1.0f, 2.0f, 3.0f));
    
    EXPECT_EQ(index, 0);
    EXPECT_EQ(mesh_->getVertexCount(), 1);
    
    const Vertex& result = mesh_->getVertex(0);
    EXPECT_EQ(result.position, glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(MeshTest, AddTriangle) {
    mesh_->addVertex(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(1.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(0.5f, 1.0f, 0.0f));
    
    int faceIndex = mesh_->addTriangle(0, 1, 2);
    
    EXPECT_EQ(faceIndex, 0);
    EXPECT_EQ(mesh_->getFaceCount(), 1);
    
    const Face& face = mesh_->getFace(0);
    EXPECT_EQ(face.vertices.size(), 3);
    EXPECT_EQ(face.vertices[0], 0);
    EXPECT_EQ(face.vertices[1], 1);
    EXPECT_EQ(face.vertices[2], 2);
}

TEST_F(MeshTest, AddQuad) {
    mesh_->addVertex(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(1.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(1.0f, 1.0f, 0.0f));
    mesh_->addVertex(glm::vec3(0.0f, 1.0f, 0.0f));
    
    int faceIndex = mesh_->addQuad(0, 1, 2, 3);
    
    EXPECT_EQ(faceIndex, 0);
    EXPECT_EQ(mesh_->getFaceCount(), 1);
    
    const Face& face = mesh_->getFace(0);
    EXPECT_EQ(face.vertices.size(), 4);
}

TEST_F(MeshTest, CalculateNormals) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    cube->calculateNormals();
    
    for (int i = 0; i < cube->getVertexCount(); ++i) {
        const Vertex& vertex = cube->getVertex(i);
        float length = glm::length(vertex.normal);
        EXPECT_NEAR(length, 1.0f, 0.001f);
    }
}

TEST_F(MeshTest, CalculateBoundingBox) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    cube->calculateBoundingBox();
    
    glm::vec3 min = cube->getMin();
    glm::vec3 max = cube->getMax();
    
    EXPECT_FLOAT_EQ(min.x, -0.5f);
    EXPECT_FLOAT_EQ(min.y, -0.5f);
    EXPECT_FLOAT_EQ(min.z, -0.5f);
    
    EXPECT_FLOAT_EQ(max.x, 0.5f);
    EXPECT_FLOAT_EQ(max.y, 0.5f);
    EXPECT_FLOAT_EQ(max.z, 0.5f);
}

TEST_F(MeshTest, GetCenter) {
    auto cube = TestDataGenerator::createCubeMesh();
    cube->calculateBoundingBox();
    
    glm::vec3 center = cube->getCenter();
    
    EXPECT_FLOAT_EQ(center.x, 0.0f);
    EXPECT_FLOAT_EQ(center.y, 0.0f);
    EXPECT_FLOAT_EQ(center.z, 0.0f);
}

TEST_F(MeshTest, Translate) {
    auto cube = TestDataGenerator::createCubeMesh();
    cube->calculateBoundingBox();
    
    glm::vec3 offset(10.0f, 20.0f, 30.0f);
    cube->translate(offset);
    
    cube->calculateBoundingBox();
    glm::vec3 center = cube->getCenter();
    
    EXPECT_FLOAT_EQ(center.x, 10.0f);
    EXPECT_FLOAT_EQ(center.y, 20.0f);
    EXPECT_FLOAT_EQ(center.z, 30.0f);
}

TEST_F(MeshTest, Rotate) {
    auto cube = TestDataGenerator::createCubeMesh();
    cube->calculateBoundingBox();
    
    cube->rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    
    cube->calculateBoundingBox();
    glm::vec3 center = cube->getCenter();
    
    EXPECT_FLOAT_EQ(center.x, 0.0f);
    EXPECT_FLOAT_EQ(center.y, 0.0f);
    EXPECT_FLOAT_EQ(center.z, 0.0f);
}

TEST_F(MeshTest, Scale) {
    auto cube = TestDataGenerator::createCubeMesh();
    cube->calculateBoundingBox();
    
    glm::vec3 scaleFactor(2.0f, 2.0f, 2.0f);
    cube->scale(scaleFactor);
    
    cube->calculateBoundingBox();
    glm::vec3 size = cube->getSize();
    
    EXPECT_FLOAT_EQ(size.x, 2.0f);
    EXPECT_FLOAT_EQ(size.y, 2.0f);
    EXPECT_FLOAT_EQ(size.z, 2.0f);
}

TEST_F(MeshTest, MergeMeshes) {
    auto cube1 = TestDataGenerator::createCubeMesh();
    auto cube2 = TestDataGenerator::createCubeMesh();
    
    cube2->translate(glm::vec3(2.0f, 0.0f, 0.0f));
    
    int vertexCount1 = cube1->getVertexCount();
    int vertexCount2 = cube2->getVertexCount();
    
    cube1->merge(*cube2);
    
    EXPECT_EQ(cube1->getVertexCount(), vertexCount1 + vertexCount2);
}

TEST_F(MeshTest, IsValid) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    EXPECT_TRUE(cube->isValid());
    
    Mesh emptyMesh;
    EXPECT_FALSE(emptyMesh.isValid());
}

TEST_F(MeshTest, InvertNormals) {
    auto cube = TestDataGenerator::createCubeMesh();
    cube->calculateNormals();
    
    glm::vec3 normalBefore = cube->getVertex(0).normal;
    
    cube->invertNormals();
    
    glm::vec3 normalAfter = cube->getVertex(0).normal;
    
    EXPECT_EQ(normalAfter, -normalBefore);
}

TEST_F(MeshTest, FlipFaces) {
    mesh_->addVertex(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(1.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(0.5f, 1.0f, 0.0f));
    
    mesh_->addTriangle(0, 1, 2);
    
    const Face& faceBefore = mesh_->getFace(0);
    
    mesh_->flipFaces();
    
    const Face& faceAfter = mesh_->getFace(0);
    
    EXPECT_EQ(faceAfter.vertices[0], 2);
    EXPECT_EQ(faceAfter.vertices[1], 1);
    EXPECT_EQ(faceAfter.vertices[2], 0);
}

TEST_F(MeshTest, GetAdjacentVertices) {
    mesh_->addVertex(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(1.0f, 0.0f, 0.0f));
    mesh_->addVertex(glm::vec3(0.5f, 1.0f, 0.0f));
    
    mesh_->addTriangle(0, 1, 2);
    
    auto adjacent = mesh_->getAdjacentVertices(0);
    
    EXPECT_EQ(adjacent.size(), 2);
    EXPECT_TRUE(std::find(adjacent.begin(), adjacent.end(), 1) != adjacent.end());
    EXPECT_TRUE(std::find(adjacent.begin(), adjacent.end(), 2) != adjacent.end());
}

TEST_F(MeshTest, Clear) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    EXPECT_GT(cube->getVertexCount(), 0);
    EXPECT_GT(cube->getFaceCount(), 0);
    
    cube->clear();
    
    EXPECT_EQ(cube->getVertexCount(), 0);
    EXPECT_EQ(cube->getFaceCount(), 0);
}

TEST_F(MeshTest, SetGetName) {
    mesh_->setName("TestMesh");
    
    EXPECT_EQ(mesh_->getName(), "TestMesh");
}

TEST_F(MeshTest, SetGetMaterialId) {
    mesh_->setMaterialId(42);
    
    EXPECT_EQ(mesh_->getMaterialId(), 42);
}

TEST_F(MeshTest, SmoothShading) {
    mesh_->setSmoothShading(true);
    
    EXPECT_TRUE(mesh_->isSmoothShading());
    
    mesh_->setSmoothShading(false);
    
    EXPECT_FALSE(mesh_->isSmoothShading());
}

TEST_F(MeshTest, LargeMesh) {
    const int segments = 100;
    
    TestUtils::measureTime("Create large sphere mesh", [segments]() {
        auto sphere = TestDataGenerator::createSphereMesh(segments);
        
        EXPECT_GT(sphere->getVertexCount(), 0);
        EXPECT_GT(sphere->getFaceCount(), 0);
    });
}

TEST_F(MeshTest, StressTest_ManyOperations) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    TestUtils::measureTime("Many mesh operations", [cube]() {
        for (int i = 0; i < 1000; ++i) {
            cube->translate(glm::vec3(0.001f, 0.0f, 0.0f));
            cube->calculateNormals();
            cube->calculateBoundingBox();
        }
    });
}
