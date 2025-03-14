#include "Compiler.h"
#include "BgfxSlang/EntryPoint.h"
#include "BgfxSlang/Status.h"
#include "BgfxSlang/Target.h"
#include "BgfxSlang/TextureData.h"
#include "BgfxSlang/Types.h"
#include "BgfxSlang/Utils/IWriter.h"
#include "Glsl.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <slang-com-ptr.h>
#include <slang.h>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace BgfxSlang {

namespace {
constexpr uint32_t shift1Byte = 8;
constexpr uint32_t shift2Bytes = 16;
constexpr uint32_t shift3Bytes = 24;

constexpr uint8_t kUniformFragmentBit = 0x10;

constexpr uint8_t version = 11;

constexpr uint32_t composeMagic(char a, char b, char c, uint8_t ver) {
  return (static_cast<uint32_t>(a)) | (static_cast<uint32_t>(b) << shift1Byte) | (static_cast<uint32_t>(c) << shift2Bytes) |
         static_cast<uint32_t>(ver) << shift3Bytes;
}

constexpr uint32_t magicCsh = composeMagic('C', 'S', 'H', version);
constexpr uint32_t magicFsh = composeMagic('F', 'S', 'H', version);
constexpr uint32_t magicVsh = composeMagic('V', 'S', 'H', version);

constexpr uint32_t GetMagic(SlangStage stage) {
  switch (stage) {
  case SLANG_STAGE_VERTEX:
    return magicVsh;
  case SLANG_STAGE_FRAGMENT:
    return magicFsh;
  case SLANG_STAGE_COMPUTE:
    return magicCsh;
  default:
    return 0;
  }
}

Attrib semanticNameToAttrib(std::string_view name, uint32_t index) {
  if (name == "POSITION") {
    return Attrib::Position;
  }
  if (name == "NORMAL") {
    return Attrib::Normal;
  }
  if (name == "TANGENT") {
    return Attrib::Tangent;
  }
  if (name == "BITANGENT") {
    return Attrib::Bitangent;
  }
  if (name == "COLOR") {
    return static_cast<Attrib>(static_cast<int>(Attrib::Color0) + index);
  }
  if (name == "TEXCOORD") {
    return static_cast<Attrib>(static_cast<int>(Attrib::TexCoord0) + index);
  }
  if (name == "SV_POSITION" || name == "SV_TARGET") {
    return Attrib::Internal;
  }
  return Attrib::Unknown;
}

Status processInOutParams(slang::VariableLayoutReflection *varLayout, std::vector<Param> &params, const std::string &qualifiedPrefix = "") {
  auto *paramTypeLayout = varLayout->getTypeLayout();

  switch (paramTypeLayout->getKind()) {
  case slang::TypeReflection::Kind::Struct: {
    for (int i = 0; i < paramTypeLayout->getFieldCount(); i++) {
      auto *field = paramTypeLayout->getFieldByIndex(i);
      auto prefix = varLayout->getName() != nullptr ? qualifiedPrefix + varLayout->getName() + "." : "";
      auto status = processInOutParams(field, params, prefix);
      if (!status.IsOk()) {
        return status;
      }
    }
    return Status{};
  }
  case slang::TypeReflection::Kind::Vector:
  case slang::TypeReflection::Kind::Scalar: {
    auto attribType = semanticNameToAttrib(varLayout->getSemanticName(), varLayout->getSemanticIndex());
    if (attribType == Attrib::Unknown) {
      return Status{StatusCode::Error, "Unsupported semantic name: " + std::string(varLayout->getSemanticName())};
    }
    if (attribType != Attrib::Internal) {
      params.push_back(Param{varLayout->getName(), qualifiedPrefix + varLayout->getName(), attribType});
    }
    return Status{};
  }
  default:
    return Status{StatusCode::Error, "Unsupported type of param " + std::string(varLayout->getName())};
  }
}

Status getInputParams(slang::EntryPointReflection *entryPoint, std::vector<Param> &params) {

  for (int i = 0; i < entryPoint->getParameterCount(); i++) {
    auto *param = entryPoint->getParameterByIndex(i);
    auto status = processInOutParams(param, params);
    if (!status.IsOk()) {
      return status;
    }
  }

  return Status{};
}

Status getOutputParams(slang::EntryPointReflection *entryPoint, std::vector<Param> &params) {
  auto *resultVarLayout = entryPoint->getResultVarLayout();

  return processInOutParams(resultVarLayout, params);
}

uint32_t hashParams(const std::vector<Param> &params) {
  uint32_t hash = 0;
  std::vector<std::string> names(params.size());

  std::transform(params.begin(), params.end(), names.begin(), [](const Param &p) { return p.Name; });
  std::sort(names.begin(), names.end());
  std::hash<std::string> hasher;
  for (const auto &name : names) {
    hash ^= hasher(name);
  }
  return hash;
}

bool isSkippableUniform(slang::TypeReflection *type) { return type->getKind() == slang::TypeReflection::Kind::SamplerState; }

UniformType convertUniformType(slang::TypeReflection *type) {
  switch (type->getKind()) {
  case slang::TypeReflection::Kind::Resource:
    return UniformType::Sampler;
  case slang::TypeReflection::Kind::Vector:
    return UniformType::Vec4;
  case slang::TypeReflection::Kind::Matrix:
    return type->getRowCount() == 3 ? UniformType::Mat3 : UniformType::Mat4;
  default:
    return UniformType::Unknown;
  }
}

Status getUniforms(slang::ProgramLayout *programLayout, std::vector<Uniform> &uniforms, uint16_t &uniformBufferSize) {
  auto *globalVarLayout = programLayout->getGlobalParamsVarLayout();
  auto *scopeTypeLayout = globalVarLayout->getTypeLayout();

  slang::VariableLayoutReflection *elementsVarLayout = globalVarLayout;

  if (scopeTypeLayout->getKind() != slang::TypeReflection::Kind::Struct) {
    if (scopeTypeLayout->getKind() != slang::TypeReflection::Kind::ConstantBuffer) {
      return Status{StatusCode::Error, "Global scope is not a struct or constant buffer"};
    }

    elementsVarLayout = scopeTypeLayout->getElementVarLayout();
  }

  auto *elementsTypeLayout = elementsVarLayout->getTypeLayout();

  if (elementsTypeLayout->getKind() != slang::TypeReflection::Kind::Struct) {
    return Status{StatusCode::Error, "Global scope elements are not a struct"};
  }

  uint64_t textureIndex = 0;
  for (int i = 0; i < elementsTypeLayout->getFieldCount(); i++) {
    auto *param = elementsTypeLayout->getFieldByIndex(i);
    Uniform uniform;

    auto *paramType = param->getType();
    bool isArray = paramType->getKind() == slang::TypeReflection::Kind::Array;

    auto *elementType = isArray ? paramType->getElementType() : paramType;

    if (isSkippableUniform(elementType)) {
      continue;
    }

    auto convertedType = convertUniformType(elementType);
    if (convertedType == UniformType::Unknown) {
      return Status{StatusCode::Error, "Unsupported uniform type for param " + std::string(param->getName())};
    }

    bool isSampler = convertedType == UniformType::Sampler;

    uniform.Name = param->getName();
    uniform.Type = convertedType;
    uniform.Count = isArray ? paramType->getElementCount() : 1;
    uniform.RegIndex = isSampler ? param->getBindingIndex() : param->getOffset();
    uniform.RegCount = (isSampler ? 1 : elementType->getRowCount()) * uniform.Count;

    if (isSampler) {
      uniform.TexComponent = convertTextureComponentType(param->getType());
      uniform.TexDimension = convertTextureDimension(param->getType());
      uniform.TexFormat = convertTextureFormat(elementsTypeLayout, textureIndex++);
    }

    uniforms.push_back(uniform);
  }

  uniformBufferSize = static_cast<uint16_t>(elementsTypeLayout->getSize());
  return Status{};
}

} // namespace

Status Compiler::AddTarget(std::string_view profile, std::span<slang::CompilerOptionEntry> additionalCompilerOptions) {
  auto target = findProfile(profile);
  if (target.Format == TargetFormat::Unknown) {
    return Status{StatusCode::Error, "Unknown target profile: " + std::string(profile)};
  }

  TargetSettings settings;
  settings.Profile = target;
  settings.CompilerOptions = std::vector<slang::CompilerOptionEntry>(additionalCompilerOptions.begin(), additionalCompilerOptions.end());

  targets.push_back(settings);
  return Status{};
}

Status Compiler::LoadProgramFromPath(std::string_view path) {

  std::ifstream file;
  file.open(path.data());

  if (!file.is_open()) {
    return Status{StatusCode::Error, "Failed to open file: " + std::string(path)};
  }

  std::string code;
  std::stringstream buffer;
  buffer << file.rdbuf();
  code = buffer.str();

  return LoadProgram(code);
}

Status Compiler::LoadProgram(std::string_view code) {
  writeLog("Loading Program...");
  inputCode = code;
  std::string warnings;

  Slang::ComPtr<slang::IComponentType> linkedProgram;

  if (auto status = processProgram(code, linkedProgram.writeRef()); !status.IsOk()) {
    return status;
  }

  auto *layout = linkedProgram->getLayout(0, diagnostics.writeRef());
  if (layout == nullptr) {
    return Status{StatusCode::Error, diagnostics};
  }
  appendWarnings(warnings, diagnostics);

  auto entryPointCount = layout->getEntryPointCount();
  writeLog("   Found " + std::to_string(entryPointCount) + " entry points:");
  for (int i = 0; i < entryPointCount; i++) {
    auto *ep = layout->getEntryPointByIndex(i);
    auto entryPoint = EntryPoint::FromSlangEntryPoint(i, ep);
    availableEntryPoints.push_back(entryPoint);
    writeLog("      - " + entryPoint.Name + " (" + std::string(getStageShortName(entryPoint.Stage)) + ")");
  }

  return !warnings.empty() ? Status{StatusCode::Warning, warnings} : Status{};
}

Status Compiler::createSession(slang::ISession **outSession, int64_t entryPointIdx, int64_t targetIdx) {
  if (slangGlobalSession == nullptr) {
    writeLog("CreateSession: Creating global session...");
    SlangGlobalSessionDesc slangGlobalSessionDesc;
    slang::createGlobalSession(&slangGlobalSessionDesc, slangGlobalSession.writeRef());
  }

  slang::SessionDesc sessionDesc{};
  sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

  std::vector<std::vector<slang::CompilerOptionEntry>> optionsPerTarget;
  std::vector<slang::TargetDesc> targetDescs;

  if (targetIdx > -1) {
    EntryPoint entryPoint = entryPointIdx > -1 ? entryPointsSource()[entryPointIdx] : EntryPoint{};
    slang::TargetDesc targetDesc;
    TargetSettings &target = targets[targetIdx];
    targetDesc.format = target.Profile.GetSlangTarget();
    targetDesc.profile = slangGlobalSession->findProfile(target.Profile.GetProfile().data());
    optionsPerTarget.push_back(target.GetCompilerOptions(entryPoint.Stage));
    targetDesc.compilerOptionEntryCount = optionsPerTarget.back().size();
    targetDesc.compilerOptionEntries = optionsPerTarget.back().data();
    targetDescs.push_back(targetDesc);

    writeLog("CreateSession: Creating session for target " + std::string(target.Profile.Id) + "...");
  } else {
    if (targets.empty()) {
      return Status{StatusCode::Error, "No targets specified"};
    }
    writeLog("CreateSession: Creating session for preprocess...");
    for (const auto &target : targets) {
      slang::TargetDesc targetDesc;
      targetDesc.format = target.Profile.GetSlangTarget();
      targetDesc.profile = slangGlobalSession->findProfile(target.Profile.Id.data());
      optionsPerTarget.push_back(target.GetCompilerOptions(StageType::Vertex));
      targetDesc.compilerOptionEntryCount = optionsPerTarget.back().size();
      targetDesc.compilerOptionEntries = optionsPerTarget.back().data();
      targetDescs.push_back(targetDesc);
    }
  }
  sessionDesc.targetCount = targetDescs.size();
  sessionDesc.targets = targetDescs.data();

  std::vector<const char *> modulesSearchPathsChar;
  modulesSearchPathsChar.reserve(modulesSearchPaths.size());
  for (const auto &path : modulesSearchPaths) {
    modulesSearchPathsChar.push_back(path.c_str());
  }

  sessionDesc.searchPathCount = modulesSearchPaths.size();
  sessionDesc.searchPaths = modulesSearchPathsChar.data();

  slangGlobalSession->createSession(sessionDesc, outSession);

  return Status{};
}

Status Compiler::processProgram(std::string_view code, slang::IComponentType **outProgram, int64_t entryPointIdx, int64_t targetIdx) {
  slang::ISession *session;
  std::string warnings;

  if (auto status = createSession(&session, entryPointIdx, targetIdx); !status.IsOk()) {
    return status;
  }

  slang::IModule *module = session->loadModuleFromSourceString("sh", "sh.slang", code.data(), diagnostics.writeRef());
  if (module == nullptr) {
    return Status{StatusCode::Error, diagnostics};
  }
  appendWarnings(warnings, diagnostics);

  std::vector<slang::IComponentType *> components;
  components.reserve(module->getDefinedEntryPointCount() + 1);
  components.push_back(module);

  if (entryPointIdx > -1) {
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    module->getDefinedEntryPoint(entryPointsSource()[entryPointIdx].Idx, entryPoint.writeRef());
    components.push_back(entryPoint);

  } else {
    for (int i = 0; i < module->getDefinedEntryPointCount(); i++) {
      Slang::ComPtr<slang::IEntryPoint> entryPoint;
      module->getDefinedEntryPoint(i, entryPoint.writeRef());
      components.push_back(entryPoint);
    }
  }

  Slang::ComPtr<slang::IComponentType> program;
  writeLog("LoadProgram: Creating composite component type...");

  SlangResult res1 =
      session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());
  if (SLANG_FAILED(res1)) {
    return Status{StatusCode::Error, diagnostics};
  }
  appendWarnings(warnings, diagnostics);

  writeLog("LoadProgram: Linking program...");
  SlangResult res2 = program->link(outProgram, diagnostics.writeRef());
  if (SLANG_FAILED(res2)) {
    return Status{StatusCode::Error, diagnostics};
  }
  appendWarnings(warnings, diagnostics);

  return !warnings.empty() ? Status{StatusCode::Warning, warnings} : Status{};
}

Status Compiler::AddEntryPoint(std::string_view name) {
  for (const auto &entryPoint : availableEntryPoints) {
    if (entryPoint.Name == name) {
      selectedEntryPoints.push_back(entryPoint);
      return Status{};
    }
  }
  return Status{StatusCode::Error, "Entry point not found: " + std::string(name)};
}

Status Compiler::AddEntryPoint(StageType stage) {
  bool found = false;
  for (const auto &entryPoint : availableEntryPoints) {
    if (entryPoint.Stage == stage) {
      selectedEntryPoints.push_back(entryPoint);
      found = true;
    }
  }
  if (found) {
    return Status{};
  }
  return Status{StatusCode::Error, "Entry point not found for stage: " + std::string(getStageShortName(stage))};
}

Status Compiler::Compile(int64_t entryPointIdx, int64_t targetIdx, IWriter &writer) {
  Slang::ComPtr<slang::IComponentType> linkedProgram;

  if (auto status = processProgram(inputCode, linkedProgram.writeRef(), entryPointIdx, targetIdx); !status.IsOk()) {
    return status;
  }

  auto target = targets[targetIdx].Profile;

  std::string warnings;
  int64_t processedTargetIndex = 0;   // we always compile one target at a time
  int64_t processedEntryPointIdx = 0; // we always compile one entry point at a time

  auto *layout = linkedProgram->getLayout(processedTargetIndex, diagnostics.writeRef());

  if (layout == nullptr) {
    return Status{StatusCode::Error, diagnostics};
  }

  if (diagnostics != nullptr) {
    appendWarnings(warnings, diagnostics);
  }

  Slang::ComPtr<ISlangBlob> hash;

  std::vector<Param> inputParams;
  std::vector<Param> outputParams;
  if (auto status = getInputParams(layout->getEntryPointByIndex(processedEntryPointIdx), inputParams); !status.IsOk()) {
    return status;
  }
  if (auto status = getOutputParams(layout->getEntryPointByIndex(processedEntryPointIdx), outputParams); !status.IsOk()) {
    return status;
  }
  std::vector<Uniform> uniforms;
  uint16_t uniformBufferSize = 0;
  if (auto status = getUniforms(layout, uniforms, uniformBufferSize); !status.IsOk()) {
    return status;
  }

  if (verboseWriter != nullptr) {
    writeLog("   Found " + std::to_string(inputParams.size()) + " input params:");
    for (const auto &param : inputParams) {
      writeLog("      - " + param.Name + " (" + std::string(attribToString(param.Attr)) + ")");
    }
    writeLog("   Found " + std::to_string(outputParams.size()) + " output params:");
    for (const auto &param : outputParams) {
      writeLog("      - " + param.Name + " (" + std::string(attribToString(param.Attr)) + ")");
    }
    writeLog("   Found " + std::to_string(uniforms.size()) + " uniforms:");
    for (const auto &uniform : uniforms) {
      writeLog("      - " + uniform.Name + " (" + std::string(uniformTypeToString(uniform.Type)) + ")");
    }
  }

  Slang::ComPtr<slang::IBlob> code;
  SlangResult result =
      linkedProgram->getEntryPointCode(processedEntryPointIdx, processedTargetIndex, code.writeRef(), diagnostics.writeRef());
  if (SLANG_FAILED(result)) {
    return Status{StatusCode::Error, diagnostics};
  }
  if (diagnostics != nullptr) {
    appendWarnings(warnings, diagnostics);
  }

  std::string source(reinterpret_cast<const char *>(code->getBufferPointer()), code->getBufferSize());

  auto *entryPointLayout = layout->getEntryPointByIndex(processedEntryPointIdx);
  auto stage = entryPointLayout->getStage();
  auto magic = GetMagic(stage);
  if (magic == 0) {
    return Status{StatusCode::Error, "Unsupported stage"};
  }

  writer.Write(magic);

  auto hashInput = magic == magicFsh ? hashParams(inputParams) : 0;
  auto hashOutput = magic == magicVsh ? hashParams(outputParams) : 0;
  writer.Write(hashInput);
  writer.Write(hashOutput);

  writer.Write<uint16_t>(uniforms.size());

  const uint32_t fragmentBit = stage == SLANG_STAGE_FRAGMENT ? kUniformFragmentBit : 0;

  for (const auto &uniform : uniforms) {
    uint8_t nameSize = uniform.Name.size();
    writer.Write(nameSize);
    writer.Write(uniform.Name.data(), nameSize);
    uint8_t type = static_cast<uint8_t>(uniform.Type) | fragmentBit;
    writer.Write(type);
    writer.Write(uniform.Count);
    writer.Write(uniform.RegIndex);
    writer.Write(uniform.RegCount);
    writer.Write(uniform.TexComponent);
    writer.Write(uniform.TexDimension);
    writer.Write(uniform.TexFormat);
  }

  if (target.Format == TargetFormat::OpenGL) {
    return writeGlslShader(linkedProgram, target, processedEntryPointIdx, processedTargetIndex, writer, inputParams, uniforms);
  }

  uint32_t codeSize = code->getBufferSize();
  writer.Write(codeSize);
  writer.Write(code->getBufferPointer(), codeSize);

  uint8_t nul = 0;
  writer.Write(nul);

  writer.Write<uint8_t>(inputParams.size());
  for (const auto &param : inputParams) {
    uint16_t attrId = paramToId(param, target.Format);
    writer.Write(attrId);
  }

  writer.Write(uniformBufferSize);

  return !warnings.empty() ? Status{StatusCode::Warning, warnings} : Status{};
}

EntryPoint Compiler::GetEntryPointByIndex(int64_t idx) const { return entryPointsSource()[idx]; }

EntryPoint Compiler::GetEntryPointByName(std::string_view name) const {
  for (const auto &entryPoint : entryPointsSource()) {
    if (entryPoint.Name == name) {
      return entryPoint;
    }
  }
  return {};
}

} // namespace BgfxSlang