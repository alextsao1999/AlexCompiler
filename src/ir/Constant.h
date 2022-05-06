//
// Created by Alex on 2022/3/30.
//

#ifndef DRAGON_CONSTANT_H
#define DRAGON_CONSTANT_H

#include "Value.h"
#include "Type.h"
/**
 * Undef is the undefined value.
 * We don't use nullptr to avoid confusion.
 */
class Undef : public Value {
public:
    Undef() {
        incRef();
    }
    void dumpAsOperand(std::ostream &os) override {
        os << "undef";
    }
};

/**
 * Param is the parameter value.
 */
class Param : public Value {
    std::string name;
    Type *type;
public:
    Param() : type(nullptr) {}
    Param(StrView name, Type *type) : name(name), type(type) {
        incRef();
    }

    std::string &getName() {
        return name;
    }

    Type *getType() override {
        return type;
    }

    void dumpAsOperand(std::ostream &os) override {
        type->dump(os);
        os << " %" << name;
    }

};

class Global : public Value {
    std::string name;
    Type *type;
public:
    Global(const std::string &name, Type *type) : name(name), type(type) {}

    const std::string &getName() const {
        return name;
    }

    Type *getType() const {
        return type;
    }

    void dumpAsOperand(std::ostream &os) override {
        os << "%" << name;
    }

};

/**
 * Constant is the base class of all constant values.
 */
class Constant : public Value {
protected:
    Type *type;
public:
    Constant(Type *type) : type(type) {
        incRef();
    }

    Type *getType() override {
        return type;
    }

};

template<typename Ty>
class ConstantVal : public Constant {
public:
    ConstantVal(Type *type, const Ty &val) : Constant(type), val(val) {
    }

    Ty &getVal() {
        return val;
    }

    const Ty &getVal() const {
        return val;
    }

    void dumpAsOperand(std::ostream &os) override {
        type->dump(os);
        os << " " << val;
    }

private:
    Ty val;
};
using IntConstant = ConstantVal<int64_t>;
using StrConstant = ConstantVal<std::string>;
using BoolConstant = ConstantVal<bool>;

#endif //DRAGON_CONSTANT_H
