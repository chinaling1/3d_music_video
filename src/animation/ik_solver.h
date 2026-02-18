#pragma once

#include "skeleton.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace v3d {
namespace animation {

enum class IKType {
    TwoBone,
    FABRIK,
    CCD,
    Jacobian
};

class IKSolver {
public:
    IKSolver();
    virtual ~IKSolver();

    virtual bool solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) = 0;

    void setMaxIterations(int iterations);
    int getMaxIterations() const;

    void setTolerance(float tolerance);
    float getTolerance() const;

    void setChainLength(int length);
    int getChainLength() const;

protected:
    int maxIterations_;
    float tolerance_;
    int chainLength_;

    std::vector<Bone*> buildIKChain(Bone* tipBone, int length);
};

class TwoBoneIKSolver : public IKSolver {
public:
    TwoBoneIKSolver();

    bool solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) override;

    void setPoleVector(const glm::vec3& pole);
    glm::vec3 getPoleVector() const;

    void setPoleEnabled(bool enabled);
    bool isPoleEnabled() const;

private:
    glm::vec3 poleVector_;
    bool poleEnabled_;
};

class FABRIKSolver : public IKSolver {
public:
    FABRIKSolver();

    bool solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) override;

    void setTargetRotation(const glm::quat& rotation);
    glm::quat getTargetRotation() const;

    void setTargetRotationEnabled(bool enabled);
    bool isTargetRotationEnabled() const;

private:
    glm::quat targetRotation_;
    bool targetRotationEnabled_;
};

class CCDSolver : public IKSolver {
public:
    CCDSolver();

    bool solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) override;

private:
};

class JacobianSolver : public IKSolver {
public:
    JacobianSolver();

    bool solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) override;

    void setDamping(float damping);
    float getDamping() const;

    void setSecondaryTask(std::function<void(Skeleton*)> task);

private:
    float damping_;
    std::function<void(Skeleton*)> secondaryTask_;
};

class IKController {
public:
    IKController();
    ~IKController();

    void addIKSolver(const std::string& name, std::unique_ptr<IKSolver> solver);
    void removeIKSolver(const std::string& name);
    IKSolver* getIKSolver(const std::string& name) const;

    void solveAll(Skeleton* skeleton);

    void setIKTarget(const std::string& ikName, const glm::vec3& position);
    glm::vec3 getIKTarget(const std::string& ikName) const;

    void setIKEnabled(const std::string& ikName, bool enabled);
    bool isIKEnabled(const std::string& ikName) const;

    void clear();

private:
    struct IKHandle {
        std::unique_ptr<IKSolver> solver;
        glm::vec3 targetPosition;
        bool enabled;
        std::string tipBoneName;
    };

    std::unordered_map<std::string, IKHandle> ikHandles_;
};

class LookAtIK {
public:
    LookAtIK();

    void setTargetBone(const std::string& boneName);
    std::string getTargetBone() const;

    void setLookAtTarget(const glm::vec3& target);
    glm::vec3 getLookAtTarget() const;

    void setUpVector(const glm::vec3& up);
    glm::vec3 getUpVector() const;

    void setWeight(float weight);
    float getWeight() const;

    void apply(Skeleton* skeleton);

private:
    std::string targetBone_;
    glm::vec3 lookAtTarget_;
    glm::vec3 upVector_;
    float weight_;
};

}
}