#include "object_manager.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace v3d {
namespace core {

Object::Object()
    : active_(true) {
    generateId();
    type_ = typeid(*this).name();
}

Object::~Object() {
}

std::string Object::getId() const {
    return id_;
}

std::string Object::getName() const {
    return name_;
}

void Object::setName(const std::string& name) {
    name_ = name;
}

std::string Object::getType() const {
    return type_;
}

std::string Object::getCategory() const {
    return category_;
}

bool Object::isActive() const {
    return active_;
}

void Object::setActive(bool active) {
    active_ = active;
}

void Object::update(float deltaTime) {
}

void Object::destroy() {
    active_ = false;
}

void Object::generateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 8; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 4; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-4";

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    ss << std::setw(1) << (8 + dis(gen) % 4);

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 12; ++i) {
        ss << std::setw(1) << dis(gen);
    }

    id_ = ss.str();
}

std::vector<std::string> Object::getComponentNames() const {
    std::lock_guard<std::mutex> lock(componentMutex_);
    std::vector<std::string> names;

    for (const auto& pair : components_) {
        names.push_back(pair.first.name());
    }

    return names;
}

ObjectManager& ObjectManager::getInstance() {
    static ObjectManager instance;
    return instance;
}

ObjectManager::ObjectManager()
    : maxObjects_(10000) {
}

ObjectManager::~ObjectManager() {
    destroyAllObjects();
}

void ObjectManager::destroyObject(const std::string& id) {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    auto it = objects_.find(id);
    if (it != objects_.end()) {
        nameToIdMap_.erase(it->second->getName());
        it->second->destroy();
        objects_.erase(it);
    }
}

void ObjectManager::destroyAllObjects() {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    for (auto& pair : objects_) {
        pair.second->destroy();
    }

    objects_.clear();
    nameToIdMap_.clear();
}

std::vector<Object*> ObjectManager::getObjectsByCategory(const std::string& category) const {
    std::lock_guard<std::mutex> lock(objectsMutex_);
    std::vector<Object*> result;

    for (const auto& pair : objects_) {
        if (pair.second->getCategory() == category) {
            result.push_back(pair.second.get());
        }
    }

    return result;
}

std::vector<Object*> ObjectManager::getActiveObjects() const {
    std::lock_guard<std::mutex> lock(objectsMutex_);
    std::vector<Object*> result;

    for (const auto& pair : objects_) {
        if (pair.second->isActive()) {
            result.push_back(pair.second.get());
        }
    }

    return result;
}

Object* ObjectManager::findObject(const std::string& name) const {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    auto it = nameToIdMap_.find(name);
    if (it != nameToIdMap_.end()) {
        return objects_.at(it->second).get();
    }

    return nullptr;
}

Object* ObjectManager::findObjectById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    auto it = objects_.find(id);
    if (it != objects_.end()) {
        return it->second.get();
    }

    return nullptr;
}

void ObjectManager::updateAllObjects(float deltaTime) {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    for (auto& pair : objects_) {
        if (pair.second->isActive()) {
            pair.second->update(deltaTime);
        }
    }
}

size_t ObjectManager::getObjectCount() const {
    return objects_.size();
}

size_t ObjectManager::getActiveObjectCount() const {
    std::lock_guard<std::mutex> lock(objectsMutex_);

    size_t count = 0;
    for (const auto& pair : objects_) {
        if (pair.second->isActive()) {
            count++;
        }
    }

    return count;
}

void ObjectManager::setMaxObjects(size_t maxObjects) {
    maxObjects_ = maxObjects;
}

size_t ObjectManager::getMaxObjects() const {
    return maxObjects_;
}

std::string ObjectManager::generateId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 8; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 4; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-4";

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    ss << std::setw(1) << (8 + dis(gen) % 4);

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 12; ++i) {
        ss << std::setw(1) << dis(gen);
    }

    return ss.str();
}

}
}
