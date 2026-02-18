#pragma once

#include "mesh.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace v3d {
namespace modeling {

class UVEditor {
public:
    UVEditor();
    ~UVEditor();

    void setMesh(Mesh* mesh);
    Mesh* getMesh() const;

    void generateUVs(int method = 0);
    void generateBoxUVs();
    void generateSphericalUVs();
    void generateCylindricalUVs();
    void generatePlanarUVs(const glm::vec3& axis = glm::vec3(0.0f, 1.0f, 0.0f));

    void unwrapUVs(int iterations = 100);
    void relaxUVs(int iterations = 10);
    void packUVs(float padding = 0.01f);

    void scaleUVs(const glm::vec2& scale);
    void translateUVs(const glm::vec2& offset);
    void rotateUVs(float angle);

    void flipUVs(bool horizontal, bool vertical);

    void selectUVs(const std::vector<int>& indices);
    void deselectUVs();
    void invertSelection();

    std::vector<int> getSelectedUVs() const;

    void weldUVs(float threshold = 0.001f);
    void splitUVs(const std::vector<int>& indices);
    void stitchUVs(const std::vector<int>& indices);

    void alignUVs(const glm::vec2& direction);
    void distributeUVs(const glm::vec2& direction);

    void straightenUVs();
    void relaxSelectedUVs(int iterations = 10);

    void projectUVs(const glm::vec3& axis, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    void createUVIsland(const std::vector<int>& faceIndices);
    void separateUVIslands();
    void mergeUVIslands(const std::vector<int>& islandIndices);

    std::vector<std::vector<int>> getUVIslands() const;

    void optimizeUVLayout();
    void minimizeUVDistortion();

    void setUVChannel(int channel);
    int getUVChannel() const;

    void addUVChannel();
    void removeUVChannel(int channel);
    int getUVChannelCount() const;

    void copyUVs(int sourceChannel, int targetChannel);

    void normalizeUVs();
    void centerUVs();

    void bakeLightmapUVs(float texelSize = 1.0f);

    void validateUVs();
    void fixUVSeams();

private:
    struct UVIsland {
        std::vector<int> faceIndices;
        glm::vec2 minUV;
        glm::vec2 maxUV;
        glm::vec2 center;
        glm::vec2 size;
        float area;
    };

    void calculateUVIslands();
    void calculateIslandBounds(UVIsland& island);

    Mesh* mesh_;
    std::vector<std::vector<glm::vec2>> uvChannels_;
    int currentUVChannel_;
    std::vector<int> selectedUVs_;
    std::vector<UVIsland> uvIslands_;
    bool islandsDirty_;
};

}
}