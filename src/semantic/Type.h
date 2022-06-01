//
// Created by Alex on 2021/9/4.
//

#ifndef DRAGONCOMPILER_TYPE_H
#define DRAGONCOMPILER_TYPE_H

#include <string>
#include <map>
#include <utility>
#include <vector>
#include "Value.h"
class Context;

enum TypeID {
    TypeVoid,
    TypeInt,
    TypeFloat,
    TypeDouble,
    TypeLabel,
    TypeString,
    TypePointer,
    TypeArray,
    TypeFunction,
    TypeStruct,
    TypeUnion,
    TypeEnum,
    TypeClass,
    TypeError
};
enum MachineType {
    MachineVoid,
    MachineI1,
    MachineI8,
    MachineI16,
    MachineI32,
    MachineI64,
    MachineF32,
    MachineF64,
    MachinePtr,
};
class Type {
protected:
    Context *context = nullptr;
    TypeID typeId = TypeVoid;
    Type **containedType = nullptr;
    unsigned containedTypeCount = 0;
public:
    static Type *getMaxType(Type *type1, Type *type2) {
        ASSERT(type1 && type2);
        ASSERT(type1->getTypeId() == type2->getTypeId());
        TypeID TypeId = type1->getTypeId();
        switch (TypeId) {
            case TypeInt:
                return type1->getBitSize() > type2->getBitSize() ? type1 : type2;
            default:
                return type1;
        }
    }
public:
    Type(TypeID typeId, Type **containedType, unsigned size) : context((*containedType)->getContext()), typeId(typeId),
                                                              containedType(containedType), containedTypeCount(size) {}
    Type(Context *ctx, TypeID typeId, Type **containedType) : context(ctx), typeId(typeId),
                                                              containedType(containedType), containedTypeCount(1) {}
    Type(Context *ctx, TypeID typeId) : context(ctx), typeId(typeId) {}

    virtual ~Type() = default;

    inline Context *getContext() const {
        return context;
    }

    inline TypeID getTypeId() const {
        return typeId;
    }

    inline unsigned getBitSize();

    inline bool isVoidType() const {
        return typeId == TypeVoid;
    }

    inline bool isIntegerType() const {
        return typeId == TypeInt;
    }

    inline bool isFloatType() const {
        return typeId == TypeFloat || typeId == TypeDouble;
    }

    inline bool isNumericType() const {
        return isIntegerType() || isFloatType();
    }

    inline bool isLabelType() const {
        return typeId == TypeLabel;
    }

    inline bool isPointerType() const {
        return typeId == TypePointer;
    }

    inline bool isArrayType() const {
        return typeId == TypeArray;
    }

    inline bool isFunctionType() const {
        return typeId == TypeFunction;
    }

    inline bool isStructType() const {
        return typeId == TypeStruct;
    }

    inline bool isUnionType() const {
        return typeId == TypeUnion;
    }

    Type *getPointerType();

    Type *getPointerElementType() const {
        ASSERT(typeId == TypePointer && containedType);
        return *containedType;
    }

    Type *getStructElementType(unsigned index) const {
        ASSERT(index < containedTypeCount);
        ASSERT(typeId == TypeStruct);
        return containedType[index];
    }

    Type *getParameterType(unsigned index) const {
        ASSERT(typeId == TypeFunction);
        ASSERT((index + 1) < containedTypeCount);
        return containedType[index + 1];
    }

    Type *getReturnType() const {
        ASSERT(typeId == TypeFunction);
        return containedType[0];
    }

    unsigned getContainedTypeCount() const {
        return containedTypeCount;
    }

    auto subtypes() {
        return iter(containedType, containedType + containedTypeCount);
    }

    auto params() {
        return iter(containedType + 1, containedType + containedTypeCount);
    }

    MachineType getMachineType() const {
        return MachineI1;
    }

    virtual void dump(std::ostream &os) {
        // dump type id
        switch (getTypeId()) {
            case TypeVoid:
                os << "void";
                break;
            case TypeInt:
                os << "int";
                break;
            case TypeFloat:
                os << "float";
                break;
            case TypeDouble:
                os << "double";
                break;
            case TypeLabel:
                os << "label";
                break;
            case TypeString:
                os << "string";
                break;
            case TypePointer:
                os << "pointer";
                break;
            case TypeArray:
                os << "array";
                break;
            case TypeFunction:
                os << "function";
                break;
            case TypeStruct:
                os << "struct";
                break;
            case TypeUnion:
                os << "union";
                break;
            case TypeEnum:
                os << "enum";
                break;
            case TypeClass:
                os << "class";
                break;
            case TypeError:
                os << "error";
                break;
            default:
                ASSERT(false);
        }

    }

};

class IntegerType : public Type {
    unsigned bitSize;
public:
    IntegerType(Context *ctx, unsigned bitSize) : Type(ctx, TypeInt), bitSize(bitSize) {}

    inline unsigned getBitSize() const {
        return bitSize;
    }

    void dump(std::ostream &os) override {
        os << "i" << bitSize;
    }

};

class FunctionType : public Type {
    std::vector<Type *> types;
    bool isVarArg = false;
public:
    FunctionType(std::vector<Type *> types, bool isVarArg = false) : Type(TypeFunction, types.data(), types.size()),
                                                                     types(std::move(types)), isVarArg(isVarArg) {
    }

    void dump(std::ostream &os) override {
        ASSERT(types.size() > 0);

        os << "(";
        for (auto I = 1; I < types.size(); I++) {
            if (I > 1) {
                os << ", ";
            }
            types[I]->dump(os);
        }

        if (isVarArg) {
            if (types.size() > 0) {
                os << ", ";
            }
            os << "...";
        }
        os << ") -> ";
        getReturnType()->dump(os);

    }
};

class ArrayType : public Type {
public:
    Type *elementType;
    unsigned size;
    ArrayType(Type *original, unsigned size) : Type(original->getContext(), TypeArray, &elementType), elementType(original), size(size) {}

    void dump(std::ostream &os) override {
        os << "[" << size << " x ";
        elementType->dump(os);
        os << "]";
    }

};

class PointerType : public Type {
    Type *elementType;
public:
    PointerType(Type *original) : Type(original->getContext(), TypePointer, &elementType), elementType(original) {}

    void dump(std::ostream &os) override {
        ASSERT(elementType);
        elementType->dump(os);
        os << "*";
    }
};

class StructType : public Type {
    std::vector<Type *> elementTypes;
public:
    StructType(std::vector<Type *> elementTypes) : Type(TypeStruct, elementTypes.data(), elementTypes.size()),
                                                   elementTypes(elementTypes) {}

    StructType(Context *ctx) : Type(ctx, TypeStruct) {}

    void dump(std::ostream &os) override {
        Type::dump(os);
        os << "{";
        for (unsigned I = 0; I < elementTypes.size(); I++) {
            if (I != 0) {
                os << ", ";
            }
            elementTypes[I]->dump(os);
        }
        os << "}";
    }


};

inline unsigned Type::getBitSize() {
    switch (getTypeId()) {
        case TypeVoid:
            return 0;
        case TypeInt:
            return static_cast<IntegerType *>(this)->getBitSize();
        default:
            break;
    }
    UNREACHEABLE();
    return 0;
}

#endif //DRAGONCOMPILER_TYPE_H
