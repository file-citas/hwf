#include "llvm/Pass.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeFinder.h"

using namespace llvm;

namespace {
struct SharedMemoryWrite : public ModulePass {
  static char ID;
  SharedMemoryWrite() : ModulePass(ID) {}

  // TODO: Inter-procedural
  StructType *getAllocatedType(Value* V) {
    Type *Ty = V->getType();
    while (CastInst *CI = dyn_cast<CastInst>(V)) {
      V = CI->getOperand(0);
      Ty = V->getType();
    }
    return dyn_cast<StructType>(Ty);
  }

  bool runOnModule(Module &M) override {
    TypeFinder TF;
    TF.run(M, true /* onlyNamed */);

    SmallSet<StructType *, 16> SharedTypes;
    for (StructType* STy : TF) {
      if (STy->getName().startswith("struct.hfi_cmd")) {
        SharedTypes.insert(STy);
      }
      STy->dump();
    }

    for (Function &F: M) {
      for (BasicBlock &B: F) {
        for (Instruction &I : B) {
          Value *V = nullptr;
          if (MemTransferInst *MTI = dyn_cast<MemTransferInst>(&I)) {
            V = MTI->getRawSource();
          }
          else if (StoreInst* SI = dyn_cast<StoreInst>(&I)) {
            V = SI->getValueOperand();
          }

          if (V != nullptr) {
            if (StructType *STy = getAllocatedType(V)) {
              if (SharedTypes.count(STy) > 0) {
                I.dump();
              }
            }
          }
        }
      }
    }
    return false;
  }
};
} //end of anonymous namespace

char SharedMemoryWrite::ID = 0;
static RegisterPass<SharedMemoryWrite> X("smem-write", "Shared Memory Write Analysis Pass",
                                         false /* Only looks at CFG */,
                                         true /* Analysis Pass */);
