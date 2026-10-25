#include "Status.h"
#include "Target.h"
#include "Types.h"
#include "Utils/IWriter.h"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include <array>
#include <cstddef>
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

class PlainUniformCompiler final : public spirv_cross::CompilerGLSL {
public:
  using spirv_cross::CompilerGLSL::CompilerGLSL;

protected:
  bool is_non_native_row_major_matrix(uint32_t id) override {
    const auto *expression = maybe_get<spirv_cross::SPIRExpression>(id);
    if (expression != nullptr && expression->need_transpose) {
      return true;
    }
    return spirv_cross::CompilerGLSL::is_non_native_row_major_matrix(id);
  }

  void emit_instruction(const spirv_cross::Instruction &instruction) override {
    const auto opcode = static_cast<spv::Op>(instruction.op);
    const bool accessesMemory = opcode == spv::OpAccessChain || opcode == spv::OpInBoundsAccessChain || opcode == spv::OpLoad;
    const bool nativeRowMajor = backend.native_row_major_matrix;
    if (accessesMemory) {
      const auto *variable = maybe_get_backing_variable(stream(instruction)[2]);
      // Plain uniforms cannot preserve row_major layout qualifiers. Let SPIRV-Cross
      // lower their matrix loads and indexing, while retaining native layouts for SSBOs.
      if (variable != nullptr && variable->storage == spv::StorageClassUniform &&
          has_decoration(get_type(variable->basetype).self, spv::DecorationBlock)) {
        backend.native_row_major_matrix = false;
      }
    }
    spirv_cross::CompilerGLSL::emit_instruction(instruction);
    backend.native_row_major_matrix = nativeRowMajor;
  }
};

constexpr std::string_view entryPointParamPrefix = "entryPointParam_";

bool consumeBalancedSquareBrackets(const std::string &s, size_t &i) {
  if (i >= s.size() || s[i] != '[') {
    return false;
  }

  int depth = 0;
  while (i < s.size()) {
    char c = s[i++];

    if (c == '[') {
      depth++;
    } else if (c == ']') {
      depth--;
      if (depth == 0) {
        return true;
      }
    } else if (c == '"' || c == '\'') {
      char q = c;
      while (i < s.size()) {
        char d = s[i++];
        if (d == '\\' && i < s.size()) {
          i++;
          continue;
        }
        if (d == q) {
          break;
        }
      }
    }
  }
  return false;
}

void consumeAnyIndexing(const std::string &s, size_t &i) {
  while (i < s.size() && s[i] == '[') {
    size_t tmp = i;
    if (!consumeBalancedSquareBrackets(s, tmp)) {
      break;
    }
    i = tmp;
  }
}

bool startsWithAt(const std::string &s, size_t i, const std::string &needle) { return s.compare(i, needle.size(), needle) == 0; }

void removeDataAfter(std::string &source, const std::string &bufferName, const std::string &memberName) {
  const std::string head = bufferName + "." + memberName;
  const std::string dataTok = ".data";

  size_t pos = 0;
  while ((pos = source.find(head, pos)) != std::string::npos) {
    size_t i = pos + head.size();

    consumeAnyIndexing(source, i);

    for (int k = 0; k < 2; k++) {
      if (!startsWithAt(source, i, dataTok)) {
        break;
      }

      source.erase(i, dataTok.size());
      consumeAnyIndexing(source, i);
    }

    pos = i;
  }
}
} // namespace

struct DefaultParam {
  std::string_view Name;
  Attrib Attr;
};

constexpr std::array defaultInputParamNames = {
    DefaultParam("a_position", Attrib::Position),     DefaultParam("a_normal", Attrib::Normal),
    DefaultParam("a_tangent", Attrib::Tangent),       DefaultParam("a_bitangent", Attrib::Bitangent),
    DefaultParam("a_color0", Attrib::Color0),         DefaultParam("a_color1", Attrib::Color1),
    DefaultParam("a_color2", Attrib::Color2),         DefaultParam("a_color3", Attrib::Color3),
    DefaultParam("a_indices", Attrib::Indices),       DefaultParam("a_weight", Attrib::Weight),
    DefaultParam("a_texcoord0", Attrib::TexCoord0),   DefaultParam("a_texcoord1", Attrib::TexCoord1),
    DefaultParam("a_texcoord2", Attrib::TexCoord2),   DefaultParam("a_texcoord3", Attrib::TexCoord3),
    DefaultParam("a_texcoord4", Attrib::TexCoord4),   DefaultParam("a_texcoord5", Attrib::TexCoord5),
    DefaultParam("a_texcoord6", Attrib::TexCoord6),   DefaultParam("a_texcoord7", Attrib::TexCoord7),
    DefaultParam("a_texcoord8", Attrib::TexCoord8),   DefaultParam("a_texcoord9", Attrib::TexCoord9),
    DefaultParam("a_texcoord10", Attrib::TexCoord10), DefaultParam("a_texcoord11", Attrib::TexCoord11),
    DefaultParam("a_texcoord12", Attrib::TexCoord12), DefaultParam("a_texcoord13", Attrib::TexCoord13),
    DefaultParam("a_texcoord14", Attrib::TexCoord14), DefaultParam("a_texcoord15", Attrib::TexCoord15),
};

constexpr std::array defaultInputInstanceBufferParamNames = {
    DefaultParam("i_data0", Attrib::InstanceData0),   DefaultParam("i_data1", Attrib::InstanceData1),
    DefaultParam("i_data2", Attrib::InstanceData2),   DefaultParam("i_data3", Attrib::InstanceData3),
    DefaultParam("i_data4", Attrib::InstanceData4),   DefaultParam("i_data5", Attrib::InstanceData5),
    DefaultParam("i_data6", Attrib::InstanceData6),   DefaultParam("i_data7", Attrib::InstanceData7),
    DefaultParam("i_data8", Attrib::InstanceData8),   DefaultParam("i_data9", Attrib::InstanceData9),
    DefaultParam("i_data10", Attrib::InstanceData10), DefaultParam("i_data11", Attrib::InstanceData11),
    DefaultParam("i_data12", Attrib::InstanceData12), DefaultParam("i_data13", Attrib::InstanceData13),
    DefaultParam("i_data14", Attrib::InstanceData14), DefaultParam("i_data15", Attrib::InstanceData15),
};

std::string_view getDefaultInputName(const Param &param) {
  if (isInstanceAttrib(param.Attr)) {
    return defaultInputInstanceBufferParamNames.at(static_cast<size_t>(param.Attr) - static_cast<size_t>(Attrib::InstanceData0)).Name;
  }

  for (const auto &defaultParam : defaultInputParamNames) {
    if (defaultParam.Attr == param.Attr) {
      return defaultParam.Name;
    }
  }
  return "";
}

std::string uniformDeclLine(const Uniform &uniform, bool isArray) {
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
  if (isArray || uniform.Count > 1) {
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

  PlainUniformCompiler glsl(reinterpret_cast<const uint32_t *>(code->getBufferPointer()), code->getBufferSize() / 4);
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

  glsl.build_dummy_sampler_for_combined_images();
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

      auto memberType = glsl.get_type(type.member_types[i]);
      bool isArray = !memberType.array.empty();
      if (memberType.op == spv::OpTypeStruct && memberType.member_types.size() == 1 && glsl.get_member_name(memberType.self, 0) == "data") {
        isArray = isArray || !glsl.get_type(memberType.member_types[0]).array.empty();
      }
      unfiormsList += uniformDeclLine(uniform, isArray);
      if (memberType.op == spv::OpTypeStruct) {
        // remove all .data after bufferName.memberName
        // for example globalParams.lightBuffer.data[0] to globalParams.lightBuffer[0] etc.
        removeDataAfter(source, bufferName, memberName);
      }

      // just vec4 etc
      std::string pattern = bufferName + "\\.";
      pattern += memberName + "(?![A-Za-z0-9_])";
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
