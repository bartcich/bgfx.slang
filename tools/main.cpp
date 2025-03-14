#include "BgfxSlang/Compiler.h"
#include "BgfxSlang/EntryPoint.h"
#include "BgfxSlang/Status.h"
#include "BgfxSlang/Target.h"
#include "BgfxSlang/Utils/Bin2cWriter.h"
#include "BgfxSlang/Utils/ConsoleWriter.h"
#include "BgfxSlang/Utils/FileWriter.h"
#include "Utils/CmdLine.h"
#include "Utils/StringFormat.h"
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

void verifyStatus(const BgfxSlang::Status &status) {
  if (!status.IsOk()) {
    std::cerr << status.GetMessage() << '\n';
    if (status.IsError()) {
      exit(1);
    }
  }
}

void validateArgs(const BgfxSlangCmd::CmdLine &cmdLine) {
  if (!cmdLine.Has(BgfxSlangCmd::TokenType::Input)) {
    std::cerr << "Input file is not specified\n";
    exit(1);
  }
  if (cmdLine.GetCount(BgfxSlangCmd::TokenType::Input) > 1) {
    std::cerr << "Only one input file is allowed\n";
    exit(1);
  }
  if (!cmdLine.Has(BgfxSlangCmd::TokenType::Target)) {
    std::cerr << "At least one target needs to be specified\n";
    exit(1);
  }
}

void printLog(bool verbose, std::string_view message) {
  if (verbose) {
    std::cout << message << '\n';
  }
}

std::string formatOutputPath(std::string_view format, const std::filesystem::path &inputPath, const BgfxSlang::TargetProfile &target,
                             const BgfxSlang::EntryPoint &entryPoint) {

  return BgfxSlangCmd::formatString(format, {{"{{name}}", inputPath.stem().string()},
                                             {"{{entryPoint}}", entryPoint.Name},
                                             {"{{stage}}", BgfxSlang::getStageShortName(entryPoint.Stage)},
                                             {"{{target}}", BgfxSlang::GetTargetShortName(target)}});
}

int main(int argc, char **argv) {
  BgfxSlangCmd::CmdLine cmdLine(argc, argv);
  validateArgs(cmdLine);

  std::string_view outputFormat = "{{target}}/{{name}}_{{stage}}.bin";

  if (cmdLine.Has(BgfxSlangCmd::TokenType::Output)) {
    outputFormat = cmdLine.GetOne(BgfxSlangCmd::TokenType::Output);
  }

  auto targetCount = cmdLine.GetCount(BgfxSlangCmd::TokenType::Target);
  auto verbose = cmdLine.Has(BgfxSlangCmd::TokenType::Verbose);
  auto bin2C = cmdLine.Has(BgfxSlangCmd::TokenType::Bin2C);

  std::string_view bin2CVarFormat = "{{name}}_{{stage}}_{{target}}";
  if (cmdLine.Has(BgfxSlangCmd::TokenType::Bin2C)) {
    bin2CVarFormat = cmdLine.GetOne(BgfxSlangCmd::TokenType::Bin2C, bin2CVarFormat);
  }

  BgfxSlang::Compiler compiler;
  BgfxSlang::ConsoleWriter writer;

  if (cmdLine.Has(BgfxSlangCmd::TokenType::Include)) {
    for (const auto includePath : *cmdLine.Get(BgfxSlangCmd::TokenType::Include)) {
      compiler.AddModulesSearchPath(includePath);
    }
  }

  if (verbose) {
    // job.SetVerboseWriter(&writer);
    compiler.SetVerboseWriter(&writer);
  }

  for (const auto &target : *cmdLine.Get(BgfxSlangCmd::TokenType::Target)) {
    printLog(verbose, "Adding target: " + std::string(target));
    verifyStatus(compiler.AddTarget(target));
  }

  // printLog(verbose, "Creating session...");
  // verifyStatus(job.CreateSession());

  auto inputPath = cmdLine.GetOne(BgfxSlangCmd::TokenType::Input);
  printLog(verbose, "Loading program: " + std::string(inputPath) + "...");
  // verifyStatus(job.LoadProgram(inputPath));
  verifyStatus(compiler.LoadProgramFromPath(inputPath));
  std::filesystem::path inputFilePath{inputPath};

  for (int64_t targetIdx = 0; targetIdx < targetCount; targetIdx++) {

    auto target = compiler.GetTarget(targetIdx);

    for (uint64_t i = 0; i < compiler.GetEntryPointCount(); i++) {
      auto entryPoint = compiler.GetEntryPointByIndex(i);

      std::string outputPath = formatOutputPath(outputFormat, inputFilePath, target, entryPoint);

      printLog(verbose, "Compiling entry point '" + entryPoint.Name + "' (" + std::string(BgfxSlang::getStageShortName(entryPoint.Stage)) +
                            ") to: " + outputPath);

      std::unique_ptr<BgfxSlang::FileWriter> writer;
      if (bin2C) {
        writer = std::make_unique<BgfxSlang::Bin2cWriter>(formatOutputPath(bin2CVarFormat, inputFilePath, target, entryPoint));
      } else {
        writer = std::make_unique<BgfxSlang::FileWriter>();
      }
      std::filesystem::create_directory(std::filesystem::path{outputPath}.parent_path());
      if (!writer->Open(outputPath)) {
        std::cerr << "Failed to open file: " << outputPath << '\n';
        exit(1);
      }
      verifyStatus(compiler.Compile(entryPoint.Idx, targetIdx, *writer));
      writer->Close();
    }
  }
}