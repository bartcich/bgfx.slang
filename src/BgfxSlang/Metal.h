#pragma once

#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include <slang.h>
#include <vector>

namespace BgfxSlang {

Status writeMetalShader(slang::IBlob *spirv, slang::TypeLayoutReflection *globals, TargetProfile targetProfile, SlangStage stage,
                        IWriter &writer, const std::vector<Param> &inputParams);
}
