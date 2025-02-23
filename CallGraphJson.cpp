#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CommandLine.h"
#include <string>
#include <unordered_map>
#include <set>

using namespace llvm;

// Command-line option to specify the output file.
static cl::opt<std::string>
    OutputFilename("cg-json-output",
                   cl::desc("Output file for call graph JSON"),
                   cl::init("callgraph.json"));

struct FuncInfo {
  std::string name;
  std::string filePath;
  unsigned line;
  unsigned column;
};

struct CallGraphJSONPass : public PassInfoMixin<CallGraphJSONPass> {
  // Run the pass on the module.
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    std::unordered_map<const Function*, FuncInfo> funcInfos;
    for (Function &F : M) {
      if (F.isDeclaration())
        continue;
      FuncInfo info;
      info.name = F.getName().str();
      info.line = 0;
      info.column = 0;
      if (DISubprogram *SP = F.getSubprogram()) {
        if (DIFile *DF = SP->getFile())
          info.filePath = DF->getDirectory().str() + "/" + DF->getFilename().str();
        info.line = SP->getLine();
        // Try to get column info from the first instruction's debug location.
        if (!F.empty()) {
          Instruction &firstInst = *F.getEntryBlock().begin();
          if (DILocation *loc = firstInst.getDebugLoc())
            info.column = loc->getColumn();
        }
      }
      funcInfos[&F] = info;
    }

    // Collect direct call edges.
    std::set<std::pair<std::string, std::string>> edges;
    for (Function &F : M) {
      if (F.isDeclaration())
        continue;
      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          if (CallInst *CI = dyn_cast<CallInst>(&I)) {
            if (Function *callee = CI->getCalledFunction()) {
              if (!callee->isDeclaration())
                edges.insert({F.getName().str(), callee->getName().str()});
            }
          }
        }
      }
    }

    // Open the output file.
    std::error_code EC;
    raw_fd_ostream out(OutputFilename, EC, sys::fs::OF_Text);
    if (EC) {
      errs() << "Error opening file " << OutputFilename << ": " << EC.message() << "\n";
      return PreservedAnalyses::none();
    }

    // Write JSON output.
    out << "{\n";
    out << "  \"functions\": [\n";
    bool firstFunc = true;
    for (auto &pair : funcInfos) {
      if (!firstFunc)
        out << ",\n";
      firstFunc = false;
      const FuncInfo &info = pair.second;
      out << "    {";
      out << "\"name\": \"" << info.name << "\", ";
      out << "\"file\": \"" << info.filePath << "\", ";
      out << "\"line\": " << info.line << ", ";
      out << "\"column\": " << info.column;
      out << "}";
    }
    out << "\n  ],\n";
    out << "  \"calls\": [\n";
    bool firstEdge = true;
    for (auto &edge : edges) {
      if (!firstEdge)
        out << ",\n";
      firstEdge = false;
      out << "    {";
      out << "\"caller\": \"" << edge.first << "\", ";
      out << "\"callee\": \"" << edge.second << "\"";
      out << "}";
    }
    out << "\n  ]\n";
    out << "}\n";
    return PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "cg-json", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "cg-json") {
                    MPM.addPass(CallGraphJSONPass());
                    return true;
                  }
                  return false;
                });
          }};
}
