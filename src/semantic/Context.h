//
// Created by Alex on 2022/3/10.
//

#ifndef DRAGON_CONTEXT_H
#define DRAGON_CONTEXT_H

#include <unordered_map>
#include "Node.h"
#include "Type.h"
#include "Constant.h"

class Context {
    Undef undefInstance;

    Type voidTy{this, TypeVoid}, floatTy{this, TypeFloat}, doubleTy{this, TypeDouble},
            stringTy{this, TypeString}, lableTy{this, TypeLabel};
    IntegerType i1Ty{this, 1}, i8Ty{this, 8}, i16Ty{this, 16}, i32Ty{this, 32}, i64Ty{this, 64}, i128Ty{this, 128};

    std::map<Type *, std::string> typeNames;
    std::map<int64_t, std::unique_ptr<IntConstant>> intSlots;
    std::map<std::pair<std::vector<Type *>, bool>, std::unique_ptr<FunctionType>> functionTypes;
    std::map<Type *, std::unique_ptr<PointerType>> pointerTypes;

public:
    Context() = default;
    ~Context() {

    }

    Type *getVoidTy() {
        return &voidTy;
    }

    Type *getFloatTy() {
        return &floatTy;
    }

    Type *getDoubleTy() {
        return &doubleTy;
    }

    Type *getLableTy() {
        return &lableTy;
    }

    IntegerType *getInt1Ty() {
        return &i1Ty;
    }

    IntegerType *getInt8Ty() {
        return &i8Ty;
    }

    IntegerType *getIInt16Ty() {
        return &i16Ty;
    }

    IntegerType *getInt32Ty() {
        return &i32Ty;
    }

    PointerType *getPointerTy(Type *ty) {
        auto &PtrTy = pointerTypes[ty];
        if (!PtrTy)
            PtrTy = std::make_unique<PointerType>(ty);
        return PtrTy.get();
    }

    FunctionType *getFunctionTy(std::vector<Type *> &types, bool isVarArg = false) {
        auto &FT = functionTypes[std::make_pair(types, isVarArg)];
        if (FT)
            return FT.get();
        auto *Ty = new FunctionType(types, isVarArg);
        FT = std::unique_ptr<FunctionType>(Ty);
        return Ty;
    }

    FunctionType *getFunctionTy(Type *returnTy, std::initializer_list<Type *> types = {}, bool isVarArg = false) {
        std::vector<Type *> Vec{returnTy};
        Vec.insert(Vec.end(), types.begin(), types.end());
        return getFunctionTy(Vec, isVarArg);
    }

    inline FunctionType *getVoidFunTy() {
        return getFunctionTy(getVoidTy());
    }

    inline Undef *getUndef() {
        return &undefInstance;
    }

    inline Constant *getInt(int64_t value) {
        auto It = intSlots.find(value);
        if (It != intSlots.end()) {
            return It->second.get();
        }
        intSlots[value] = std::make_unique<IntConstant>(getInt32Ty(), value);
        return intSlots[value].get();
    }

private:

};

#endif //DRAGON_CONTEXT_H
