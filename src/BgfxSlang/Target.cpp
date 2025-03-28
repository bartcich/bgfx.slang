#include "Target.h"
#include "EntryPoint.h"
#include <slang.h>
#include <vector>

namespace BgfxSlang {

namespace {

// HLSLToVulkanLayoutOptions::Kind in slang
constexpr int slangShiftKindUnorderedAccess = 0;
constexpr int slangShiftKindSampler = 1;
constexpr int slangShiftKindShaderResource = 2;
constexpr int slangShiftKindConstantBuffer = 3;

constexpr int vulkanVertexCBufferShift = 0;
constexpr int vulkanFragmentCBufferShift = 1;
constexpr int vulkanTextureShift = 2;
constexpr int vulkanSamplerShift = 18;
} // namespace

std::vector<slang::CompilerOptionEntry> TargetSettings::GetCompilerOptions(StageType stage) const {

  std::vector<slang::CompilerOptionEntry> options(CompilerOptions.begin(), CompilerOptions.end());
  options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::Optimization,
                                               {.intValue0 = SlangOptimizationLevel::SLANG_OPTIMIZATION_LEVEL_MAXIMAL}});

  switch (Profile.Format) {
  case TargetFormat::DirectX:
    return options;
  case TargetFormat::OpenGL:
  case TargetFormat::OpenGLES:
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindConstantBuffer, .intValue1 = 0}});
    options.push_back(
        slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll, {.intValue0 = slangShiftKindSampler, .intValue1 = 0}});
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindShaderResource, .intValue1 = 0}});
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindUnorderedAccess, .intValue1 = 0}});
    return options;
  case TargetFormat::SpirV:
    options.push_back(
        slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                   {.intValue0 = slangShiftKindConstantBuffer,
                                    .intValue1 = stage == StageType::Fragment ? vulkanFragmentCBufferShift : vulkanVertexCBufferShift}});
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindSampler, .intValue1 = vulkanSamplerShift}});
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindShaderResource, .intValue1 = vulkanTextureShift}});
    options.push_back(slang::CompilerOptionEntry{slang::CompilerOptionName::VulkanBindShiftAll,
                                                 {.intValue0 = slangShiftKindUnorderedAccess, .intValue1 = vulkanTextureShift}});

    return options;
  default:
    return options;
  }
  return {};
}
} // namespace BgfxSlang