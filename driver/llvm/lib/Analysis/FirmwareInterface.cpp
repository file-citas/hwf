#include "llvm/Pass.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeFinder.h"

using namespace llvm;

namespace {

class SharedMemoryWriteResults {
  SmallSet<Instruction *, 8> Instrs;
public:
  void insert(Instruction* I) {
    Instrs.insert(I);
  }
  SmallSet<Instruction *, 8> &getInstructions() {
    return Instrs;
  }
};

class SharedMemoryWrite : public FunctionPass {
  SharedMemoryWriteResults SMemWrite;

public:
  static char ID;
  SharedMemoryWrite() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  SharedMemoryWriteResults &getSMemWrite() { return SMemWrite; }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char SharedMemoryWrite::ID = 0;
static RegisterPass<SharedMemoryWrite> S("smem-write", "Shared Memory Write Analysis Pass",
                                         false /* Only looks at CFG */,
                                         true /* Analysis Pass */);

bool SharedMemoryWrite::runOnFunction(Function &F) {
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (isa<StoreInst>(&I) || isa<MemTransferInst>(&I)) {
        // TODO: Hard-code for now
        DILocation *DL = dyn_cast_or_null<DILocation>(I.getDebugLoc().getAsMDNode());
        if (DL) {
          unsigned Line = DL->getLine();
          StringRef File = DL->getFilename();
          if (File.equals("smem-write.c") && Line == 12) {
            SMemWrite.insert(&I);
            errs() << "[INFO] write to smem at " << File << ":" << Line << "\n";
          }
          else if (File.endswith("venus_hfi.c") && (Line == 372 || Line == 375 || Line == 377)) {
            SMemWrite.insert(&I);
            errs() << "[INFO] write to smem at " << File << ":" << Line << "\n";
          }
        }
      }
    }
  }
  return false;
}


class ReachingDefinitionLocalResults {
  SmallSet<unsigned long, 8> TaintedArguments;
public:
  void setTainted(unsigned long Idx) {
    TaintedArguments.insert(Idx);
  }
  bool isTainted(unsigned long Idx) const {
    return (TaintedArguments.count(Idx) > 0);
  }
};

class ReachingDefinition : public FunctionPass {
  DenseMap<Function *, ReachingDefinitionLocalResults> RDef;

public:
  static char ID;
  ReachingDefinition() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  ReachingDefinitionLocalResults getDef(Function *F) const { return RDef.lookup(F); }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MemoryDependenceWrapperPass>();
    AU.addRequired<SharedMemoryWrite>();
    AU.setPreservesAll();
  }
};

char ReachingDefinition::ID = 0;
static RegisterPass<ReachingDefinition> X("reaching-def", "",
                                          false /* Only looks at CFG */,
                                          true /* Analysis Pass */);

class ReachingDefinitionGlobal : public ModulePass {
public:
  static char ID;
  ReachingDefinitionGlobal() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ReachingDefinition>();
    AU.setPreservesAll();
  }
};

char ReachingDefinitionGlobal::ID = 0;
static RegisterPass<ReachingDefinitionGlobal> Y("reaching-def-global", "",
                                                false /* Only looks at CFG */,
                                                true /* Analysis Pass */);


Value *getDefinition(MemoryDependenceResults &MD, Value* V) {
  bool StopSearch = false;
  while (!StopSearch) {
    // Look through casts
    if (CastInst *CI = dyn_cast<CastInst>(V)) {
      V = CI->getOperand(0);
      continue;
    }
    // Look through store/load pairs
    else if (LoadInst *LI = dyn_cast<LoadInst>(V)) {
      MemDepResult Res = MD.getDependency(LI);

      // If it is defined in another block, try harder.
      if (Res.isNonLocal()) {
        SmallVector<NonLocalDepResult, 4> Deps;
        MD.getNonLocalPointerDependency(LI, Deps);
        if (Deps.size() == 1) {
          Res = Deps[0].getResult();
        }
        else if (Deps.size() > 1) {
          errs() << "[WARNING] Deps.size() > 1\n";
          Res = Deps[0].getResult();
        }
      }

      Instruction *I = Res.getInst();
      if (I != nullptr) {
        if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
          V = SI->getValueOperand();
          continue;
        }
        else if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
          V = LI;
          continue;
        }
        else {
          errs() << "[WARNING] Instruction not handled: ";
          I->dump();
        }
      }
    }
    StopSearch = true;
  }
  return V;
}

bool ReachingDefinition::runOnFunction(Function &F) {
  Module *M = F.getParent();
  TypeFinder TF;
  TF.run(*M, true /* onlyNamed */);

  /*
  SmallSet<StructType *, 16> SharedTypes;
  for (StructType* STy : TF) {
    if (STy->getName().startswith("struct.hfi_cmd")) {
      SharedTypes.insert(STy);
    }
    STy->dump();
  }
  */

  SharedMemoryWriteResults &SMW = getAnalysis<SharedMemoryWrite>().getSMemWrite();
  for (Instruction *I : SMW.getInstructions()) {
    if (I->getFunction() != &F) continue;

    Value *V = nullptr;
    if (MemTransferInst *MTI = dyn_cast<MemTransferInst>(I)) {
      V = MTI->getRawSource();
    }

    if (V != nullptr) {
      MemoryDependenceResults &MD = getAnalysis<MemoryDependenceWrapperPass>().getMemDep();
      if (Value *Def = getDefinition(MD, V)) {
        if (isa<Argument>(Def)) {
          unsigned long i = 0;
          for (auto B = F.arg_begin(), E = F.arg_end(); B != E; ++B, ++i) {
            if (&(*B) == Def) {
              RDef[&F].setTainted(i);
              errs() << "[INFO] " << F.getName() << " is marked tainted.\n";
            }
          }
        }
      }
    }
  }

  return false;
}

bool ReachingDefinitionGlobal::runOnModule(Module &M) {
  // TODO
  for (Function &F : M) {
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (CallInst *CI = dyn_cast<CallInst>(&I)) {
          Function *Callee = CI->getCalledFunction();
          if (Callee && !Callee->isDeclaration()) {
            ReachingDefinitionLocalResults RD = getAnalysis<ReachingDefinition>(*Callee).getDef(Callee);

            for (unsigned i=0; i<Callee->arg_size(); ++i) {
              if (RD.isTainted(i)) {
                errs() << "[INFO] arg " << i << " of " << Callee->getName() << " is tainted!!\n";
              }
            }
          }
        }
      }
    }
  }
  return false;
}

} //end of anonymous namespace
