#pragma once

#include "EntryPoint.h"
#include "Status.h"
#include "Target.h"
#include "Utils/IWriter.h"
#include <cstdint>
#include <slang-com-ptr.h>
#include <slang.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace BgfxSlang {

class Compiler {
public:
  Compiler() = default;
  Compiler(const Compiler &) = delete;
  Compiler &operator=(const Compiler &) = delete;
  Compiler(Compiler &&) = delete;
  Compiler &operator=(Compiler &&) = delete;

  void SetVerboseWriter(IWriter *writer) { verboseWriter = writer; }

  Status AddTarget(std::string_view profile, std::span<slang::CompilerOptionEntry> additionalCompilerOptions = {});
  Status LoadProgram(std::string_view code);
  Status LoadProgramFromPath(std::string_view path);

  Status AddEntryPoint(std::string_view name);
  Status AddEntryPoint(StageType stage);

  void AddModulesSearchPath(std::string_view path) { modulesSearchPaths.emplace_back(path); }

  Status Compile(int64_t entryPointIdx, int64_t targetIdx, IWriter &writer);

  [[nodiscard]] inline TargetProfile GetTarget(int64_t idx) const { return targets.at(idx).Profile; }
  [[nodiscard]] inline int64_t GetTargetCount() const { return targets.size(); }

  [[nodiscard]] inline uint64_t GetEntryPointCount() const { return entryPointsSource().size(); };
  [[nodiscard]] const EntryPoint *GetEntryPointByName(std::string_view name) const;
  [[nodiscard]] const EntryPoint *GetEntryPointByIndex(int64_t idx) const;

private:
  IWriter *verboseWriter = nullptr;
  std::vector<TargetSettings> targets;
  std::vector<std::string> modulesSearchPaths;
  Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;
  Slang::ComPtr<slang::IBlob> diagnostics;

  std::string inputCode;
  std::vector<EntryPoint> availableEntryPoints;
  std::vector<EntryPoint> selectedEntryPoints;

  Status createSession(slang::ISession **outSession, int64_t entryPointIdx, int64_t targetIdx);

  Status processProgram(std::string_view code, slang::IComponentType **outProgram, int64_t entryPointIdx = -1, int64_t targetIdx = -1);

  [[nodiscard]] const std::vector<EntryPoint> &entryPointsSource() const {
    return selectedEntryPoints.empty() ? availableEntryPoints : selectedEntryPoints;
  }

  inline void appendWarnings(std::string &warnings, slang::IBlob *diagnostics) {
    if (diagnostics != nullptr) {
      warnings += static_cast<const char *>(diagnostics->getBufferPointer());
    }
  }

  inline void writeLog(std::string_view message) {
    if (verboseWriter != nullptr) {
      verboseWriter->Write(message);
    }
  }
};
} // namespace BgfxSlang