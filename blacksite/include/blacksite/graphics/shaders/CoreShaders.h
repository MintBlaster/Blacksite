#pragma once

namespace Blacksite {
namespace Shaders {

// Basic Phong lighting shader - the workhorse of the engine
extern const char* BASIC_VERTEX_SHADER;
extern const char* BASIC_FRAGMENT_SHADER;

// Unlit shader - for UI, particles, or objects that don't need lighting
extern const char* UNLIT_VERTEX_SHADER;
extern const char* UNLIT_FRAGMENT_SHADER;

// Transparent shader - for alpha blending
extern const char* TRANSPARENT_VERTEX_SHADER;
extern const char* TRANSPARENT_FRAGMENT_SHADER;

} // namespace Shaders
} // namespace Blacksite
