#pragma once

#include "Types.h"
#include <array>
#include <slang.h>
#include <string_view>
#include <vector>

namespace BgfxSlang {

struct TargetProfile {
  TargetFormat Format;
  std::string_view Name;
  std::string_view Id;

  [[nodiscard]] SlangCompileTarget GetSlangTarget() const {
    switch (Format) {
    case TargetFormat::DirectX:
      return SLANG_DXBC;
    case TargetFormat::SpirV:
    case TargetFormat::OpenGL:
    case TargetFormat::OpenGLES:
    case TargetFormat::Metal:
      return SLANG_SPIRV;
    default:
      return SLANG_TARGET_UNKNOWN;
    }
  }

  [[nodiscard]] std::string_view GetProfile() const {
    if (Format == TargetFormat::OpenGL || Format == TargetFormat::OpenGLES || Format == TargetFormat::Metal) {
      return "spirv_1_3";
    }
    return Id;
  }

  void AddTargetMacros(std::vector<slang::PreprocessorMacroDesc> &macros) const {
    switch (Format) {
    case TargetFormat::DirectX:
      macros.emplace_back("BGFX_SHADER_LANGUAGE_HLSL", "1");
      break;
    case TargetFormat::SpirV:
      macros.emplace_back("BGFX_SHADER_LANGUAGE_SPIRV", "1");
      break;
    case TargetFormat::OpenGL:
      macros.emplace_back("BGFX_SHADER_LANGUAGE_GLSL", "1");
      break;
    case TargetFormat::OpenGLES:
      macros.emplace_back("BGFX_SHADER_LANGUAGE_GLSL", "1");
      macros.emplace_back("BGFX_SHADER_LANGUAGE_GLES", "1");
      break;
    case TargetFormat::Metal:
      macros.emplace_back("BGFX_SHADER_LANGUAGE_METAL", "1");
      break;
    default:
      break;
    }
  }
};

struct TargetSettings {
  TargetProfile Profile;
  std::vector<slang::CompilerOptionEntry> CompilerOptions;

  [[nodiscard]] std::vector<slang::CompilerOptionEntry> GetCompilerOptions(StageType stage) const;
};

constexpr std::array targetProfiles = {
    TargetProfile{TargetFormat::DirectX, "dx", "sm_5_0"},           TargetProfile{TargetFormat::DirectX, "sm_5_0", "sm_5_0"},
    TargetProfile{TargetFormat::SpirV, "spirv", "spirv_1_3"},       TargetProfile{TargetFormat::SpirV, "spirv_1_3", "spirv_1_3"},
    TargetProfile{TargetFormat::SpirV, "spirv_1_4", "spirv_1_4"},   TargetProfile{TargetFormat::SpirV, "spirv_1_5", "spirv_1_5"},
    TargetProfile{TargetFormat::SpirV, "spirv_1_6", "spirv_1_6"},   TargetProfile{TargetFormat::OpenGL, "glsl", "glsl_150"},
    TargetProfile{TargetFormat::OpenGL, "glsl_150", "glsl_150"},    TargetProfile{TargetFormat::OpenGL, "glsl_330", "glsl_330"},
    TargetProfile{TargetFormat::OpenGL, "glsl_400", "glsl_400"},    TargetProfile{TargetFormat::OpenGL, "glsl_410", "glsl_410"},
    TargetProfile{TargetFormat::OpenGL, "glsl_420", "glsl_420"},    TargetProfile{TargetFormat::OpenGL, "glsl_430", "glsl_430"},
    TargetProfile{TargetFormat::OpenGL, "glsl_440", "glsl_440"},    TargetProfile{TargetFormat::OpenGLES, "gles_100", "gles_100"},
    TargetProfile{TargetFormat::OpenGLES, "gles", "gles_300"},      TargetProfile{TargetFormat::OpenGLES, "gles_300", "gles_300"},
    TargetProfile{TargetFormat::OpenGLES, "gles_310", "gles_310"},  TargetProfile{TargetFormat::OpenGLES, "gles_320", "gles_320"},
    TargetProfile{TargetFormat::Metal, "metal", "macos_1_2"},       TargetProfile{TargetFormat::Metal, "metal_1_0", "macos_1_0"},
    TargetProfile{TargetFormat::Metal, "metal_1_1", "macos_1_1"},   TargetProfile{TargetFormat::Metal, "metal_1_2", "macos_1_2"},
    TargetProfile{TargetFormat::Metal, "metal_2_0", "macos_2_0"},   TargetProfile{TargetFormat::Metal, "metal_2_1", "macos_2_1"},
    TargetProfile{TargetFormat::Metal, "metal_2_2", "macos_2_2"},   TargetProfile{TargetFormat::Metal, "metal_2_3", "macos_2_3"},
    TargetProfile{TargetFormat::Metal, "metal_2_4", "macos_2_4"},   TargetProfile{TargetFormat::Metal, "metal_3_0", "macos_3_0"},
    TargetProfile{TargetFormat::Metal, "metal_3_1", "macos_3_1"},   TargetProfile{TargetFormat::Metal, "metal_ios", "ios_1_2"},
    TargetProfile{TargetFormat::Metal, "metal_ios_1_0", "ios_1_0"}, TargetProfile{TargetFormat::Metal, "metal_ios_1_1", "ios_1_1"},
    TargetProfile{TargetFormat::Metal, "metal_ios_1_2", "ios_1_2"}, TargetProfile{TargetFormat::Metal, "metal_ios_2_0", "ios_2_0"},
    TargetProfile{TargetFormat::Metal, "metal_ios_2_1", "ios_2_1"}, TargetProfile{TargetFormat::Metal, "metal_ios_2_2", "ios_2_2"},
    TargetProfile{TargetFormat::Metal, "metal_ios_2_3", "ios_2_3"}, TargetProfile{TargetFormat::Metal, "metal_ios_2_4", "ios_2_4"},
    TargetProfile{TargetFormat::Metal, "metal_ios_3_0", "ios_3_0"}, TargetProfile{TargetFormat::Metal, "metal_ios_3_1", "ios_3_1"},
};

inline TargetProfile findProfile(std::string_view name) {
  for (const auto &profile : targetProfiles) {
    if (profile.Name == name) {
      return profile;
    }
  }
  return {TargetFormat::Unknown, ""};
}

inline std::string_view GetTargetShortName(const TargetProfile &profile) {
  switch (profile.Format) {
  case TargetFormat::DirectX:
    return "dx11";
  case TargetFormat::SpirV:
    return "spirv";
  case TargetFormat::OpenGL:
    return "glsl";
  case TargetFormat::OpenGLES:
    return "essl";
  case TargetFormat::Metal:
    return "metal";
  default:
    return "unknown";
  }
}

inline std::string_view GetTargetShortNameForHeaderVar(const TargetProfile &profile) {
  switch (profile.Format) {
  case TargetFormat::DirectX:
    return "dx11";
  case TargetFormat::SpirV:
    return "spv";
  case TargetFormat::OpenGL:
    return "glsl";
  case TargetFormat::OpenGLES:
    return "essl";
  case TargetFormat::Metal:
    return "mtl";
  default:
    return "unknown";
  }
}
} // namespace BgfxSlang