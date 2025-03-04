#pragma once

#include <cstdint>
#include <slang.h>
#include <string>
#include <string_view>

namespace BgfxSlang {

enum class StageType {
  Unknown,
  Vertex,
  Fragment,
  Compute,
};

inline StageType ConvertStageType(SlangStage stage) {
  switch (stage) {
  case SLANG_STAGE_VERTEX:
    return StageType::Vertex;
  case SLANG_STAGE_FRAGMENT:
    return StageType::Fragment;
  case SLANG_STAGE_COMPUTE:
    return StageType::Compute;
  default:
    return StageType::Unknown;
  }
}

struct EntryPoint {
  std::string Name;
  int64_t Idx;
  StageType Stage;

  static EntryPoint FromSlangEntryPoint(int64_t idx, slang::EntryPointReflection *entryPoint) {
    return {entryPoint->getName(), idx, ConvertStageType(entryPoint->getStage())};
  }
};

inline std::string_view getStageShortName(StageType stage) {
  switch (stage) {
  case StageType::Vertex:
    return "vs";
  case StageType::Fragment:
    return "fs";
  case StageType::Compute:
    return "cs";
  default:
    return "unknown";
  }
}

} // namespace BgfxSlang