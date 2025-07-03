#include "blacksite/core/EntityRegistry.h"

namespace Blacksite {

std::unordered_map<std::string, std::function<std::unique_ptr<Entity>()>> EntityRegistry::s_factories;
std::unordered_map<std::string, std::string> EntityRegistry::s_typeNames;

}  // namespace Blacksite
