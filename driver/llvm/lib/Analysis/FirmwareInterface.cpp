#include "llvm/Pass.h"
#include "llvm/ADT/SmallSet.h"
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
            errs() << File << ":" << Line << " (OK)\n";
          }
          else if (File.equals("venus_hfi.c")) {
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
  ReachingDefinitionLocalResults RDef;

public:
  static char ID;
  ReachingDefinition() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  ReachingDefinitionLocalResults &getDef() { return RDef; }

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
      if (Res.isDef()) {
        errs() << "DEF:";
        Res.getInst()->dump();
        if (StoreInst *SI = dyn_cast<StoreInst>(Res.getInst())) {
          V = SI->getValueOperand();
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

  MemoryDependenceResults &MD = getAnalysis<MemoryDependenceWrapperPass>().getMemDep();
  SharedMemoryWriteResults &SMW = getAnalysis<SharedMemoryWrite>().getSMemWrite();
  for (Instruction *I : SMW.getInstructions()) {
    Value *V = nullptr;
    if (MemTransferInst *MTI = dyn_cast<MemTransferInst>(I)) {
      V = MTI->getRawSource();
    }
    else if (StoreInst* SI = dyn_cast<StoreInst>(I)) {
      V = SI->getValueOperand();
    }

    if (V != nullptr) {
      if (Value *Def = getDefinition(MD, V)) {
        if (isa<Argument>(Def)) {
          unsigned long i = 0;
          for (auto B = F.arg_begin(), E = F.arg_end(); B != E; ++B, ++i) {
            if (&(*B) == Def) {
              RDef.setTainted(i);
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
          if (!Callee->isDeclaration()) {
            ReachingDefinitionLocalResults &RD = getAnalysis<ReachingDefinition>(*Callee).getDef();

            for (unsigned i=0; i<Callee->arg_size(); ++i) {
              if (RD.isTainted(i)) {
                errs() << "arg " << i << " of " << Callee->getName() << " is tainted!!\n";
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
