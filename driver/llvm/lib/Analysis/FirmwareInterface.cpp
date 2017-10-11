#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct FirmwareInterface : public FunctionPass {
  static char ID;
  FirmwareInterface() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "Firmware Interface: ";
    errs().write_escaped(F.getName()) << '\n';
    return false;
  }
};
} //end of anonymous namespace

char FirmwareInterface::ID = 0;
static RegisterPass<FirmwareInterface> X("Firmware Interface", "Firmware Interface Pass",
                                         false /* Only looks at CFG */,
                                         false /* Analysis Pass */);
