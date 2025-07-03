#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include "blacksite/core/Entity.h"

namespace Blacksite {

class EntityRegistry {
public:
    template<typename T>
    static void Register(const std::string& typeName) {
        static_assert(std::is_base_of_v<Entity, T>, "T must derive from Entity");
        s_factories[typeName] = []() { return std::make_unique<T>(); };
        s_typeNames[typeid(T).name()] = typeName;
    }

    static std::unique_ptr<Entity> Create(const std::string& typeName) {
        auto it = s_factories.find(typeName);
        if (it != s_factories.end()) {
            return it->second();
        }
        return nullptr;
    }

    static std::string GetTypeName(const std::type_info& typeInfo) {
        auto it = s_typeNames.find(typeInfo.name());
        return it != s_typeNames.end() ? it->second : "Unknown";
    }

    static std::vector<std::string> GetRegisteredTypes() {
        std::vector<std::string> types;
        for (const auto& pair : s_factories) {
            types.push_back(pair.first);
        }
        return types;
    }

private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<Entity>()>> s_factories;
    static std::unordered_map<std::string, std::string> s_typeNames;
};

}  // namespace Blacksite
