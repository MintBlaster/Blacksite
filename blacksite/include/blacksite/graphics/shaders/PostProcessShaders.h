#pragma once

namespace Blacksite {
namespace Shaders {

// Basic post-process quad shader
extern const char* POSTPROCESS_VERTEX_SHADER;
extern const char* POSTPROCESS_FRAGMENT_SHADER;

// Gaussian blur
extern const char* BLUR_VERTEX_SHADER;
extern const char* BLUR_FRAGMENT_SHADER;

// Simple bloom effect
extern const char* BLOOM_VERTEX_SHADER;
extern const char* BLOOM_FRAGMENT_SHADER;

// Fast Approximate Anti-Aliasing
extern const char* FXAA_VERTEX_SHADER;
extern const char* FXAA_FRAGMENT_SHADER;

} // namespace Shaders
} // namespace Blacksite
