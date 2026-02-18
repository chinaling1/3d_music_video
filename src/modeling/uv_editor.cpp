#include "uv_editor.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

UVEditor::UVEditor()
    : mesh_(nullptr)
    , currentUVChannel_(0)
    , islandsDirty_(true) {
    uvChannels_.push_back(std::vector<glm::vec2>());
}

UVEditor::~UVEditor() {
}

void UVEditor::setMesh(Mesh* mesh) {
    mesh_ = mesh;
    islandsDirty_ = true;

    if (mesh_) {
        uvChannels_[0].resize(mesh_->getVertexCount());
        for (int i = 0; i < mesh_->getVertexCount(); ++i) {
            uvChannels_[0][i] = mesh_->getVertex(i).texCoord;
        }
    }
}

Mesh* UVEditor::getMesh() const {
    return mesh_;
}

void UVEditor::generateUVs(int method) {
    if (!mesh_) {
        return;
    }

    switch (method) {
        case 0:
            generateBoxUVs();
            break;
        case 1:
            generateSphericalUVs();
            break;
        case 2:
            generateCylindricalUVs();
            break;
        case 3:
            generatePlanarUVs();
            break;
        default:
            generateBoxUVs();
            break;
    }
}

void UVEditor::generateBoxUVs() {
    if (!mesh_) {
        return;
    }

    glm::vec3 center = mesh_->getCenter();
    glm::vec3 size = mesh_->getSize();

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        Vertex vertex = mesh_->getVertex(i);
        glm::vec3 pos = vertex.position - center;

        glm::vec3 absPos = glm::abs(pos);
        glm::vec2 uv;

        if (absPos.x > absPos.y && absPos.x > absPos.z) {
            uv = glm::vec2(pos.z, pos.y);
            if (pos.x < 0) {
                uv.x = -uv.x;
            }
        } else if (absPos.y > absPos.x && absPos.y > absPos.z) {
            uv = glm::vec2(pos.x, pos.z);
            if (pos.y < 0) {
                uv.y = -uv.y;
            }
        } else {
            uv = glm::vec2(pos.x, pos.y);
            if (pos.z < 0) {
                uv.x = -uv.x;
            }
        }

        uv = (uv + glm::vec2(size.x, size.y) * 0.5f) / glm::vec2(size.x, size.y);
        uvChannels_[currentUVChannel_][i] = uv;
    }
}

void UVEditor::generateSphericalUVs() {
    if (!mesh_) {
        return;
    }

    glm::vec3 center = mesh_->getCenter();
    float radius = glm::length(mesh_->getSize()) * 0.5f;

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        Vertex vertex = mesh_->getVertex(i);
        glm::vec3 pos = glm::normalize(vertex.position - center);

        float u = std::atan2(pos.z, pos.x) / (2.0f * glm::pi<float>()) + 0.5f;
        float v = std::asin(pos.y) / glm::pi<float>() + 0.5f;

        uvChannels_[currentUVChannel_][i] = glm::vec2(u, v);
    }
}

void UVEditor::generateCylindricalUVs() {
    if (!mesh_) {
        return;
    }

    glm::vec3 center = mesh_->getCenter();
    glm::vec3 size = mesh_->getSize();
    float radius = std::max(size.x, size.z) * 0.5f;

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        Vertex vertex = mesh_->getVertex(i);
        glm::vec3 pos = vertex.position - center;

        float u = std::atan2(pos.z, pos.x) / (2.0f * glm::pi<float>()) + 0.5f;
        float v = (pos.y + size.y * 0.5f) / size.y;

        uvChannels_[currentUVChannel_][i] = glm::vec2(u, v);
    }
}

void UVEditor::generatePlanarUVs(const glm::vec3& axis) {
    if (!mesh_) {
        return;
    }

    glm::vec3 normalizedAxis = glm::normalize(axis);
    glm::vec3 center = mesh_->getCenter();
    glm::vec3 size = mesh_->getSize();

    glm::vec3 uAxis, vAxis;

    if (std::abs(normalizedAxis.x) > 0.9f) {
        uAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        vAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    } else if (std::abs(normalizedAxis.y) > 0.9f) {
        uAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        vAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        uAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        vAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        Vertex vertex = mesh_->getVertex(i);
        glm::vec3 pos = vertex.position - center;

        float u = glm::dot(pos, uAxis);
        float v = glm::dot(pos, vAxis);

        uvChannels_[currentUVChannel_][i] = glm::vec2(u, v);
    }
}

void UVEditor::unwrapUVs(int iterations) {
    if (!mesh_) {
        return;
    }

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<glm::vec2> newUVs = uvChannels_[currentUVChannel_];

        for (int i = 0; i < mesh_->getVertexCount(); ++i) {
            std::vector<int> adjacentVertices = mesh_->getAdjacentVertices(i);

            if (adjacentVertices.empty()) {
                continue;
            }

            glm::vec2 sum(0.0f);
            for (int av : adjacentVertices) {
                sum += uvChannels_[currentUVChannel_][av];
            }
            sum /= static_cast<float>(adjacentVertices.size());

            newUVs[i] = glm::mix(uvChannels_[currentUVChannel_][i], sum, 0.5f);
        }

        uvChannels_[currentUVChannel_] = newUVs;
    }
}

void UVEditor::relaxUVs(int iterations) {
    unwrapUVs(iterations);
}

void UVEditor::packUVs(float padding) {
    if (!mesh_ || uvChannels_.empty()) {
        return;
    }

    calculateUVIslands();

    struct PackedIsland {
        int index;
        glm::vec2 position;
        glm::vec2 size;
    };

    std::vector<PackedIsland> packedIslands;

    for (size_t i = 0; i < uvIslands_.size(); ++i) {
        PackedIsland packed;
        packed.index = static_cast<int>(i);
        packed.size = uvIslands_[i].maxUV - uvIslands_[i].minUV;
        packed.size += padding * 2.0f;
        packedIslands.push_back(packed);
    }

    std::sort(packedIslands.begin(), packedIslands.end(),
        [](const PackedIsland& a, const PackedIsland& b) {
            return a.size.x * a.size.y > b.size.x * b.size.y;
        });

    glm::vec2 cursor(0.0f, 0.0f);
    float rowHeight = 0.0f;

    for (auto& packed : packedIslands) {
        packed.position = cursor;

        cursor.x += packed.size.x;
        rowHeight = std::max(rowHeight, packed.size.y);

        if (cursor.x > 1.0f) {
            cursor.x = 0.0f;
            cursor.y += rowHeight;
            rowHeight = 0.0f;
        }
    }

    for (const auto& packed : packedIslands) {
        const UVIsland& island = uvIslands_[packed.index];
        glm::vec2 offset = packed.position - island.minUV + padding;

        for (int faceIndex : island.faceIndices) {
            const Face& face = mesh_->getFace(faceIndex);
            for (int vertexIndex : face.vertices) {
                uvChannels_[currentUVChannel_][vertexIndex] += offset;
            }
        }
    }

    normalizeUVs();
}

void UVEditor::scaleUVs(const glm::vec2& scale) {
    if (uvChannels_.empty()) {
        return;
    }

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        uv *= scale;
    }
}

void UVEditor::translateUVs(const glm::vec2& offset) {
    if (uvChannels_.empty()) {
        return;
    }

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        uv += offset;
    }
}

void UVEditor::rotateUVs(float angle) {
    if (uvChannels_.empty()) {
        return;
    }

    float cosAngle = std::cos(angle);
    float sinAngle = std::sin(angle);

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        glm::vec2 rotated;
        rotated.x = uv.x * cosAngle - uv.y * sinAngle;
        rotated.y = uv.x * sinAngle + uv.y * cosAngle;
        uv = rotated;
    }
}

void UVEditor::flipUVs(bool horizontal, bool vertical) {
    if (uvChannels_.empty()) {
        return;
    }

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        if (horizontal) {
            uv.x = 1.0f - uv.x;
        }
        if (vertical) {
            uv.y = 1.0f - uv.y;
        }
    }
}

void UVEditor::selectUVs(const std::vector<int>& indices) {
    selectedUVs_ = indices;
}

void UVEditor::deselectUVs() {
    selectedUVs_.clear();
}

void UVEditor::invertSelection() {
    std::vector<int> allIndices;
    if (!uvChannels_.empty()) {
        allIndices.resize(uvChannels_[currentUVChannel_].size());
        for (size_t i = 0; i < allIndices.size(); ++i) {
            allIndices[i] = static_cast<int>(i);
        }
    }

    std::vector<int> newSelection;
    for (int index : allIndices) {
        if (std::find(selectedUVs_.begin(), selectedUVs_.end(), index) == selectedUVs_.end()) {
            newSelection.push_back(index);
        }
    }

    selectedUVs_ = newSelection;
}

std::vector<int> UVEditor::getSelectedUVs() const {
    return selectedUVs_;
}

void UVEditor::weldUVs(float threshold) {
    if (uvChannels_.empty()) {
        return;
    }

    for (size_t i = 0; i < uvChannels_[currentUVChannel_].size(); ++i) {
        for (size_t j = i + 1; j < uvChannels_[currentUVChannel_].size(); ++j) {
            if (glm::length(uvChannels_[currentUVChannel_][i] - uvChannels_[currentUVChannel_][j]) < threshold) {
                uvChannels_[currentUVChannel_][j] = uvChannels_[currentUVChannel_][i];
            }
        }
    }
}

void UVEditor::splitUVs(const std::vector<int>& indices) {
    if (!mesh_ || uvChannels_.empty()) {
        return;
    }

    for (int index : indices) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            Vertex vertex = mesh_->getVertex(index);
            vertex.texCoord = uvChannels_[currentUVChannel_][index];
            int newIndex = mesh_->addVertex(vertex);
            uvChannels_[currentUVChannel_].push_back(uvChannels_[currentUVChannel_][index]);
        }
    }
}

void UVEditor::stitchUVs(const std::vector<int>& indices) {
    if (indices.empty() || uvChannels_.empty()) {
        return;
    }

    glm::vec2 average(0.0f);
    for (int index : indices) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            average += uvChannels_[currentUVChannel_][index];
        }
    }
    average /= static_cast<float>(indices.size());

    for (int index : indices) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            uvChannels_[currentUVChannel_][index] = average;
        }
    }
}

void UVEditor::alignUVs(const glm::vec2& direction) {
    if (selectedUVs_.empty() || uvChannels_.empty()) {
        return;
    }

    glm::vec2 normalizedDir = glm::normalize(direction);

    float minProjection = std::numeric_limits<float>::max();
    float maxProjection = std::numeric_limits<float>::lowest();

    for (int index : selectedUVs_) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            float projection = glm::dot(uvChannels_[currentUVChannel_][index], normalizedDir);
            minProjection = std::min(minProjection, projection);
            maxProjection = std::max(maxProjection, projection);
        }
    }

    float targetProjection = (minProjection + maxProjection) * 0.5f;

    for (int index : selectedUVs_) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            glm::vec2 uv = uvChannels_[currentUVChannel_][index];
            float currentProjection = glm::dot(uv, normalizedDir);
            uv += (targetProjection - currentProjection) * normalizedDir;
            uvChannels_[currentUVChannel_][index] = uv;
        }
    }
}

void UVEditor::distributeUVs(const glm::vec2& direction) {
    if (selectedUVs_.size() < 2 || uvChannels_.empty()) {
        return;
    }

    glm::vec2 normalizedDir = glm::normalize(direction);

    std::vector<std::pair<float, int>> projections;
    for (int index : selectedUVs_) {
        if (index >= 0 && index < static_cast<int>(uvChannels_[currentUVChannel_].size())) {
            float projection = glm::dot(uvChannels_[currentUVChannel_][index], normalizedDir);
            projections.push_back({projection, index});
        }
    }

    std::sort(projections.begin(), projections.end());

    float minProj = projections.front().first;
    float maxProj = projections.back().first;
    float range = maxProj - minProj;

    if (range < 0.0001f) {
        return;
    }

    for (size_t i = 0; i < projections.size(); ++i) {
        int index = projections[i].second;
        float targetProjection = minProj + static_cast<float>(i) / (projections.size() - 1) * range;

        glm::vec2 uv = uvChannels_[currentUVChannel_][index];
        float currentProjection = glm::dot(uv, normalizedDir);
        uv += (targetProjection - currentProjection) * normalizedDir;
        uvChannels_[currentUVChannel_][index] = uv;
    }
}

void UVEditor::straightenUVs() {
    if (selectedUVs_.size() < 2 || uvChannels_.empty()) {
        return;
    }

    glm::vec2 start = uvChannels_[currentUVChannel_][selectedUVs_[0]];
    glm::vec2 end = uvChannels_[currentUVChannel_][selectedUVs_.back()];

    glm::vec2 direction = glm::normalize(end - start);

    for (size_t i = 1; i < selectedUVs_.size() - 1; ++i) {
        int index = selectedUVs_[i];
        float t = static_cast<float>(i) / (selectedUVs_.size() - 1);
        uvChannels_[currentUVChannel_][index] = start + direction * t * glm::length(end - start);
    }
}

void UVEditor::relaxSelectedUVs(int iterations) {
    if (selectedUVs_.empty() || !mesh_) {
        return;
    }

    for (int iter = 0; iter < iterations; ++iter) {
        std::unordered_map<int, glm::vec2> newUVs;

        for (int index : selectedUVs_) {
            std::vector<int> adjacentVertices = mesh_->getAdjacentVertices(index);

            if (adjacentVertices.empty()) {
                continue;
            }

            glm::vec2 sum(0.0f);
            int count = 0;

            for (int av : adjacentVertices) {
                if (std::find(selectedUVs_.begin(), selectedUVs_.end(), av) != selectedUVs_.end()) {
                    sum += uvChannels_[currentUVChannel_][av];
                    count++;
                }
            }

            if (count > 0) {
                newUVs[index] = glm::mix(uvChannels_[currentUVChannel_][index], sum / static_cast<float>(count), 0.5f);
            }
        }

        for (const auto& pair : newUVs) {
            uvChannels_[currentUVChannel_][pair.first] = pair.second;
        }
    }
}

void UVEditor::projectUVs(const glm::vec3& axis, const glm::vec3& up) {
    if (!mesh_) {
        return;
    }

    glm::vec3 normalizedAxis = glm::normalize(axis);
    glm::vec3 normalizedUp = glm::normalize(up);
    glm::vec3 right = glm::cross(normalizedUp, normalizedAxis);

    glm::vec3 center = mesh_->getCenter();
    glm::vec3 size = mesh_->getSize();

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        Vertex vertex = mesh_->getVertex(i);
        glm::vec3 pos = vertex.position - center;

        float u = glm::dot(pos, right);
        float v = glm::dot(pos, normalizedUp);

        uvChannels_[currentUVChannel_][i] = glm::vec2(u, v);
    }
}

void UVEditor::createUVIsland(const std::vector<int>& faceIndices) {
}

void UVEditor::separateUVIslands() {
    if (!mesh_) {
        return;
    }

    calculateUVIslands();

    for (size_t i = 0; i < uvIslands_.size(); ++i) {
        UVIsland& island = uvIslands_[i];
        glm::vec2 offset = glm::vec2(static_cast<float>(i) * 0.1f, 0.0f);

        for (int faceIndex : island.faceIndices) {
            const Face& face = mesh_->getFace(faceIndex);
            for (int vertexIndex : face.vertices) {
                uvChannels_[currentUVChannel_][vertexIndex] += offset;
            }
        }
    }
}

void UVEditor::mergeUVIslands(const std::vector<int>& islandIndices) {
}

std::vector<std::vector<int>> UVEditor::getUVIslands() const {
    std::vector<std::vector<int>> islands;

    for (const auto& uvIsland : uvIslands_) {
        std::vector<int> islandFaces = uvIsland.faceIndices;
        islands.push_back(islandFaces);
    }

    return islands;
}

void UVEditor::optimizeUVLayout() {
    packUVs(0.01f);
}

void UVEditor::minimizeUVDistortion() {
    relaxUVs(10);
}

void UVEditor::setUVChannel(int channel) {
    if (channel >= 0 && channel < static_cast<int>(uvChannels_.size())) {
        currentUVChannel_ = channel;
    }
}

int UVEditor::getUVChannel() const {
    return currentUVChannel_;
}

void UVEditor::addUVChannel() {
    uvChannels_.push_back(std::vector<glm::vec2>());

    if (mesh_) {
        uvChannels_.back().resize(mesh_->getVertexCount());
        for (int i = 0; i < mesh_->getVertexCount(); ++i) {
            uvChannels_.back()[i] = mesh_->getVertex(i).texCoord;
        }
    }
}

void UVEditor::removeUVChannel(int channel) {
    if (channel >= 0 && channel < static_cast<int>(uvChannels_.size())) {
        uvChannels_.erase(uvChannels_.begin() + channel);

        if (currentUVChannel_ >= static_cast<int>(uvChannels_.size())) {
            currentUVChannel_ = static_cast<int>(uvChannels_.size()) - 1;
        }
    }
}

int UVEditor::getUVChannelCount() const {
    return static_cast<int>(uvChannels_.size());
}

void UVEditor::copyUVs(int sourceChannel, int targetChannel) {
    if (sourceChannel >= 0 && sourceChannel < static_cast<int>(uvChannels_.size()) &&
        targetChannel >= 0 && targetChannel < static_cast<int>(uvChannels_.size())) {
        uvChannels_[targetChannel] = uvChannels_[sourceChannel];
    }
}

void UVEditor::normalizeUVs() {
    if (uvChannels_.empty()) {
        return;
    }

    glm::vec2 minUV(std::numeric_limits<float>::max());
    glm::vec2 maxUV(std::numeric_limits<float>::lowest());

    for (const auto& uv : uvChannels_[currentUVChannel_]) {
        minUV = glm::min(minUV, uv);
        maxUV = glm::max(maxUV, uv);
    }

    glm::vec2 range = maxUV - minUV;
    if (range.x < 0.0001f) range.x = 1.0f;
    if (range.y < 0.0001f) range.y = 1.0f;

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        uv = (uv - minUV) / range;
    }
}

void UVEditor::centerUVs() {
    if (uvChannels_.empty()) {
        return;
    }

    glm::vec2 sum(0.0f);
    for (const auto& uv : uvChannels_[currentUVChannel_]) {
        sum += uv;
    }

    glm::vec2 center = sum / static_cast<float>(uvChannels_[currentUVChannel_].size());
    glm::vec2 offset = glm::vec2(0.5f) - center;

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        uv += offset;
    }
}

void UVEditor::bakeLightmapUVs(float texelSize) {
}

void UVEditor::validateUVs() {
    if (uvChannels_.empty()) {
        return;
    }

    for (auto& uv : uvChannels_[currentUVChannel_]) {
        uv.x = std::clamp(uv.x, 0.0f, 1.0f);
        uv.y = std::clamp(uv.y, 0.0f, 1.0f);
    }
}

void UVEditor::fixUVSeams() {
    if (!mesh_ || uvChannels_.empty()) {
        return;
    }

    std::unordered_map<size_t, std::vector<int>> uvGroups;

    for (int i = 0; i < mesh_->getVertexCount(); ++i) {
        const Vertex& vertex = mesh_->getVertex(i);
        size_t hash = vertex.hash();

        if (uvGroups.find(hash) == uvGroups.end()) {
            uvGroups[hash] = std::vector<int>();
        }
        uvGroups[hash].push_back(i);
    }

    for (const auto& pair : uvGroups) {
        if (pair.second.size() > 1) {
            glm::vec2 averageUV(0.0f);
            for (int index : pair.second) {
                averageUV += uvChannels_[currentUVChannel_][index];
            }
            averageUV /= static_cast<float>(pair.second.size());

            for (int index : pair.second) {
                uvChannels_[currentUVChannel_][index] = averageUV;
            }
        }
    }
}

void UVEditor::calculateUVIslands() {
    if (!mesh_ || !islandsDirty_) {
        return;
    }

    uvIslands_.clear();

    std::vector<bool> visited(mesh_->getFaceCount(), false);

    for (int i = 0; i < mesh_->getFaceCount(); ++i) {
        if (visited[i]) {
            continue;
        }

        UVIsland island;
        std::vector<int> stack = {i};

        while (!stack.empty()) {
            int faceIndex = stack.back();
            stack.pop_back();

            if (visited[faceIndex]) {
                continue;
            }

            visited[faceIndex] = true;
            island.faceIndices.push_back(faceIndex);

            const Face& face = mesh_->getFace(faceIndex);

            for (int vertexIndex : face.vertices) {
                std::vector<int> adjacentFaces = mesh_->getAdjacentFaces(vertexIndex);

                for (int adjacentFaceIndex : adjacentFaces) {
                    if (!visited[adjacentFaceIndex]) {
                        stack.push_back(adjacentFaceIndex);
                    }
                }
            }
        }

        calculateIslandBounds(island);
        uvIslands_.push_back(island);
    }

    islandsDirty_ = false;
}

void UVEditor::calculateIslandBounds(UVIsland& island) {
    if (!mesh_ || uvChannels_.empty()) {
        return;
    }

    island.minUV = glm::vec2(std::numeric_limits<float>::max());
    island.maxUV = glm::vec2(std::numeric_limits<float>::lowest());

    for (int faceIndex : island.faceIndices) {
        const Face& face = mesh_->getFace(faceIndex);
        for (int vertexIndex : face.vertices) {
            const glm::vec2& uv = uvChannels_[currentUVChannel_][vertexIndex];
            island.minUV = glm::min(island.minUV, uv);
            island.maxUV = glm::max(island.maxUV, uv);
        }
    }

    island.center = (island.minUV + island.maxUV) * 0.5f;
    island.size = island.maxUV - island.minUV;
    island.area = island.size.x * island.size.y;
}

}
}