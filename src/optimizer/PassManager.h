//
// Created by Alex on 2022/3/10.
//

#ifndef DRAGONIR_PASSMANAGER_H
#define DRAGONIR_PASSMANAGER_H
#include <memory>
#include <vector>
#include <unordered_map>

class Module;
class Function;
class BasicBlock;

class Pass {
public:
    virtual ~Pass() = default;
    virtual void run(Module *module) = 0;
};

class FunctionPass : public Pass {
public:
    virtual void runOnFunction(Function &function) = 0;
    virtual void initialize(Module *module) {}
    virtual void finalize(Module *module) {}
    void run(Module *module) override;
};

class BasicBlockPass : public FunctionPass {
public:
    void runOnFunction(Function &function) override;
    virtual void runOnBasicBlock(BasicBlock *basicBlock) = 0;
};

class PassManager {
    std::vector<std::unique_ptr<Pass>> passes;
public:
    PassManager() = default;

    void addPass(Pass *pass) {
        passes.emplace_back(pass);
    }

    void run(Module *module) {
        for (auto &Pass : passes) {
            Pass->run(module);
        }
    }

};


#endif //DRAGONIR_PASSMANAGER_H
