#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <typeindex>
#include <vector>
#include <mutex>
#include <atomic>
#include <random>

namespace v3d {
namespace core {

class Object {
public:
    friend class ObjectManager;
    
    Object();
    virtual ~Object();

    std::string getId() const;
    std::string getName() const;
    void setName(const std::string& name);

    std::string getType() const;
    std::string getCategory() const;

    bool isActive() const;
    void setActive(bool active);

    virtual void update(float deltaTime);
    virtual void destroy();

    template<typename T>
    T* getComponent() const;

    template<typename T>
    T* addComponent();

    template<typename T>
    void removeComponent();

    std::vector<std::string> getComponentNames() const;

protected:
    std::string id_;
    std::string name_;
    std::string type_;
    std::string category_;
    bool active_;

    struct ComponentWrapper {
        virtual ~ComponentWrapper() = default;
        virtual void* get() = 0;
    };
    
    template<typename T>
    struct TypedComponentWrapper : ComponentWrapper {
        std::unique_ptr<T> component;
        TypedComponentWrapper(std::unique_ptr<T> comp) : component(std::move(comp)) {}
        void* get() override { return component.get(); }
    };
    
    std::unordered_map<std::type_index, std::unique_ptr<ComponentWrapper>> components_;
    mutable std::mutex componentMutex_;

    void generateId();
};

class ObjectManager {
public:
    static ObjectManager& getInstance();

    template<typename T, typename... Args>
    T* createObject(Args&&... args);

    void destroyObject(const std::string& id);
    void destroyAllObjects();

    template<typename T>
    T* getObject(const std::string& id) const;

    template<typename T>
    std::vector<T*> getObjectsByType() const;

    std::vector<Object*> getObjectsByCategory(const std::string& category) const;
    std::vector<Object*> getActiveObjects() const;

    Object* findObject(const std::string& name) const;
    Object* findObjectById(const std::string& id) const;

    void updateAllObjects(float deltaTime);

    size_t getObjectCount() const;
    size_t getActiveObjectCount() const;

    void setMaxObjects(size_t maxObjects);
    size_t getMaxObjects() const;

private:
    ObjectManager();
    ~ObjectManager();

    std::string generateId() const;

    std::unordered_map<std::string, std::unique_ptr<Object>> objects_;
    std::unordered_map<std::string, std::string> nameToIdMap_;
    mutable std::mutex objectsMutex_;
    std::atomic<size_t> maxObjects_;
};

template<typename T>
class ObjectFactory {
public:
    using Creator = std::function<T*()>;

    static ObjectFactory<T>& getInstance();

    void registerCreator(const std::string& type, Creator creator);
    T* create(const std::string& type);

    std::vector<std::string> getRegisteredTypes() const;

private:
    ObjectFactory() = default;

    std::unordered_map<std::string, Creator> creators_;
    mutable std::mutex mutex_;
};

template<typename T>
T* Object::getComponent() const {
    std::lock_guard<std::mutex> lock(componentMutex_);
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
        return static_cast<T*>(it->second->get());
    }
    return nullptr;
}

template<typename T>
T* Object::addComponent() {
    std::lock_guard<std::mutex> lock(componentMutex_);
    auto component = std::make_unique<T>();
    T* rawPtr = component.get();
    auto wrapper = std::make_unique<TypedComponentWrapper<T>>(std::move(component));
    components_[std::type_index(typeid(T))] = std::move(wrapper);
    return rawPtr;
}

template<typename T>
void Object::removeComponent() {
    std::lock_guard<std::mutex> lock(componentMutex_);
    components_.erase(std::type_index(typeid(T)));
}

template<typename T, typename... Args>
T* ObjectManager::createObject(Args&&... args) {
    auto obj = std::make_unique<T>(std::forward<Args>(args)...);
    T* rawPtr = obj.get();

    std::lock_guard<std::mutex> lock(objectsMutex_);

    if (objects_.size() >= maxObjects_) {
        return nullptr;
    }

    std::string id = generateId();
    obj->id_ = id;

    if (obj->name_.empty()) {
        obj->name_ = obj->type_ + "_" + id.substr(0, 8);
    }

    nameToIdMap_[obj->name_] = id;
    objects_[id] = std::move(obj);

    return rawPtr;
}

template<typename T>
T* ObjectManager::getObject(const std::string& id) const {
    std::lock_guard<std::mutex> lock(objectsMutex_);
    auto it = objects_.find(id);
    if (it != objects_.end()) {
        return dynamic_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
std::vector<T*> ObjectManager::getObjectsByType() const {
    std::lock_guard<std::mutex> lock(objectsMutex_);
    std::vector<T*> result;

    for (const auto& pair : objects_) {
        if (auto obj = dynamic_cast<T*>(pair.second.get())) {
            result.push_back(obj);
        }
    }

    return result;
}

template<typename T>
ObjectFactory<T>& ObjectFactory<T>::getInstance() {
    static ObjectFactory<T> instance;
    return instance;
}

template<typename T>
void ObjectFactory<T>::registerCreator(const std::string& type, Creator creator) {
    std::lock_guard<std::mutex> lock(mutex_);
    creators_[type] = creator;
}

template<typename T>
T* ObjectFactory<T>::create(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = creators_.find(type);
    if (it != creators_.end()) {
        return it->second();
    }
    return nullptr;
}

template<typename T>
std::vector<std::string> ObjectFactory<T>::getRegisteredTypes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> types;
    for (const auto& pair : creators_) {
        types.push_back(pair.first);
    }
    return types;
}

}
}