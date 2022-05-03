//
// Created by Alex on 2022/3/13.
//

#ifndef DRAGON_PATTERNNODE_H
#define DRAGON_PATTERNNODE_H

#include <cassert>
#include <new>
#include <vector>
#include "Node.h"
#include "Value.h"
namespace Pattern {
    enum Opcode {
        None,
        Add,
        Sub,
        Mul,
        Div,
        Rem,
        Mod,
        Shl,
        Shr,
        And,
        Or,
        Xor,
        Eq,
        Ne,
        Lt,
        Le,
        Gt,
        Ge,
        Stack,
        Operands,
        Address,
        Call,
        Register,
        Return,
        LAST_OPCODE,
    };
} // namespace Pattern

class PatternUse;
class NodeLoc {
public:
    int id = 0;
    NodeLoc() = default;
    NodeLoc(int id) : id(id) {}

};

/**
 * The node of pattern tree.
 */
class PatternNode : public Node<PatternNode> {
    friend class PatternUse;
protected:
    static PatternNode *allocateWithUses(size_t size, const std::vector<PatternNode *> &nodes);
    // override new
    void *operator new(size_t size, size_t useSize);
public:
    // override delete
    void operator delete(void *p);
    static PatternNode *New(unsigned opcode, unsigned numOperands) {
        auto *Ptr = PatternNode::operator new(sizeof(PatternNode), numOperands);
        return ::new(Ptr) PatternNode(opcode, numOperands);
    }
    template<typename T, typename ...Args>
    static T *New(unsigned useSize, Args&&...args) {
        auto *Ptr = PatternNode::operator new(sizeof(T), useSize);
        return ::new(Ptr) T(std::forward<Args>(args)...);
    }

    template<typename T = PatternNode, typename ...Args>
    static T *createNode(const std::vector<PatternNode *> &nodes, Args &&...args) {
        auto *Ptr = allocateWithUses(sizeof(T), nodes);
        ::new(Ptr) T(std::forward<Args>(args)...);
        Ptr->numOperands = nodes.size();
        return static_cast<T *>(Ptr);
    }
protected:
    PatternUse *users = nullptr;
    unsigned opcode = 0;
    unsigned numOperands = 0;
    Type *type = nullptr;
    NodeLoc loc;
    PatternNode(unsigned opcode, unsigned numOperands) : opcode(opcode), numOperands(numOperands) {}
public:
    using UseIterator = UseIteratorImpl<PatternUse, UseGetter<PatternUse>>;
    using UserIterator = UseIteratorImpl<PatternUse, UserGetter<PatternUse, PatternNode>>;
    PatternNode(unsigned int opcode) : opcode(opcode) {}

    inline unsigned getOpcode() const {
        return opcode;
    }

    inline unsigned getNumOperands() const {
        return numOperands;
    }

    inline PatternUse *getHungoffOperands();
    inline void setHungOffOperand(unsigned index, PatternNode *operand);

    inline void setLoc(NodeLoc l) {
        loc = l;
    }

    Type *getType() const {
        return type;
    }
    void setType(Type *t) {
        type = t;
    }

    inline UseIterator::range getUses() {
        return iter(UseIterator(users), UseIterator());
    }

    inline UserIterator::range getUsers() {
        return iter(UserIterator(users), UserIterator());
    }

    inline UserIterator user_begin() const {
        return UserIterator(users);
    }

    inline UserIterator user_end() const {
        return UserIterator();
    }

    inline UseIterator use_begin() const {
        return UseIterator(users);
    }

    inline UseIterator use_end() const {
        return UseIterator();
    }

    template<typename T, typename UserAsIter = UseIteratorImpl<Use, UserGetter<Use, T>>, typename range = typename UserAsIter::range>
    inline range getUsersAs() {
        return iter(UserAsIter(users), UserAsIter());
    }

    template<typename T>
    inline T *as() { return opcode == T::OPCODE ? static_cast<T *>(this) : nullptr; }
    template<typename T>
    inline bool is() const { return opcode == T::OPCODE; }

};

class PatternUse {
private:
    friend class PatternNode;
    /// The user of this use.
    /// E.g. For add instruction, if it uses two operands,
    /// every use has the same user pointer to the add instruction.
    PatternNode *parent = nullptr;
    /// The used value.
    PatternNode *value = nullptr;
    /// The previous use reference address. So we can remove this use from the list.
    PatternUse **prev = nullptr;
    /// The next pointer of the use list.
    PatternUse *next = nullptr;
public:
    PatternUse() {}
    PatternUse(PatternNode *parent) : parent(parent) {}
    PatternUse(PatternNode *parent, PatternNode *value) : parent(parent) {
        set(value);
    }

    bool empty() const { return !value; }

    void set(PatternNode *v) {
        assert(parent);
        unset();
        value = v;
        if (v) {
            next = v->users;
            if (next) {
                next->prev = &next;
            }
            prev = &v->users;
            v->users = this;
        }
    }

    void unset() {
        assert(parent);
        if (!value)
            return;
        if (next)
            next->prev = prev;
        *prev = next;
        value = nullptr;
    }

    /// Set the user of this use
    void setUser(PatternNode *u) {
        parent = u;
    }

    /// Get the user of this use
    inline PatternNode *getUser() {
        return parent;
    }

    inline const PatternNode *getUser() const {
        return parent;
    }

    inline const PatternNode *getValue() const {
        return value;
    }

    /// Get the used value
    inline PatternNode *getValue() {
        return value;
    }

    inline const PatternUse *getNext() const {
        return next;
    }

    inline PatternUse *getNext() {
        return next;
    }

    inline operator bool() const {
        return value;
    }

    inline PatternNode *operator->() {
        return value;
    }

    inline const PatternNode *operator->() const {
        return value;
    }

};

inline PatternUse *PatternNode::getHungoffOperands() {
    return reinterpret_cast<PatternUse *>(this) - numOperands;
}

void PatternNode::setHungOffOperand(unsigned int index, PatternNode *operand) {
    assert(index < numOperands);
    getHungoffOperands()[index].set(operand);
}

template<unsigned Opcode, unsigned OpNum>
class PatternNodeBase : public PatternNode {
public:
    constexpr static unsigned OPCODE = Opcode;
    void *operator new(size_t S) { return PatternNode::operator new(S, OpNum); }
    void operator delete(void *Ptr) { return PatternNode::operator delete(Ptr); }
public:
    PatternNodeBase() : PatternNode(Opcode, OpNum) {}

    PatternNodeBase(std::initializer_list<PatternNode *> nodes) : PatternNode(Opcode, OpNum) {
        assert(nodes.size() == OpNum);
        unsigned I = 0;
        for (auto *Node : nodes) {
            setHungOffOperand(I++, Node);
        }
    }
};

class AddNode : public PatternNodeBase<Pattern::Add, 2> {
public:
    AddNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class SubNode : public PatternNodeBase<Pattern::Sub, 2> {
public:
    SubNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class MulNode : public PatternNodeBase<Pattern::Mul, 2> {
public:
    MulNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class DivNode : public PatternNodeBase<Pattern::Div, 2> {
public:
    DivNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class ModNode : public PatternNodeBase<Pattern::Mod, 2> {
public:
    ModNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class ShlNode : public PatternNodeBase<Pattern::Shl, 2> {
public:
    ShlNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class ShrNode : public PatternNodeBase<Pattern::Shr, 2> {
public:
    ShrNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class AndNode : public PatternNodeBase<Pattern::And, 2> {
public:
    AndNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class OrNode : public PatternNodeBase<Pattern::Or, 2> {
public:
    OrNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class XorNode : public PatternNodeBase<Pattern::Xor, 2> {
public:
    XorNode(PatternNode *lhs, PatternNode *rhs) : PatternNodeBase({lhs, rhs}) {}
};

class Register {
    unsigned regId;
public:
    Register() {}
    Register(unsigned int regId) : regId(regId) {}
    unsigned getRegId() const { return regId; }
};
class RegisterNode : public PatternNodeBase<Pattern::Register, 0> {
    Register reg;
public:
    RegisterNode(const Register &reg) : reg(reg) {}
    inline Register getRegister() const {
        return reg;
    }
};

class ReturnNode : public PatternNodeBase<Pattern::Return, 1> {
public:
    ReturnNode(PatternNode *value) : PatternNodeBase({value}) {}
};

class OperandList : public PatternNode {
public:
    constexpr static unsigned OPCODE = Pattern::Operands;
    OperandList() : PatternNode(OPCODE) {}

    bool hasOnlyOneUser() const {
        return users && users->getNext() == nullptr;
    }

    PatternNode *getOperand(size_t index) {
        return getHungoffOperands()[index].getValue();
    }

    void setOperand(size_t index, PatternNode *node) {
        getHungoffOperands()[index].set(node);
    }
};

class Address : public PatternNode {
    Value *value;
public:
    constexpr static unsigned OPCODE = Pattern::Address;
    void *operator new(size_t S) { return PatternNode::operator new(S, 0); }
    void operator delete(void *Ptr) { return PatternNode::operator delete(Ptr); }
public:
    Address(Value *value) : PatternNode(OPCODE), value(value) {}

    Value *getValue() const {
        return value;
    }

    void setValue(Value *v) {
        this->value = v;
    }
};

class CallNode : public PatternNode {
public:
    constexpr static unsigned OPCODE = Pattern::Call;
    CallNode() : PatternNode(OPCODE) {}

};

#endif //DRAGON_PATTERNNODE_H
