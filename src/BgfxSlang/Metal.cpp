#include "Metal.h"
#include "Glsl.h"
#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <slang.h>
#include <spirv_cross_error_handling.hpp>
#include <spirv_msl.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace BgfxSlang {

namespace {
// Names and slots bgfx's Metal renderer looks up through Metal reflection
constexpr std::string_view entryPointName = "xlatMtlMain";
constexpr std::string_view uniformBufferName = "_mtl_u";
constexpr std::string_view std140BufferName = "_mtl_u_std140";
constexpr std::string_view uniformStructName = "xlatMtlUniforms";
constexpr uint32_t uniformBufferSlot = 0;
constexpr size_t vec4Size = 16;

// https://github.com/bkaradzic/bgfx/issues/2822
constexpr std::string_view vertexOutputStructHead = "xlatMtlMain_out\n{";
constexpr std::string_view pointSizeMember = "\n    float bgfx_metal_pointSize [[point_size]] = 1;";

// Profile ids are "<platform>_<major>_<minor>", e.g. "macos_2_1"
void applyProfile(std::string_view id, spirv_cross::CompilerMSL::Options &options) {
  const auto majorPos = id.find('_') + 1;
  const auto minorPos = id.find('_', majorPos) + 1;

  options.platform =
      id.starts_with("ios") ? spirv_cross::CompilerMSL::Options::Platform::iOS : spirv_cross::CompilerMSL::Options::Platform::macOS;
  options.set_msl_version(std::stoul(std::string(id.substr(majorPos, minorPos - majorPos - 1))),
                          std::stoul(std::string(id.substr(minorPos))));
}

// Returns the MSL type of a uniform bgfx can set (vec4, mat3, mat4), or an empty string
std::string_view getMslUniformType(slang::TypeReflection *type, size_t &size) {
  if (type->getScalarType() != slang::TypeReflection::ScalarType::Float32) {
    return {};
  }
  if (type->getKind() == slang::TypeReflection::Kind::Vector && type->getElementCount() == 4) {
    size = vec4Size;
    return "float4";
  }
  if (type->getKind() == slang::TypeReflection::Kind::Matrix && type->getRowCount() == type->getColumnCount()) {
    size = type->getRowCount() * vec4Size;
    if (type->getRowCount() == 3) {
      return "float3x3";
    }
    if (type->getRowCount() == 4) {
      return "float4x4";
    }
  }
  return {};
}

// slang wraps std140 matrices and arrays in structs, which bgfx's Metal reflection can't read, so the
// uniform buffer is exposed as plain float4/float3x3/float4x4 members with the same layout.
// Returns an empty string when a member has no plain equivalent.
std::string buildUniformStruct(slang::TypeLayoutReflection *globals) {
  std::string result = "struct " + std::string(uniformStructName) + "\n{\n";
  size_t cursor = 0;

  for (unsigned i = 0; i < globals->getFieldCount(); i++) {
    auto *field = globals->getFieldByIndex(i);
    if (field->getTypeLayout()->getSize(slang::ParameterCategory::Uniform) == 0) {
      continue;
    }

    auto *type = field->getType();
    const bool isArray = type->getKind() == slang::TypeReflection::Kind::Array;
    size_t size = 0;
    auto mslType = getMslUniformType(isArray ? type->getElementType() : type, size);
    if (mslType.empty() || field->getOffset(slang::ParameterCategory::Uniform) != cursor) {
      return {};
    }

    result += "    " + std::string(mslType) + " " + field->getName();
    if (isArray) {
      result += "[" + std::to_string(type->getElementCount()) + "]";
      size *= type->getElementCount();
    }
    result += ";\n";
    cursor += size;
  }

  result += "};\n\n";
  return result;
}

// Swaps the std140 entry point parameter for the flat struct and aliases the std140 view onto it
bool exposeFlatUniforms(std::string &source, slang::TypeLayoutReflection *globals) {
  const std::string param = "& " + std::string(std140BufferName) + " [[buffer(0)]]";
  const auto paramPos = source.find(param);
  if (paramPos == std::string::npos) {
    return true;
  }

  const auto uniformStruct = buildUniformStruct(globals);
  if (uniformStruct.empty()) {
    return false;
  }

  constexpr std::string_view addressSpace = "constant ";
  const auto declPos = source.rfind(addressSpace, paramPos);
  if (declPos == std::string::npos) {
    return false;
  }
  const auto std140Type = source.substr(declPos + addressSpace.size(), paramPos - declPos - addressSpace.size());

  const auto bodyPos = source.find('{', paramPos);
  if (bodyPos == std::string::npos) {
    return false;
  }
  source.insert(bodyPos + 1, "\n    constant " + std140Type + "& " + std::string(std140BufferName) + " = *reinterpret_cast<constant " +
                                 std140Type + "*>(&" + std::string(uniformBufferName) + ");");

  source.replace(declPos, paramPos + param.size() - declPos,
                 "constant " + std::string(uniformStructName) + "& " + std::string(uniformBufferName) + " [[buffer(0)]]");

  const auto lineStart = source.rfind('\n', declPos);
  source.insert(lineStart == std::string::npos ? 0 : lineStart + 1, uniformStruct);
  return true;
}

// Shaders use HLSL registers (all Vulkan binding shifts are 0), so a binding is the bgfx stage index.
// bgfx binds textures and samplers at their stage and storage buffers at stage + 1, after the uniform buffer.
void assignBindings(spirv_cross::CompilerMSL &msl, const spirv_cross::ShaderResources &resources) {
  for (const auto &ubo : resources.uniform_buffers) {
    msl.set_name(ubo.id, std::string(std140BufferName));
    msl.set_decoration(ubo.id, spv::DecorationBinding, uniformBufferSlot);
  }

  for (const auto &buffer : resources.storage_buffers) {
    const auto binding = msl.get_decoration(buffer.id, spv::DecorationBinding);
    msl.set_decoration(buffer.id, spv::DecorationBinding, binding + 1);
  }
}

void addPointSize(std::string &source) {
  auto pos = source.find(vertexOutputStructHead);
  if (pos != std::string::npos) {
    source.insert(pos + vertexOutputStructHead.size(), pointSizeMember);
  }
}
} // namespace

Status writeMetalShader(slang::IBlob *spirv, slang::TypeLayoutReflection *globals, TargetProfile targetProfile, SlangStage stage,
                        IWriter &writer, const std::vector<Param> &inputParams) {
  std::string source;
  std::array<uint16_t, 3> numThreads{};

  try {
    spirv_cross::CompilerMSL msl(reinterpret_cast<const uint32_t *>(spirv->getBufferPointer()), spirv->getBufferSize() / 4);

    auto options = msl.get_msl_options();
    applyProfile(targetProfile.Id, options);
    options.enable_decoration_binding = true;
    msl.set_msl_options(options);

    auto entryPoints = msl.get_entry_points_and_stages();
    if (!entryPoints.empty()) {
      msl.rename_entry_point(entryPoints[0].name, std::string(entryPointName), entryPoints[0].execution_model);
    }

    auto resources = msl.get_shader_resources();
    if (resources.uniform_buffers.size() > 1) {
      return Status{StatusCode::Error, "Metal supports only the global uniforms; move cbuffers and entry point uniforms into globals"};
    }
    if (stage == SLANG_STAGE_VERTEX) {
      for (const auto &input : resources.stage_inputs) {
        renameVertexInput(msl, input, inputParams);
      }
    }
    assignBindings(msl, resources);

    source = msl.compile();

    for (uint32_t i = 0; i < numThreads.size(); i++) {
      numThreads.at(i) = static_cast<uint16_t>(msl.get_execution_mode_argument(spv::ExecutionModeLocalSize, i));
    }
  } catch (const spirv_cross::CompilerError &error) {
    return Status{StatusCode::Error, std::string("SPIRV-Cross MSL error: ") + error.what()};
  }

  if (globals != nullptr && !exposeFlatUniforms(source, globals)) {
    return Status{StatusCode::Error, "Failed to expose the uniform buffer as plain float4/float3x3/float4x4 members"};
  }

  if (stage == SLANG_STAGE_VERTEX) {
    addPointSize(source);
  }

  if (stage == SLANG_STAGE_COMPUTE) {
    for (auto dim : numThreads) {
      writer.Write(dim);
    }
  }

  writer.Write<uint32_t>(source.size());
  writer.Write(source.data(), source.size());
  uint8_t nul = 0;
  writer.Write(nul);
  return Status{};
}
} // namespace BgfxSlang
