#pragma once

#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include <cstdint>
#include <slang-com-ptr.h>
#include <slang.h>
#include <vector>

namespace BgfxSlang {

Status writeGlslShader(Slang::ComPtr<slang::IComponentType> &linkedProgram, TargetProfile targetProfile, int64_t entryPointIdx,
                       int64_t targetIdx, IWriter &writer, const std::vector<Param> &inputParams, std::vector<Uniform> &uniforms);
}