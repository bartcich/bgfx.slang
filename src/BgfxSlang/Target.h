#pragma once

#include "BgfxSlang/EntryPoint.h"
#include <array>
#include <slang.h>
#include <string_view>
#include <vector>

namespace BgfxSlang {

enum class TargetFormat { Unknown, DirectX, SpirV };

struct TargetProfile {
  TargetFormat Format;
  std::string_view Name;
  std::string_view Id;

  [[nodiscard]] SlangCompileTarget GetSlangTarget() const {
    switch (Format) {
    case TargetFormat::DirectX:
      return SLANG_DXBC;
    case TargetFormat::SpirV:
      return SLANG_SPIRV;
    default:
      return SLANG_TARGET_UNKNOWN;
    }
  }
};

struct TargetSettings {
  TargetProfile Profile;
  std::vector<slang::CompilerOptionEntry> CompilerOptions;

  [[nodiscard]] std::vector<slang::CompilerOptionEntry> GetCompilerOptions(StageType stage) const;
};

constexpr std::array targetProfiles = {
    TargetProfile{TargetFormat::DirectX, "dx", "sm_5_0"},         TargetProfile{TargetFormat::DirectX, "sm_5_0", "sm_5_0"},
    TargetProfile{TargetFormat::SpirV, "spirv", "spirv_1_3"},     TargetProfile{TargetFormat::SpirV, "spirv_1_3", "spirv_1_3"},
    TargetProfile{TargetFormat::SpirV, "spirv_1_4", "spirv_1_4"}, TargetProfile{TargetFormat::SpirV, "spirv_1_5", "spirv_1_5"},
    TargetProfile{TargetFormat::SpirV, "spirv_1_6", "spirv_1_6"},
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
  default:
    return "unknown";
  }
}
} // namespace BgfxSlang