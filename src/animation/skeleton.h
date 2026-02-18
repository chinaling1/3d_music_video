/**
 * @file skeleton.h
 * @brief 骨骼动画系统 - 提供骨骼层次结构和姿态管理
 * 
 * 该模块实现了完整的骨骼动画系统，包括：
 * - Transform: 变换结构（位置、旋转、缩放）
 * - Bone: 骨骼类
 * - Skeleton: 骨骼层次结构
 * - Pose: 姿态类
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace v3d {
namespace animation {

/**
 * @struct Transform
 * @brief 变换结构 - 表示位置、旋转和缩放的组合
 * 
 * 用于表示骨骼、对象等的变换。
 * 支持矩阵转换、插值和组合操作。
 * 
 * @example
 * @code
 * Transform t1(glm::vec3(1, 0, 0), glm::quat(), glm::vec3(1));
 * Transform t2(glm::vec3(0, 1, 0), glm::quat(), glm::vec3(2));
 * 
 * // 插值
 * Transform result = Transform::interpolate(t1, t2, 0.5f);
 * 
 * // 组合
 * Transform combined = t1 * t2;
 * 
 * // 转换为矩阵
 * glm::mat4 matrix = result.toMatrix();
 * @endcode
 */
struct Transform {
    glm::vec3 position{0.0f};   ///< 位置
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};  ///< 旋转（四元数）
    glm::vec3 scale{1.0f};      ///< 缩放

    Transform() = default;
    
    /**
     * @brief 构造变换
     * @param pos 位置
     * @param rot 旋转
     * @param scl 缩放
     */
    Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl)
        : position(pos), rotation(rot), scale(scl) {}

    /**
     * @brief 转换为4x4矩阵
     * @return 变换矩阵
     */
    glm::mat4 toMatrix() const;
    
    /**
     * @brief 从矩阵创建变换
     * @param matrix 变换矩阵
     * @return Transform对象
     */
    static Transform fromMatrix(const glm::mat4& matrix);
    
    /**
     * @brief 线性插值两个变换
     * @param a 起始变换
     * @param b 结束变换
     * @param t 插值参数 [0, 1]
     * @return 插值后的变换
     * 
     * 位置和缩放使用线性插值，旋转使用球面线性插值(SLERP)。
     */
    static Transform interpolate(const Transform& a, const Transform& b, float t);
    
    /**
     * @brief 计算逆变换
     * @return 逆变换
     */
    Transform inverse() const;
    
    /**
     * @brief 组合两个变换
     * @param other 另一个变换
     * @return 组合后的变换
     */
    Transform operator*(const Transform& other) const;
};

/**
 * @class Bone
 * @brief 骨骼类 - 表示骨骼层次结构中的一个骨骼
 * 
 * 每个骨骼包含：
 * - 局部变换：相对于父骨骼的变换
 * - 世界变换：相对于根骨骼的变换
 * - 偏移矩阵：用于蒙皮的偏移
 * 
 * @example
 * @code
 * Bone bone("spine");
 * bone.setLocalTransform(Transform(
 *     glm::vec3(0, 1, 0),
 *     glm::quat(),
 *     glm::vec3(1)
 * ));
 * 
 * // 添加子骨骼
 * Bone* child = skeleton->createBone("neck", &bone);
 * bone.addChild(child);
 * @endcode
 */
class Bone {
public:
    Bone();
    
    /**
     * @brief 构造命名的骨骼
     * @param name 骨骼名称
     */
    explicit Bone(const std::string& name);

    /**
     * @brief 获取骨骼名称
     * @return 名称
     */
    std::string getName() const;
    
    /**
     * @brief 设置骨骼名称
     * @param name 名称
     */
    void setName(const std::string& name);

    /**
     * @brief 获取骨骼索引
     * @return 在骨骼列表中的索引
     */
    int getIndex() const;
    
    /**
     * @brief 设置骨骼索引
     * @param index 索引
     */
    void setIndex(int index);

    /**
     * @brief 获取局部变换
     * @return 相对于父骨骼的变换
     */
    Transform getLocalTransform() const;
    
    /**
     * @brief 设置局部变换
     * @param transform 变换
     */
    void setLocalTransform(const Transform& transform);

    /**
     * @brief 获取世界变换
     * @return 相对于根骨骼的变换
     */
    Transform getWorldTransform() const;
    
    /**
     * @brief 设置世界变换
     * @param transform 变换
     */
    void setWorldTransform(const Transform& transform);

    /**
     * @brief 获取偏移矩阵
     * @return 用于蒙皮的偏移变换
     */
    Transform getOffsetMatrix() const;
    
    /**
     * @brief 设置偏移矩阵
     * @param transform 偏移变换
     */
    void setOffsetMatrix(const Transform& transform);

    /**
     * @brief 获取父骨骼
     * @return 父骨骼指针（根骨骼返回nullptr）
     */
    Bone* getParent() const;
    
    /**
     * @brief 设置父骨骼
     * @param parent 父骨骼指针
     */
    void setParent(Bone* parent);

    /**
     * @brief 获取子骨骼列表
     * @return 子骨骼指针列表
     */
    const std::vector<Bone*>& getChildren() const;
    
    /**
     * @brief 添加子骨骼
     * @param child 子骨骼指针
     */
    void addChild(Bone* child);
    
    /**
     * @brief 移除子骨骼
     * @param child 子骨骼指针
     */
    void removeChild(Bone* child);

    /**
     * @brief 更新世界变换
     * @param parentWorldTransform 父骨骼的世界变换
     * 
     * 递归更新所有子骨骼的世界变换。
     */
    void updateWorldTransform(const Transform& parentWorldTransform);

    /**
     * @brief 检查骨骼是否有动画
     * @return 如果骨骼被动画化返回true
     */
    bool isAnimated() const;
    
    /**
     * @brief 设置骨骼动画状态
     * @param animated 是否有动画
     */
    void setAnimated(bool animated);

private:
    std::string name_;              ///< 骨骼名称
    int index_;                     ///< 骨骼索引
    Transform localTransform_;      ///< 局部变换
    Transform worldTransform_;      ///< 世界变换
    Transform offsetMatrix_;        ///< 偏移矩阵
    Bone* parent_;                  ///< 父骨骼
    std::vector<Bone*> children_;   ///< 子骨骼列表
    bool animated_;                 ///< 动画标志
};

/**
 * @class Skeleton
 * @brief 骨骼层次结构 - 管理完整的骨骼系统
 * 
 * 提供骨骼的创建、查询和更新功能。
 * 支持绑定姿态计算和骨骼矩阵获取。
 * 
 * @example
 * @code
 * Skeleton skeleton;
 * 
 * // 创建骨骼层次
 * Bone* root = skeleton.createBone("root");
 * Bone* spine = skeleton.createBone("spine", root);
 * Bone* head = skeleton.createBone("head", spine);
 * 
 * skeleton.setRootBone(root);
 * 
 * // 计算绑定姿态
 * skeleton.calculateBindPose();
 * 
 * // 更新骨骼
 * skeleton.update();
 * 
 * // 获取骨骼矩阵用于渲染
 * auto matrices = skeleton.getBoneMatrices();
 * @endcode
 */
class Skeleton {
public:
    Skeleton();
    ~Skeleton();

    /**
     * @brief 创建骨骼
     * @param name 骨骼名称
     * @param parent 父骨骼（nullptr表示根骨骼）
     * @return 创建的骨骼指针
     */
    Bone* createBone(const std::string& name, Bone* parent = nullptr);
    
    /**
     * @brief 按名称获取骨骼
     * @param name 骨骼名称
     * @return 骨骼指针（未找到返回nullptr）
     */
    Bone* getBone(const std::string& name) const;
    
    /**
     * @brief 按索引获取骨骼
     * @param index 骨骼索引
     * @return 骨骼指针
     */
    Bone* getBone(int index) const;

    /**
     * @brief 获取所有骨骼
     * @return 骨骼指针列表
     */
    std::vector<Bone*> getBones() const;
    
    /**
     * @brief 获取骨骼数量
     * @return 骨骼数量
     */
    size_t getBoneCount() const;

    /**
     * @brief 设置根骨骼
     * @param bone 根骨骼指针
     */
    void setRootBone(Bone* bone);
    
    /**
     * @brief 获取根骨骼
     * @return 根骨骼指针
     */
    Bone* getRootBone() const;

    /**
     * @brief 更新骨骼层次
     * 
     * 从根骨骼开始递归更新所有骨骼的世界变换。
     */
    void update();
    
    /**
     * @brief 重置到绑定姿态
     */
    void resetToBindPose();

    /**
     * @brief 获取骨骼名称
     * @return 名称
     */
    std::string getName() const;
    
    /**
     * @brief 设置骨骼名称
     * @param name 名称
     */
    void setName(const std::string& name);

    /**
     * @brief 计算绑定姿态
     * 
     * 计算并存储所有骨骼的逆绑定姿态矩阵。
     */
    void calculateBindPose();

    /**
     * @brief 获取骨骼矩阵
     * @return 用于GPU蒙皮的骨骼矩阵列表
     */
    std::vector<glm::mat4> getBoneMatrices() const;
    
    /**
     * @brief 获取逆绑定姿态矩阵
     * @return 逆绑定姿态矩阵列表
     */
    std::vector<glm::mat4> getInverseBindPoseMatrices() const;

private:
    std::string name_;                          ///< 骨骼名称
    std::vector<std::unique_ptr<Bone>> bones_;  ///< 骨骼存储
    std::unordered_map<std::string, Bone*> boneMap_; ///< 名称到骨骼的映射
    Bone* rootBone_;                            ///< 根骨骼
};

/**
 * @class Pose
 * @brief 姿态类 - 存储骨骼的特定姿态
 * 
 * 用于存储和应用骨骼动画的关键帧姿态。
 * 支持姿态混合、叠加和乘法操作。
 * 
 * @example
 * @code
 * Pose pose(&skeleton);
 * 
 * // 设置骨骼变换
 * pose.setLocalTransform(spineIndex, Transform(
 *     glm::vec3(0, 1, 0),
 *     glm::quat(glm::radians(glm::vec3(0, 45, 0))),
 *     glm::vec3(1)
 * ));
 * 
 * // 混合两个姿态
 * Pose blended;
 * blended.blend(pose1, 0.5f);
 * blended.blend(pose2, 0.5f);
 * @endcode
 */
class Pose {
public:
    Pose();
    
    /**
     * @brief 从骨骼构造姿态
     * @param skeleton 骨骼指针
     */
    explicit Pose(const Skeleton* skeleton);

    /**
     * @brief 设置关联的骨骼
     * @param skeleton 骨骼指针
     */
    void setSkeleton(const Skeleton* skeleton);
    
    /**
     * @brief 获取关联的骨骼
     * @return 骨骼指针
     */
    const Skeleton* getSkeleton() const;

    /**
     * @brief 获取骨骼数量
     * @return 骨骼数量
     */
    size_t getBoneCount() const;

    /**
     * @brief 获取骨骼的局部变换
     * @param boneIndex 骨骼索引
     * @return 局部变换
     */
    Transform getLocalTransform(int boneIndex) const;
    
    /**
     * @brief 设置骨骼的局部变换
     * @param boneIndex 骨骼索引
     * @param transform 变换
     */
    void setLocalTransform(int boneIndex, const Transform& transform);

    /**
     * @brief 获取骨骼的世界变换
     * @param boneIndex 骨骼索引
     * @return 世界变换
     */
    Transform getWorldTransform(int boneIndex) const;
    
    /**
     * @brief 设置骨骼的世界变换
     * @param boneIndex 骨骼索引
     * @param transform 变换
     */
    void setWorldTransform(int boneIndex, const Transform& transform);

    /**
     * @brief 与另一个姿态混合
     * @param other 另一个姿态
     * @param weight 混合权重
     * 
     * 当前姿态 = 当前姿态 * (1 - weight) + other * weight
     */
    void blend(const Pose& other, float weight);
    
    /**
     * @brief 叠加另一个姿态
     * @param other 另一个姿态
     * @param weight 叠加权重
     */
    void add(const Pose& other, float weight);
    
    /**
     * @brief 乘以另一个姿态
     * @param other 另一个姿态
     */
    void multiply(const Pose& other);

    /**
     * @brief 从局部变换计算世界变换
     */
    void calculateWorldTransforms();
    
    /**
     * @brief 从世界变换计算局部变换
     */
    void calculateLocalTransforms();

    /**
     * @brief 检查姿态是否有效
     * @return 如果姿态有效返回true
     */
    bool isValid() const;

private:
    const Skeleton* skeleton_;              ///< 关联的骨骼
    std::vector<Transform> localTransforms_;  ///< 局部变换列表
    std::vector<Transform> worldTransforms_;  ///< 世界变换列表
};

}
}
