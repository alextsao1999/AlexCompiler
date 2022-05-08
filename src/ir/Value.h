//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_VALUE_H
#define DRAGONIR_VALUE_H
#include <iostream>
#include <sstream>
#include "Common.h"
#include "Range.h"
#include "Opcode.h"

class Value;
class Type;
class Use;
template<typename UseT, typename AsTy = Value>
struct UseOpWrapper {
    using type = AsTy;
    using value_type = std::remove_pointer_t<typename std::remove_reference_t<AsTy>>;
    inline value_type *operator()(UseT &use) const {
        return (value_type *) use.getValue();
    }
};

template<typename UseT, typename AsTy>
class UseWrapper {
public:
    using value_type = AsTy;
    using reference = value_type *;
    using pointer = value_type *;
    using difference_type = typename std::iterator_traits<UseT *>::difference_type;
    using iterator_category = typename std::random_access_iterator_tag;
private:
    UseT *_iter;
public:
    UseWrapper(UseT * iter) : _iter(iter) {}
    UseWrapper(const UseWrapper &r) : _iter(r._iter) {}
    UseWrapper &operator+=(difference_type n) {
        _iter += n;
        return *this;
    }
    UseWrapper &operator-=(difference_type n) {
        _iter -= n;
        return *this;
    }
    UseWrapper &operator++() {
        ++_iter;
        return *this;
    }
    UseWrapper operator++(int) {
        IterWrapper tmp(*this);
        ++_iter;
        return tmp;
    }
    difference_type operator-(const UseWrapper &r) const {
        return _iter - r._iter;
    }
    bool operator!=(const UseWrapper &r) {
        return _iter != r._iter;
    }
    bool operator==(const UseWrapper &r) {
        return _iter == r._iter;
    }

/*
    pointer operator->() {
        return (AsTy *) (_iter);
    }
*/

    reference operator*() {
        return (AsTy *) (_iter->getValue());
    }

};

template <typename UseT>
struct UseGetter {
    using value_type = UseT;
    inline UseT *operator()(UseT *v) {
        return v;
    }
};

template <typename UseT, typename AsTy = Value>
struct UserGetter {
    using value_type = AsTy;
    inline AsTy *operator()(UseT *v) {
        return (AsTy *) v->getUser();
    }
};

template <typename UseT = Use, typename F = UseGetter<UseT>>
class UseIteratorImpl {
private:
    UseT *use = nullptr;
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename F::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    using range = IterRange<UseIteratorImpl>;
    UseIteratorImpl() {}
    UseIteratorImpl(UseT *use) : use(use) {}
    UseIteratorImpl(const UseIteratorImpl &other) : use(other.use) {}
    bool operator==(const UseIteratorImpl &rhs) const {
        return use == rhs.use;
    }
    bool operator!=(const UseIteratorImpl &rhs) const {
        return !operator==(rhs);
    }
    bool atEnd() const {
        return use == nullptr;
    }
    Use &getUse() const { return *use; }
    pointer operator->() const { return F()(use); }
    reference operator*() const { return *operator->(); }
    UseIteratorImpl &operator++() {
        use = use->getNext();
        return *this;
    }
    UseIteratorImpl operator++(int) {
        UseIteratorImpl tmp(*this);
        operator++();
        return tmp;
    }
};

class Value {
    template<class Ty>
    friend class ListRefTrait;
    friend class Use;
public:
    using UseIterator = UseIteratorImpl<Use, UseGetter<Use>>;
    using UserIterator = UseIteratorImpl<Use, UserGetter<Use, Value>>;
protected:
    Use *users = nullptr;
    size_t refCount = 0;
public:
    Value();
    virtual ~Value();
    virtual Type *getType() { return nullptr; }

    /// memory management
    void incRef() {
        refCount++;
    }
    void decRef() {
        if (--refCount == 0) {
            delete this;
        }
    }
    inline size_t getRefCount() {
        return refCount;
    }

    /// Get opcode of instruction
    Opcode getOpcode();
    bool isInstruction() const;
    bool isFunction() const;
    bool isBasicBlock() const;
    bool isConstant() const;
    bool isConstantZero() const;

    bool isOnlyUsedOnce() const;
    bool isNotUsed() const;
    inline Use *getLastUse() const {
        return users;
    }

    void replaceAllUsesWith(Value *newVal);

    // 如果使用迭代器对调用Use.set() 会删除Use其的使用
    inline UseIterator::range getUses() {
        return iter(UseIterator(users), UseIterator());
    }
    inline UseIterator use_begin() {
        return UseIterator(users);
    }
    inline UseIterator use_end() {
        return UseIterator();
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

    inline std::string dumpToString() {
        std::stringstream SS;
        dump(SS);
        // Trim the string
        std::string Str = SS.str();
        Str.erase(Str.find_last_not_of(" \n\r\t") + 1);
        return std::move(Str);
    }
    inline std::string dumpOperandToString() {
        std::stringstream SS;
        dumpAsOperand(SS);
        // Trim the string
        std::string Str = SS.str();
        Str.erase(Str.find_last_not_of(" \n\r\t") + 1);
        return std::move(Str);
    }

    virtual void dump(std::ostream &os) {}
    virtual void dumpAsOperand(std::ostream &os) {}

    template<typename T>
    inline T *as() { return dynamic_cast<T *>(this); }
    template <typename T>
    inline T *cast() { return static_cast<T *>(this); }
    template <typename T>
    inline bool isa() { return dynamic_cast<T *>(this); }

    template<typename T>
    inline const T *as() const { return dynamic_cast<const T *>(this); }
    template <typename T>
    inline const T *cast() const { return static_cast<const T *>(this); }
    template <typename T>
    inline bool isa() const { return dynamic_cast<const T *>(this); }

};

class Use {
    friend class Value;
    /// The user of this use.
    /// E.g. For add instruction, if it uses two operands,
    /// every use has the same user pointer to the add instruction.
    Value *parent = nullptr;
    /// The used value.
    Value *value = nullptr;
    /// The previous use reference address. So we can remove this use from the list.
    Use **prev = nullptr;
    /// The next pointer of the use list.
    Use *next = nullptr;
public:
    Use() {}
    Use(Value *parent) : parent(parent) {}
    Use(Value *parent, Value *value) : parent(parent) {
        set(value);
    }
    Use(const Use &rhs) {
        parent = rhs.parent;
        set(rhs.value);
    }
    ~Use() {
        //unset();
    }

    /// We don't need the = operator for now.
    Use &operator=(const Use &rhs) = delete;
    /*Use &operator=(const Use &rhs) {
        parent = rhs.parent;
        set(rhs.value);
        return *this;
    }*/

    bool empty() const { return !value; }

    void set(Value *v) {
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
            v->incRef();
        }
    }

    void unset() {
        assert(parent);
        if (!value)
            return;
        if (next)
            next->prev = prev;
        *prev = next;
        value->decRef();
        value = nullptr;
    }

    /// Set the user of this use
    void setUser(Value *u) {
        parent = u;
    }

    /// Get the user of this use
    inline Value *getUser() {
        return parent;
    }

    /// Get the used value
    inline Value *getValue() {
        return value;
    }

    inline const Value *getUser() const {
        return parent;
    }

    inline const Value *getValue() const {
        return value;
    }

    inline const Use *getNext() const {
        return next;
    }

    inline Use *getNext() {
        return next;
    }

    inline operator bool() const {
        return value;
    }

    inline Value *operator->() {
        return value;
    }

    inline const Value *operator->() const {
        return value;
    }
};

// Strong reference to Value
class ValueRef {
    Value *value = nullptr;
public:
    ValueRef() {}
    ValueRef(Value *v) : value(v) {
        if (value)
            value->incRef();
    }
    ValueRef(const ValueRef &rhs) : value(rhs.value) {
        if (value)
            value->incRef();
    }
    ValueRef(ValueRef &&rhs) : value(rhs.value) {
        rhs.value = nullptr;
    }
    ~ValueRef() {
        delete value;
    }
    ValueRef &operator=(const ValueRef &rhs) {
        delete value;
        value = rhs.value;
        if (value)
            value->incRef();
        return *this;
    }
    ValueRef &operator=(Value *v) {
        delete value;
        value = v;
        if (value)
            value->incRef();
        return *this;
    }
    ValueRef &operator=(ValueRef &&rhs) {
        delete value;
        value = rhs.value;
        rhs.value = nullptr;
        return *this;
    }
    operator Value *() {
        return value;
    }
    operator const Value *() const {
        return value;
    }
    Value *operator->() {
        return value;
    }
    const Value *operator->() const {
        return value;
    }
    Value &operator*() {
        return *value;
    }
    const Value &operator*() const {
        return *value;
    }
    bool operator==(const ValueRef &rhs) const {
        return value == rhs.value;
    }
    bool operator!=(const ValueRef &rhs) const {
        return value != rhs.value;
    }
    bool operator==(const Value *rhs) const {
        return value == rhs;
    }
    bool operator!=(const Value *rhs) const {
        return value != rhs;
    }

};

template<typename ForwIt, typename F, typename S = std::string>
inline std::ostream &dump_iter(std::ostream &os, ForwIt begin, ForwIt end, F f, S s = ", ") {
    for (auto iter = begin; iter != end; ++iter) {
        if (iter != begin) {
            os << s;
        }
        os << f(*iter);
    }
    return os;
}

template<typename ForwIt, typename F, typename S = std::string>
inline void dump_os(ForwIt begin, ForwIt end, F f, S s = ", ") {
    for (auto iter = begin; iter != end; ) {
        auto cur = iter++;
        if (iter == end) {
            f(*cur);
            break;
        }
        f(*cur) << s;
    }
}

template<typename ForwIt, typename F, typename S = std::string>
inline void dump_os(std::ostream &os, ForwIt begin, ForwIt end, F f, S s = ", ") {
    for (auto iter = begin; iter != end; ) {
        auto cur = iter++;
        if (iter == end) {
            f(*cur);
            break;
        }
        f(*cur);
        os << s;
    }
}

template<typename C, typename F, typename S = std::string>
inline void dump_os(C &c, F f, S s = ", ") {
    dump_os(c.begin(), c.end(), std::move(f), s);
}

template<typename C, typename F, typename S = std::string>
inline std::ostream &dump_os(std::ostream &os, C c, F f, S s = ", ") {
    dump_os(os, c.begin(), c.end(), std::move(f), s);
    return os;
}

struct OperandDumper {
    template<typename T>
    inline std::string operator()(T &v) {
        if (v)
            return v->dumpOperandToString();
        return "null";
    }
    inline std::string operator()(Value *v) {
        return v->dumpOperandToString();
    }
    inline std::string operator()(Value &v) {
        return v.dumpOperandToString();
    }
};

struct ValueDumper {
    inline std::string operator()(Value *v) {
        return v->dumpToString();
    }
    inline std::string operator()(Value &v) {
        return v.dumpToString();
    }
};

template<typename C, typename Pred = OperandDumper>
std::string dump_str(const C &c, Pred fun = Pred(), const std::string &s = ", ") {
    std::string Out;
    for (auto Iter = c.begin() ; Iter != c.end(); ++Iter) {
        if (Iter != c.begin()) {
            Out += s;
        }
        Out += fun(*Iter);
    }
    return Out;
}

#define DUMP_ITER(OS, C, ARG, BODY) dump_os(C, ([&](auto ARG) -> decltype(auto) {{BODY}; return OS;}));
#define DUMP_REF(OS, C, ARG, BODY) dump_os(OS, C, ([&](auto &ARG) -> decltype(auto) {BODY;}))
#define DUMP_PTR(OS, C, ARG, BODY) dump_os(OS, C, ([&](auto *ARG) -> decltype(auto) {BODY;}))
#define DUMP_REF_S(OS, C, S, ARG, BODY) dump_os(OS, C.begin(), C.end(), ([&](auto &ARG) {BODY;}), S)

#endif //DRAGONIR_VALUE_H
