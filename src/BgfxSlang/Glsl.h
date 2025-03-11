#pragma once

#include "BgfxSlang/Status.h"
#include "BgfxSlang/Target.h"
#include "BgfxSlang/Types.h"
#include "BgfxSlang/Utils/IWriter.h"
#include <cstdint>
#include <slang-com-ptr.h>
#include <slang.h>
#include <vector>

namespace BgfxSlang {

Status writeGlslShader(Slang::ComPtr<slang::IComponentType> &linkedProgram, TargetProfile targetProfile, int64_t entryPointIdx,
                       int64_t targetIdx, IWriter &writer, const std::vector<Param> &inputParams, std::vector<Uniform> &uniforms);
}