#pragma once

#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include <cstdint>
#include <slang-com-ptr.h>
#include <slang.h>
#include <spirv_glsl.hpp>
#include <vector>

namespace BgfxSlang {

void renameVertexInput(spirv_cross::CompilerGLSL &compiler, const spirv_cross::Resource &input, const std::vector<Param> &inputParams);

Status writeGlslShader(Slang::ComPtr<slang::IComponentType> &linkedProgram, TargetProfile targetProfile, int64_t entryPointIdx,
                       int64_t targetIdx, IWriter &writer, const std::vector<Param> &inputParams, std::vector<Uniform> &uniforms);
} // namespace BgfxSlang