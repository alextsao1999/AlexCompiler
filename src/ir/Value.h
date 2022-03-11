//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_VALUE_H
#define DRAGONIR_VALUE_H
#include <iostream>
#include "Common.h"
#include "Range.h"
#define OPCODE_LIST(S)       \
S(Alloca, 0)                 \
S(Phi,    0)                 \
S(Undef,  0)                 \
S(Br,     1)                 \
S(CondBr, 3)                 \
S(Binary, 2)                 \
S(Ret,    0)                 \
S(Call,   0)                 \
S(Load,   1)                 \
S(Store,  2)

enum Opcode {
#define DEFINE_OPCODE(name, c) Opcode##name,
    OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
};

class Type;
class Use;

class Value {
    template<class Ty>
    friend class ListRefTrait;
    friend class Use;
protected:
    Use *users = nullptr;
    size_t refCount = 0;
public:
    void incRef() {
        refCount++;
    }
    void decRef() {
        refCount--;
        if (refCount == 0) {
            delete this;
        }
    }

    inline size_t getRefCount() {
        return refCount;
    }

    inline Opcode getOpcode();

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
        friend class Use;
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
        bool operator==(const UseIteratorImpl &rhs) {
            return use == rhs.use;
        }
        bool operator!=(const UseIteratorImpl &rhs) {
            return !operator==(rhs);
        }
        Use &getUse() { return *use; }
        const Use &getUse() const { return *use; }
        pointer operator->() { return F()(use); }
        reference operator*() { return *operator->(); }
        UseIteratorImpl &operator++() {
            use = use->next;
            return *this;
        }
        UseIteratorImpl operator++(int) {
            auto back = *this;
            ++*this;
            return back;
        }
    };

    using UseIterator = UseIteratorImpl<Use, UseGetter<Use>>;
    using UserIterator = UseIteratorImpl<Use, UserGetter<Use, Value>>;
    Value();
    virtual ~Value();
    virtual Type *getType() { return nullptr; }

    bool isOnlyUsedOnce();

    inline Use *getLastUse() {
        return users;
    }

    // 如果使用迭代器对调用Use.set() 会删除Use其的使用
    UseIterator::range getUses() {
        return iter(UseIterator(users), UseIterator());
    }

    UserIterator::range getUsers() {
        return iter(UserIterator(users), UserIterator());
    }

    template<typename T, typename UserAsIter = UseIteratorImpl<Use, UserGetter<Use, T>>, typename wrapper = typename UserAsIter::wrapper>
    wrapper getUsersAs() {
        return iter(UserAsIter(users), UserAsIter());
    }

    inline void dumpStd() {
        dump(std::cout, 0);
    }

    virtual void dump(std::ostream &os, int level = 0) {
        std::string Indent(level * 4, ' ');
        os << Indent;
    }
    virtual void dumpAsOperand(std::ostream &os) {}

    template<typename T>
    inline T *as() { return dynamic_cast<T *>(this); }
    template <typename T>
    inline T *cast() { return static_cast<T *>(this); }
    template <typename T>
    inline bool isa() { return dynamic_cast<T *>(this); }

};

class Use {
    friend class Value;
    Value *parent = nullptr;
    Value *value = nullptr;
    Use **prev = nullptr;
    Use *next = nullptr;
public:
    Use() {}
    Use(Value *parent) : parent(parent) {}
    Use(Value *parent, Value *value) : parent(parent) {
        set(value);
    }
/*    Use(const Use &rhs) {
        parent = rhs.parent;
        set(rhs.value);
    }
    Use(Use &&rhs) {
        parent = rhs.parent;
        set(rhs.value);
        rhs.unset();
    }*/
    ~Use() {
        unset();
    }

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
        }
        v->incRef();
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

    void setUser(Value *u) {
        parent = u;
    }

    inline Value *getUser() {
        return parent;
    }

    inline const Value *getUser() const {
        return parent;
    }

    inline const Value *getValue() const {
        return value;
    }

    inline Value *getValue() {
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

    Use &operator=(const Use &rhs) {
        parent = rhs.parent;
        set(rhs.value);
        return *this;
    }

    inline Value *operator->() {
        return value;
    }

    inline const Value *operator->() const {
        return value;
    }
};

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
inline void dump_os(C c, F f, S s = ", ") {
    dump_os(c.begin(), c.end(), std::move(f), s);
}

template<typename C, typename F, typename S = std::string>
inline std::ostream &dump_os(std::ostream &os, C c, F f, S s = ", ") {
    dump_os(os, c.begin(), c.end(), std::move(f), s);
    return os;
}

#define DUMP_ITER(OS, C, ARG, BODY) dump_os(C, ([&](auto &ARG) -> decltype(auto) {{BODY}; return OS;}));
#define DUMP_OS(OS, C, ARG, BODY) dump_os(OS, C, ([&](auto &ARG) -> decltype(auto) {{BODY;}}))

#endif //DRAGONIR_VALUE_H
