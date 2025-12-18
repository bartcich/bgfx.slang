#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include <array>
#include <cstdint>
#include <regex>
#include <slang-com-ptr.h>
#include <slang.h>
#include <spirv_glsl.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace BgfxSlang {

namespace {
constexpr std::string_view entryPointParamPrefix = "entryPointParam_";
}

struct DefaultParam {
  std::string_view Name;
  Attrib Attr;
};

constexpr std::array defaultInputParamNames = {
    DefaultParam("a_position", Attrib::Position),   DefaultParam("a_normal", Attrib::Normal),
    DefaultParam("a_tangent", Attrib::Tangent),     DefaultParam("a_bitangent", Attrib::Bitangent),
    DefaultParam("a_color0", Attrib::Color0),       DefaultParam("a_color1", Attrib::Color1),
    DefaultParam("a_color2", Attrib::Color2),       DefaultParam("a_color3", Attrib::Color3),
    DefaultParam("a_indices", Attrib::Indices),     DefaultParam("a_weight", Attrib::Weight),
    DefaultParam("a_texcoord0", Attrib::TexCoord0), DefaultParam("a_texcoord1", Attrib::TexCoord1),
    DefaultParam("a_texcoord2", Attrib::TexCoord2), DefaultParam("a_texcoord3", Attrib::TexCoord3),
    DefaultParam("a_texcoord4", Attrib::TexCoord4), DefaultParam("a_texcoord5", Attrib::TexCoord5),
    DefaultParam("a_texcoord6", Attrib::TexCoord6), DefaultParam("a_texcoord7", Attrib::TexCoord7),
};

constexpr std::array defaultInputInstanceBufferParamNames = {
    DefaultParam("i_data0", Attrib::TexCoord7), DefaultParam("i_data1", Attrib::TexCoord6), DefaultParam("i_data2", Attrib::TexCoord5),
    DefaultParam("i_data3", Attrib::TexCoord4), DefaultParam("i_data4", Attrib::TexCoord3),
};

std::string_view getDefaultInputName(const Param &param) {
  if (param.Attr >= Attrib::TexCoord3 && param.Name.find("data") != std::string::npos) {
    for (const auto &defaultParam : defaultInputInstanceBufferParamNames) {
      if (defaultParam.Attr == param.Attr) {
        return defaultParam.Name;
      }
    }
  }

  for (const auto &defaultParam : defaultInputParamNames) {
    if (defaultParam.Attr == param.Attr) {
      return defaultParam.Name;
    }
  }
  return "";
}

std::string uniformDeclLine(const Uniform &uniform) {
  std::string result = "uniform ";
  switch (uniform.Type) {
  case UniformType::Vec4:
    result += "vec4 ";
    break;
  case UniformType::Mat4:
    result += "mat4 ";
    break;
  case UniformType::Mat3:
    result += "mat3 ";
    break;
  default:
    return "";
  }

  result += uniform.Name;
  if (uniform.Count > 1) {
    result += "[";
    result += std::to_string(uniform.Count);
    result += "]";
  }
  result += ";\n";

  return result;
}

Uniform getUniformByName(std::string_view name, const std::vector<Uniform> &uniforms) {
  for (const auto &uniform : uniforms) {
    if (uniform.Name == name) {
      return uniform;
    }
  }
  return {};
}

void processOutputName(SlangStage stage, spirv_cross::CompilerGLSL &glsl, const spirv_cross::Resource &input) {
  if (stage == SLANG_STAGE_VERTEX) {
    const auto lastDotPos = input.name.rfind('.');
    if (lastDotPos == std::string::npos) {
      return;
    }
    auto newName = input.name.substr(lastDotPos + 1);
    glsl.set_name(input.id, std::string(entryPointParamPrefix) + newName);
  }
}

void processInputName(SlangStage stage, spirv_cross::CompilerGLSL &glsl, const spirv_cross::Resource &input,
                      const std::vector<Param> &inputParams) {
  if (stage == SLANG_STAGE_VERTEX) {
    for (const auto &inpParam : inputParams) {

      if (input.name == inpParam.QualifiedName) {

        auto defaultName = getDefaultInputName(inpParam);
        glsl.set_name(input.id, std::string(defaultName));
        break;
      }
    }
  } else {
    const auto lastDotPos = input.name.rfind('.');
    if (lastDotPos == std::string::npos) {
      return;
    }
    auto newName = input.name.substr(lastDotPos + 1);
    glsl.set_name(input.id, std::string(entryPointParamPrefix) + newName);
  }
}

Status writeGlslShader(Slang::ComPtr<slang::IComponentType> &linkedProgram, TargetProfile targetProfile, int64_t entryPointIdx,
                       int64_t targetIdx, IWriter &writer, const std::vector<Param> &inputParams, std::vector<Uniform> &uniforms) {
  Slang::ComPtr<slang::IBlob> code;
  Slang::ComPtr<slang::IBlob> diagnostics;
  SlangResult result = linkedProgram->getEntryPointCode(entryPointIdx, targetIdx, code.writeRef(), diagnostics.writeRef());
  if (SLANG_FAILED(result)) {
    return Status{StatusCode::Error, diagnostics};
  }
  auto *layout = linkedProgram->getLayout(targetIdx, diagnostics.writeRef());
  auto *entryPointLayout = layout->getEntryPointByIndex(entryPointIdx);
  auto stage = entryPointLayout->getStage();

  std::string warnings;
  if (diagnostics != nullptr) {
    warnings += static_cast<const char *>(diagnostics->getBufferPointer());
  }

  const auto version = static_cast<uint32_t>(std::stoul(std::string(targetProfile.Id.substr(5))));

  spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t *>(code->getBufferPointer()), code->getBufferSize() / 4);
  spirv_cross::CompilerGLSL::Options options;
  options.version = version;
  options.es = targetProfile.Format == TargetFormat::OpenGLES;
  options.emit_uniform_buffer_as_plain_uniforms = true;
  options.enable_420pack_extension = false;
  options.fragment.default_float_precision = spirv_cross::CompilerGLSL::Options::Precision::Highp;
  glsl.set_common_options(options);

  auto resources = glsl.get_shader_resources();

  for (auto &output : resources.stage_outputs) {
    auto a = output.name;
    processOutputName(stage, glsl, output);
  }

  for (auto &input : resources.stage_inputs) {
    processInputName(stage, glsl, input, inputParams);
  }

  glsl.build_combined_image_samplers();
  for (const auto &sampler : glsl.get_combined_image_samplers()) {
    glsl.set_name(sampler.combined_id, glsl.get_name(sampler.image_id));
  }
  std::string source = glsl.compile();

  for (auto &ubo : resources.uniform_buffers) {

    std::string unfiormsList;

    auto bufferTypeName = ubo.name;
    const auto &bufferName = glsl.get_name(ubo.id);

    auto type = glsl.get_type(ubo.type_id);
    auto memberCount = type.member_types.size();

    for (int i = 0; i < memberCount; i++) {
      const auto &memberName = glsl.get_member_name(ubo.base_type_id, i);
      const auto uniform = getUniformByName(memberName, uniforms);

      unfiormsList += uniformDeclLine(uniform);

      auto memberType = glsl.get_type(type.member_types[i]);
      if (memberType.op == spv::OpTypeStruct) {
        std::string pattern = bufferName + "\\.";
        pattern += memberName + R"(\.data(\[\w+\])?(?:\.data(\[\w+\])?)?)";
        std::regex re(pattern);
        std::string format = memberName + "$1$2";
        source = std::regex_replace(source, re, format);
      }

      // just vec4 etc
      std::string pattern = bufferName + "\\.";
      pattern += memberName + R"(?![A-Za-z])";
      std::regex re(pattern);
      const std::string &format = memberName;
      source = std::regex_replace(source, re, format);
    }

    std::string targetReplace = "uniform " + bufferTypeName + " ";
    targetReplace += bufferName + ";\n";

    source = std::regex_replace(source, std::regex(targetReplace), unfiormsList);
  }

  writer.Write<uint32_t>(source.size());
  writer.Write(source.data(), source.size());
  uint8_t nul = 0;
  writer.Write(nul);
  return !warnings.empty() ? Status{StatusCode::Warning, warnings} : Status{};
}
} // namespace BgfxSlang