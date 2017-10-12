#include "llvm/Pass.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeFinder.h"

using namespace llvm;

namespace {

class SharedMemoryWrite : public FunctionPass {
public:
  static char ID;
  SharedMemoryWrite() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MemoryDependenceWrapperPass>();
    AU.setPreservesAll();
  }
};

StructType *getAllocatedType(MemoryDependenceResults &MD, Value* V) {
  Type *Ty = V->getType();

  bool StopSearch = false;
  while (!StopSearch) {
    // Look through casts
    if (CastInst *CI = dyn_cast<CastInst>(V)) {
      V = CI->getOperand(0);
      Ty = V->getType();
      continue;
    }
    // Look through store/load pairs
    else if (LoadInst *LI = dyn_cast<LoadInst>(V)) {
      MemDepResult Res = MD.getDependency(LI);
      if (Res.isDef()) {
        errs() << "DEF:";
        Res.getInst()->dump();
        if (StoreInst *SI = dyn_cast<StoreInst>(Res.getInst())) {
          V = SI->getValueOperand();
          Ty = V->getType();
          continue;
        }
      }
    }
    // TODO: look through callers
    else if (Argument* Arg = dyn_cast<Argument>(V)) {
      Arg->getParent()->dump();
    }
    StopSearch = true;
  }
  return dyn_cast<StructType>(Ty);
}

bool SharedMemoryWrite::runOnFunction(Function &F) {
  Module *M = F.getParent();
  TypeFinder TF;
  TF.run(*M, true /* onlyNamed */);

  SmallSet<StructType *, 16> SharedTypes;
  for (StructType* STy : TF) {
    if (STy->getName().startswith("struct.hfi_cmd")) {
      SharedTypes.insert(STy);
    }
    STy->dump();
  }

  MemoryDependenceResults &MD = getAnalysis<MemoryDependenceWrapperPass>().getMemDep();
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
        if (StructType *STy = getAllocatedType(MD, V)) {
          if (SharedTypes.count(STy) > 0) {
            I.dump();
          }
        }
      }
    }
  }

  return false;
}

} //end of anonymous namespace

char SharedMemoryWrite::ID = 0;
static RegisterPass<SharedMemoryWrite> X("smem-write", "Shared Memory Write Analysis Pass",
                                         false /* Only looks at CFG */,
                                         true /* Analysis Pass */);
