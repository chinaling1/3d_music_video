/**
 * @file mesh_editor.h
 * @brief 网格编辑器 - 提供顶点/边/面操作功能
 * 
 * 类似Blender的网格编辑功能：
 * - 选择模式（顶点/边/面）
 * - 变换操作（移动/旋转/缩放）
 * - 网格操作（挤出/倒角/切割/合并等）
 */

#pragma once

#include "geometry_factory.h"
#include <set>
#include <memory>
#include <functional>
#include <stack>

namespace v3d {
namespace modeling {

enum class SelectionMode {
    Vertex,
    Edge,
    Face,
    Object
};

enum class TransformMode {
    Translate,
    Rotate,
    Scale
};

enum class TransformSpace {
    World,
    Local,
    View
};

struct Selection {
    SelectionMode mode;
    std::set<int> indices;
    bool isAllSelected;
    
    Selection() : mode(SelectionMode::Vertex), isAllSelected(false) {}
    
    void clear() { indices.clear(); isAllSelected = false; }
    void selectAll(int count) { indices.clear(); for (int i = 0; i < count; i++) indices.insert(i); isAllSelected = true; }
    void toggle(int index) { if (indices.count(index)) indices.erase(index); else indices.insert(index); }
    void add(int index) { indices.insert(index); }
    void remove(int index) { indices.erase(index); }
    bool contains(int index) const { return indices.count(index) > 0; }
    bool isEmpty() const { return indices.empty() && !isAllSelected; }
    size_t count() const { return indices.size(); }
};

struct TransformGizmo {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 center;
    bool visible;
    TransformMode mode;
    TransformSpace space;
    int activeAxis; // 0=none, 1=x, 2=y, 3=z, 4=xy, 5=xz, 6=yz, 7=xyz
    
    TransformGizmo() : position(0), rotation(), scale(1), center(0), visible(false),
                       mode(TransformMode::Translate), space(TransformSpace::World), activeAxis(0) {}
};

class EditOperation {
public:
    virtual ~EditOperation() = default;
    virtual void execute(MeshData& mesh) = 0;
    virtual void undo(MeshData& mesh) = 0;
    virtual std::string getName() const = 0;
};

class MeshEditor {
public:
    MeshEditor();
    ~MeshEditor();
    
    void setMesh(MeshData* mesh);
    MeshData* getMesh() { return mesh_; }
    
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const { return selection_.mode; }
    
    void select(int index, bool addToSelection = false);
    void selectBox(const glm::vec3& min, const glm::vec3& max, bool addToSelection = false);
    void selectCircle(const glm::vec3& center, float radius, bool addToSelection = false);
    void selectAll();
    void deselectAll();
    void invertSelection();
    void selectLinked();
    void selectSimilar(float threshold = 0.01f);
    void selectMore();
    void selectLess();
    
    const Selection& getSelection() const { return selection_; }
    int getSelectionCount() const { return static_cast<int>(selection_.count()); }
    
    void setTransformMode(TransformMode mode);
    TransformMode getTransformMode() const { return gizmo_.mode; }
    
    void setTransformSpace(TransformSpace space);
    TransformSpace getTransformSpace() const { return gizmo_.space; }
    
    void beginTransform();
    void updateTransform(const glm::vec3& delta);
    void applyTransform();
    void cancelTransform();
    
    void translate(const glm::vec3& offset);
    void rotate(const glm::vec3& eulerAngles);
    void rotate(const glm::quat& rotation);
    void scale(const glm::vec3& scale);
    
    void extrude(float distance = 0.0f);
    void extrudeRegion(float distance = 0.0f);
    void extrudeIndividual(float distance = 0.0f);
    
    void inset(float thickness, float depth = 0.0f);
    void bevel(float amount, int segments = 1);
    void chamfer(float amount);
    
    void subdivide(int levels = 1);
    void loopCut(int cuts = 1, float smoothness = 0.0f);
    void knifeCut(const std::vector<glm::vec3>& points);
    
    void merge(MergeMode mode);
    void mergeByDistance(float distance = 0.0001f);
    
    void deleteSelected(DeleteMode mode = DeleteMode::Vertices);
    void dissolveSelected();
    void collapseSelected();
    
    void duplicate();
    void separate();
    
    void flipNormals();
    void recalculateNormals();
    void smoothNormals();
    
    void bridgeEdgeLoops(int loop1Start, int loop1End, int loop2Start, int loop2End, int segments = 1);
    
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    
    void setGizmoPosition(const glm::vec3& pos);
    void setGizmoVisible(bool visible);
    const TransformGizmo& getGizmo() const { return gizmo_; }
    
    glm::vec3 getSelectionCenter() const;
    glm::vec3 getSelectionNormal() const;
    
    void setOnSelectionChanged(std::function<void()> callback);
    void setOnMeshChanged(std::function<void()> callback);

private:
    void pushOperation(std::unique_ptr<EditOperation> op);
    void executeOperation(EditOperation* op);
    
    void updateGizmoPosition();
    
    std::vector<int> getSelectedVertices() const;
    std::vector<int> getSelectedEdges() const;
    std::vector<int> getSelectedFaces() const;
    
    MeshData* mesh_;
    Selection selection_;
    TransformGizmo gizmo_;
    
    std::stack<std::unique_ptr<EditOperation>> undoStack_;
    std::stack<std::unique_ptr<EditOperation>> redoStack_;
    
    MeshData preTransformState_;
    bool inTransform_;
    
    std::function<void()> onSelectionChanged_;
    std::function<void()> onMeshChanged_;
};

enum class MergeMode {
    AtCenter,
    AtCursor,
    AtFirst,
    AtLast,
    Collapse
};

enum class DeleteMode {
    Vertices,
    Edges,
    Faces,
    OnlyFaces
};

class TranslateOperation : public EditOperation {
public:
    TranslateOperation(const std::vector<int>& indices, const glm::vec3& offset);
    void execute(MeshData& mesh) override;
    void undo(MeshData& mesh) override;
    std::string getName() const override { return "Translate"; }
private:
    std::vector<int> indices_;
    glm::vec3 offset_;
};

class RotateOperation : public EditOperation {
public:
    RotateOperation(const std::vector<int>& indices, const glm::quat& rotation, const glm::vec3& pivot);
    void execute(MeshData& mesh) override;
    void undo(MeshData& mesh) override;
    std::string getName() const override { return "Rotate"; }
private:
    std::vector<int> indices_;
    glm::quat rotation_;
    glm::vec3 pivot_;
    glm::quat inverseRotation_;
};

class ScaleOperation : public EditOperation {
public:
    ScaleOperation(const std::vector<int>& indices, const glm::vec3& scale, const glm::vec3& pivot);
    void execute(MeshData& mesh) override;
    void undo(MeshData& mesh) override;
    std::string getName() const override { return "Scale"; }
private:
    std::vector<int> indices_;
    glm::vec3 scale_;
    glm::vec3 pivot_;
    glm::vec3 inverseScale_;
};

class ExtrudeOperation : public EditOperation {
public:
    ExtrudeOperation(const std::vector<int>& faceIndices, float distance);
    void execute(MeshData& mesh) override;
    void undo(MeshData& mesh) override;
    std::string getName() const override { return "Extrude"; }
private:
    std::vector<int> faceIndices_;
    float distance_;
    std::vector<int> newFaceIndices_;
    std::vector<Vertex> originalVertices_;
    std::vector<Face> originalFaces_;
};

class BevelOperation : public EditOperation {
public:
    BevelOperation(const std::vector<int>& edgeIndices, float amount, int segments);
    void execute(MeshData& mesh) override;
    void undo(MeshData& mesh) override;
    std::string getName() const override { return "Bevel"; }
private:
    std::vector<int> edgeIndices_;
    float amount_;
    int segments_;
    MeshData originalMesh_;
};

}
}
