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

class Environment {
public:

};

class Type : public Value {
protected:
    Environment *env = nullptr;
    std::string name;
public:
    Type(Environment *env, const std::string &name) : env(env), name(name) {}
    virtual ~Type() = default;

    virtual unsigned getSize() = 0;

    std::string &getName() {
        return name;
    }

    Environment *getEnv() {
        return env;
    }

    const Environment *getEnv() const {
        return env;
    }

    void dump(std::ostream &os, int level) override {
        Value::dump(os, level);
    }

    void dumpAsOperand(std::ostream &os) override {
        os << name;
    }

};

class PrimitiveType : public Type {
    unsigned size;
public:
    PrimitiveType(Environment *env, const std::string &name, unsigned int size) : Type(env, name), size(size) {}
    ~PrimitiveType() override = default;

    unsigned getSize() override {
        return size;
    }



};

class StructureType : public Type {
public:
    std::vector<std::pair<std::string, Type *>> fields;
    StructureType(Environment *env, const std::string &name) : Type(env, name) {}
    ~StructureType() override {}

    void addField(const std::string &name, Type *type) {
        fields.emplace_back(name, type);
    }
    Type *lookup(const std::string &name) {
        for (auto &item : fields) {
            if (item.first == name) {
                return item.second;
            }
        }
        return nullptr;
    }
    unsigned getSize() override {
        unsigned size = 0;
        for (auto &item : fields) {
            size += item.second->getSize();
        }
        return size;
    }

};

class PointerType : public Type {
public:
    Type *original = nullptr;

    PointerType(Type *original) : Type(original->getEnv(), original->getName() + "*"), original(original) {

    }
};

#endif //DRAGONCOMPILER_TYPE_H
