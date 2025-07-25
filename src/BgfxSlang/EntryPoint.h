#pragma once

#include "Attributes.h"
#include <algorithm>
#include <cstdint>
#include <slang.h>
#include <string>
#include <string_view>
#include <vector>

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
  std::vector<UserAttribute> Attributes;

  [[nodiscard]] bool HasUserAttribute(std::string_view name) const {
    return std::ranges::any_of(Attributes, [name](const auto &attr) { return attr.GetName() == name; });
  }

  UserAttribute *GetUserAttribute(std::string_view name) {
    for (auto &attr : Attributes) {
      if (attr.GetName() == name) {
        return &attr;
      }
    }
    return nullptr;
  }

  [[nodiscard]] inline bool IsValid() const { return Idx >= 0; }

  static EntryPoint FromSlangEntryPoint(int64_t idx, slang::EntryPointReflection *entryPoint) {
    EntryPoint ep = {entryPoint->getName(), idx, ConvertStageType(entryPoint->getStage())};
    auto *fun = entryPoint->getFunction();
    for (int i = 0; i < fun->getUserAttributeCount(); ++i) {
      ep.Attributes.push_back(UserAttribute::FromSlangAttribute(fun->getUserAttributeByIndex(i)));
    }
    return ep;
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

inline StageType getStageTypeFromShortName(std::string_view shortName) {
  if (shortName == "vs") {
    return StageType::Vertex;
  }
  if (shortName == "fs") {
    return StageType::Fragment;
  }
  if (shortName == "cs") {
    return StageType::Compute;
  }
  return StageType::Unknown;
}

} // namespace BgfxSlang