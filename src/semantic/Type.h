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

class Type {
protected:
    Context *context = nullptr;
    TypeID typeId = TypeVoid;
    Type **containedType = nullptr;
    unsigned containedTypeCount = 0;

public:
    Type(Context *ctx, TypeID typeId, Type **containedType) : context(ctx), typeId(typeId),
                                                              containedType(containedType), containedTypeCount(1) {}

    Type(TypeID typeId, Type **containedType, unsigned size) : context((*containedType)->getContext()), typeId(typeId),
                                                              containedType(containedType), containedTypeCount(size) {}

    Type(Context *ctx, TypeID typeId) : context(ctx), typeId(typeId) {}

    virtual ~Type() = default;

    inline Context *getContext() const {
        return context;
    }

    inline TypeID getTypeId() const {
        return typeId;
    }

    inline unsigned getBitSize();

    inline bool isPointerType() const {
        return typeId == TypePointer;
    }

    Type *getPointerType();

    Type *getPointerElementType() {
        assert(typeId == TypePointer && containedType);
        return *containedType;
    }

    Type *getStructElementType(unsigned index) {
        assert(index < containedTypeCount);
        assert(typeId == TypeStruct);
        return containedType[index];
    }

    Type *getParameterType(unsigned index) {
        assert(typeId == TypeFunction);
        assert((index + 1) < containedTypeCount);
        return containedType[index + 1];
    }

    Type *getReturnType() {
        assert(typeId == TypeFunction);
        return containedType[0];
    }

    unsigned getContainedTypeCount() {
        return containedTypeCount;
    }

    auto subtypes() {
        return iter(containedType, containedType + containedTypeCount);
    }

    auto params() {
        return iter(containedType + 1, containedType + containedTypeCount);
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
                assert(false);
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
        assert(types.size() > 0);

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

class PointerType : public Type {
    Type *elementType;
public:
    PointerType(Type *original) : Type(original->getContext(), TypePointer, &elementType), elementType(original) {}

    void dump(std::ostream &os) override {
        assert(elementType);
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



#endif //DRAGONCOMPILER_TYPE_H
