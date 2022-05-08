//
// Created by Alex on 2021/10/19.
//

#ifndef DRAGONCOMPILER_INODE_H
#define DRAGONCOMPILER_INODE_H
#include <iostream>
#include <type_traits>
#include <iterator>
#include "Common.h"
#include "Range.h"

///< NodeBase is the base class of Node and Sentinel.
class NodeBase {
    template <typename Ty, typename Traits, typename SentinelTy> friend class INodeListImpl;
    template <typename Ty, typename As, bool> friend class NodeIter;
protected:
    NodeBase *prev = nullptr;
    NodeBase *next = nullptr;

    inline void linkNext(NodeBase *N) {
        assert(N);
        next = N;
        N->prev = this;
    }

    inline void linkPrev(NodeBase *P) {
        assert(P);
        prev = P;
        P->next = this;
    }

    inline constexpr NodeBase *getPrevNode() const {
        return prev;
    }

    inline constexpr NodeBase *getNextNode() const {
        return next;
    }

    inline constexpr void setNextNode(NodeBase *N) {
        next = N;
    }

    inline constexpr void setPrevNode(NodeBase *P) {
        prev = P;
    }

};

template<typename T>
class Node : public NodeBase {
private:
    inline T *getValue() const {
        return static_cast<T *>(this);
    }

public:
    inline T *getPrev() const {
        return static_cast<T *>(getPrevNode());
    }

    inline T *getNext() const {
        return static_cast<T *>(getNextNode());
    }

    inline void setNext(T *N) {
        next = N;
    }

    inline void setPrev(T *P) {
        prev = P;
    }

};
class NodeSentinel : public NodeBase {
public:
    constexpr NodeSentinel() {
        reset();
    }

    inline constexpr bool empty() const {
        return getPrevNode() == this;
    }

    inline constexpr void reset() {
        setNextNode(this);
        setPrevNode(this);
    }

};

template<typename T, typename ParentT>
class NodeWithParent : public Node<T> {
private:
    ParentT *parent = nullptr;
public:
    NodeWithParent() = default;
    NodeWithParent(ParentT *parent) {
        assert(parent);
        parent->append(static_cast<T *>(this));
    }

    void setParent(ParentT *Parent) {
        parent = Parent;
    }

    inline ParentT *getParent() {
        return parent;
    }

    inline ParentT *getParent() const {
        return parent;
    }

    void replaceBy(T *node) {
        assert(parent);
        parent->replace(static_cast<T *>(this), node);
    }

    void eraseFromParent() {
        assert(parent);
        parent->erase(static_cast<T *>(this));
    }

    void insertAfterThis(T *node) {
        assert(node && parent);
        parent->insertAfter(static_cast<T *>(this), node);
    }

    void insertBeforeThis(T *node) {
        assert(node && parent);
        parent->insertBefore(static_cast<T *>(this), node);
    }

    void moveAfter(T *node) {
        assert(node && parent);
        parent->insertAfter(node, static_cast<T *>(this));
    }

    void moveBefore(T *node) {
        assert(node && parent);
        parent->insertBefore(node, static_cast<T *>(this));
    }

};

template<typename Ty, typename As = Ty, bool Reverse = false>
class NodeIter {
    template<typename T, typename Traits, typename SentinelTy> friend class INodeListImpl;
    Ty *cursor = nullptr;
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = As;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    using range = IterRange<NodeIter>;

    NodeIter() {}
    NodeIter(const NodeIter &RHS) : cursor(RHS.cursor) {}
    NodeIter(Ty *cursor) : cursor(cursor) {}
    NodeIter(const Ty *cursor) : cursor(const_cast<Ty *>(cursor)) {}

    bool operator==(const NodeIter &RHS) {
        return cursor == RHS.cursor;
    }

    bool operator==(const NodeIter &RHS) const {
        return cursor == RHS.cursor;
    }

    bool operator!=(const NodeIter &RHS) {
        return cursor != RHS.cursor;
    }

    bool operator!=(const NodeIter &RHS) const {
        return cursor != RHS.cursor;
    }

    pointer operator->() const { return static_cast<pointer>(cursor); }
    reference operator*() const { return *operator->(); }

    inline pointer getPointer() const { return static_cast<pointer>(cursor); }

    inline NodeIter<Ty, As, !Reverse> getReverse() { return NodeIter<Ty, As, !Reverse>(cursor); }
    inline NodeIter<Ty, As, !Reverse> getReverse() const { return NodeIter<Ty, As, !Reverse>(cursor); }

    NodeIter &operator++() {
        cursor = Reverse ? cursor->prev : cursor->next;
        return *this;
    }

    NodeIter operator++(int) {
        auto back = *this;
        ++*this;
        return back;
    }

    NodeIter &operator--() {
        cursor = Reverse ? cursor->next : cursor->prev;
        return *this;
    }

    NodeIter operator--(int) {
        auto back = *this;
        --*this;
        return back;
    }

    NodeIter operator+(int c) {
        auto iter = *this;
        for (; c--; ++iter) {}
        return iter;
    }

};

template<typename T, typename As, bool Reverse>
class IterRange<NodeIter<T, As, Reverse>> {
public:
    using rev_t = IterRange<NodeIter<T, As, !Reverse>>;
    using Iter = NodeIter<T, As, Reverse>;
    Iter _begin;
    Iter _end;
public:
    IterRange() {}
    IterRange(const IterRange &r) : _begin(r._begin), _end(r._end) {}
    IterRange(Iter begin, Iter anEnd) : _begin(begin), _end(anEnd) {}
    Iter begin() const { return _begin; }
    Iter end() const { return _end; }
    bool empty() {
        return _begin == _end;
    }
    bool empty() const {
        return _begin == _end;
    }
    rev_t reverse() {
        return rev_t((--_end).getReverse(), (--_begin).getReverse());
    }
};

///< Alloc traits
template <typename T>
struct ListAllocTrait {
    template<typename ...Args>
    static T *newNode(Args &&...args) { return new T(std::forward<Args...>(args)...); }
    static void refNode(T *V) {}
    static void derefNode(T *V) { delete V; }
};
template <typename T>
struct ListNonAllocTrait {
    template<typename ...Args>
    static T *newNode(Args &&...args) { return nullptr; }
    static void refNode(T *V) {}
    static void derefNode(T *V) {}
};
template <typename T>
struct ListRefTrait : public ListNonAllocTrait<T> {
    static void refNode(T *V) {
        V->incRef();
    }
    static void derefNode(T *V) {
        V->decRef();
    }
};
template <typename T>
struct StrongRefTrait : public ListRefTrait<T> {
    static void derefNode(T *V) { delete V; }
};

template<typename T, typename Traits, typename SentinelTy = NodeSentinel>
class INodeListImpl : protected Traits {
private:
    using NodeTy = NodeBase;
    static_assert(std::is_base_of_v<NodeTy, T>, "INodeWithParent must be base of element type");
    SentinelTy sentinel;
public:
    template<typename As>
    using iterator_as = NodeIter<NodeTy, As, false>;
    using iterator = NodeIter<NodeTy, T, false>;
    using reverse_iterator = NodeIter<NodeTy, T, true>;
    using pointer = T *;
    using reference = T &;
    using const_reference = const T &;

    INodeListImpl() {}
    INodeListImpl(const INodeListImpl &RHS) = delete;
    ~INodeListImpl() {
        delete_range(begin(), end());
    }

    INodeListImpl &operator=(const INodeListImpl &RHS) = delete;

    INodeListImpl &operator=(INodeListImpl &&RHS) {
        erase(begin(), end());
        RHS.sentinel.getNextNode()->linkPrev(&sentinel);
        RHS.sentinel.getPrevNode()->linkNext(&sentinel);
        RHS.sentinel.reset();
        return *this;
    }

    inline bool empty() const {
        return sentinel.empty();
    }

    inline void clear() {
        erase(begin(), end());
    }

    inline void push_front(pointer node) {
        insert_after(end(), node);
    }

    template<typename ...Args>
    inline reference emplace_front(Args &&...args) {
        auto *Node = Traits::newNode(args...);
        push_front(Node);
        return *Node;
    }

    inline void push_back(pointer node) {
        insert_before(end(), node);
    }

    template<typename ...Args>
    inline reference emplace_back(Args &&...args) {
        auto *Node = Traits::newNode(args...);
        insert_before(Node);
        return *Node;
    }

    template<typename ...Args>
    inline iterator emplace(iterator where, Args &&...args) {
        auto *Node = Traits::newNode(args...);
        insert_before(where, Node);
        return Node;
    }

    inline void pop_back() {
        assert(!empty());
        erase(--end());
    }

    inline reference front() {
        assert(!empty());
        return *begin();
    }

    inline reference back() {
        assert(!empty());
        return *(--end());
    }

    inline void push_back(T &&node) {
        insert_before(end(), Traits::cloneNode(std::move(node)));
    }

    inline iterator insert(iterator where, pointer node) {
        return insert_before(where, node);
    }

    template<typename It>
    inline void insert(iterator where, It begin, It end) {
        for (; begin != end; ++begin) {
            insert(where, *begin);
        }
    }

    inline size_t size() const {
        return std::distance(begin(), end());
    }

    inline void remove(iterator where) {
        auto Next = where->getNext();
        auto Prev = where->getPrev();

        Next->setPrev(Prev);
        Prev->setNext(Next);

        //where->linkNext(where.getPointer());
    }

    inline void remove(iterator first, iterator last) {
        //auto End = last->getPrev();
        auto Prev = first->getPrev();
        Prev->linkNext(last.getPointer());
        //first->linkPrev(End);
    }

    inline void extract(iterator where) {
        remove(where);
        where->linkNext(where.getPointer());
    }

    inline void extract(iterator first, iterator last) {
        auto End = last->getPrev();
        remove(first, last);
        first->linkPrev(End);
    }

    inline void inject_before(iterator where, pointer node) {
        auto Prev = where->getPrev();
        auto End = node->getPrev();

        Prev->linkNext(node);
        End->linkNext(where.getPointer());
    }

    inline void inject_after(iterator where, pointer node) {
        auto Next = where->getNext();
        auto End = node->getPrev();

        where->linkNext(node);
        End->linkNext(Next);
    }

    inline void inject_before(iterator where, pointer first, pointer last) {
        auto Prev = where->getPrev();
        last->linkNext(where.getPointer());
        first->linkPrev(Prev);
    }

    inline void inject_after(iterator where, pointer first, pointer last) {
        auto Next = where->getNext();
        first->linkPrev(where.getPointer());
        last->linkNext(Next);
    }

    inline iterator erase(iterator where) {
        if (where == end())
            return where;
        auto *Next = where->getNext();
        remove(where);
        Traits::derefNode(where.getPointer());
        return Next;
    }

    inline void erase(iterator first, iterator last) {
        remove(first, last);
        delete_range(first, last);
    }

    inline iterator insert_after_without_ref(iterator where, pointer node) {
        assert(node);
        // no unlink
        auto Next = where->getNext();
        Next->setPrev(node);
        node->setPrev(where.getPointer());
        node->setNext(Next);
        where->setNext(node);

        return node;
    }

    inline iterator insert_before_without_ref(iterator where, pointer node) {
        assert(node);
        // no unlink
        auto Prev = where->getPrev();
        Prev->setNext(node);
        node->setPrev(Prev);
        node->setNext(where.getPointer());
        where->setPrev(node);
        return node;
    }

    inline iterator insert_after(iterator where, pointer node) {
        assert(node);
        // before insert, node must be unlinked
        if (node->getPrev() && node->getNext()) {
            remove(node);
        }
        insert_after_without_ref(where, node);
        Traits::refNode(node);
        return node;
    }

    inline iterator insert_before(iterator where, pointer node) {
        assert(node);
        // before insert, node must be unlinked
        if (node->getPrev() && node->getNext()) {
            remove(node);
        }
        insert_before_without_ref(where, node);
        Traits::refNode(node);
        return node;
    }

    inline iterator replace_without_ref(iterator where, pointer node) {
        assert(node);
        auto *Ptr = where.getPointer();
        auto *Prev = Ptr->getPrev();
        auto *Next = Ptr->getNext();
        Prev->setNext(node);
        Next->setPrev(node);
        node->setPrev(Prev);
        node->setNext(Next);
        Traits::derefNode(Ptr);
        return node;
    }

    inline iterator replace(iterator where, pointer node) {
        if (node->getPrev() && node->getNext()) {
            remove(node);
        }
        replace_without_ref(where, node);
        Traits::refNode(node);
        return iterator(node);
    }

    inline iterator begin() {
        return ++iterator(&sentinel);
    }

    inline iterator end() {
        return iterator(&sentinel);
    }

    inline iterator begin() const {
        return ++iterator(&sentinel);
    }

    inline iterator end() const {
        return iterator(&sentinel);
    }

    inline reverse_iterator rbegin() {
        return (--end()).getReverse();
    }

    inline reverse_iterator rend() {
        return end().getReverse();
    }

    inline reverse_iterator rbegin() const {
        return (--end()).getReverse();
    }

    inline reverse_iterator rend() const {
        return end().getReverse();
    }

    inline auto rev() const {
        return iter(rbegin(), rend());
    }

    inline SentinelTy *get_sentinel() const {
        return &sentinel;
    }

protected:
    inline void delete_range(iterator first, iterator last) {
        /*while (first != last) {
            auto Cur = first++;
            Traits::derefNode(Cur.getPointer());
        }*/
        if (first == last) {
            return;
        }
        do {
            auto Cur = first++;
            Traits::derefNode(Cur.getPointer());
        } while (first != last);

    }

};
template<typename T, typename Traits = ListAllocTrait<T>>
using NodeList = INodeListImpl<T, Traits, NodeSentinel>;

template<typename ParentTy, typename NodeTy, typename Traits = StrongRefTrait<NodeTy>>
class NodeParent {
public:
    using NodeListTy = INodeListImpl<NodeTy, Traits, NodeSentinel>;
    template <typename As>
    using iterator_as = typename NodeListTy::template iterator_as<As>;
    using iterator = typename NodeListTy::iterator;
    using reverse_iterator = typename NodeListTy::reverse_iterator;

    NodeParent() = default;

    NodeParent(const NodeListTy &list) : list(list) {}

    void erase(NodeTy *node) {
        list.erase(node);
    }

    // remove node from parent, but not deref it
    void remove(NodeTy *node) {
        list.remove(node);
    }

    void append(NodeTy *node) {
        assert(node);
        if (auto *OldParent = node->getParent()) {
            // 之前有父节点, 先从父节点中移除
            OldParent->remove(node);
            node->setParent(static_cast<ParentTy *>(this));
            list.insert_before_without_ref(list.end(), node);
        } else {
            node->setParent(static_cast<ParentTy *>(this));
            list.push_back(node);
        }
    }

    void insertAfter(NodeTy *node, NodeTy *after) {
        if (auto *OldParent = after->getParent()) {
            // 之前有父节点, 先从父节点中移除
            OldParent->remove(after);
            after->setParent(static_cast<ParentTy *>(this));
            list.insert_after_without_ref(node, after);
        } else {
            after->setParent(static_cast<ParentTy *>(this));
            list.insert_after(node, after);
        }
    }

    void insertBefore(NodeTy *node, NodeTy *before) {
        if (auto *OldParent = before->getParent()) {
            // 之前有父节点, 先从父节点中移除
            OldParent->remove(before);
            before->setParent(static_cast<ParentTy *>(this));
            list.insert_before_without_ref(node, before);
        } else {
            before->setParent(static_cast<ParentTy *>(this));
            list.insert_before(node, before);
        }
    }

    void replace(NodeTy *node, NodeTy *newNode) {
        if (auto *OldParent = newNode->getParent()) {
            // 之前有父节点, 先从父节点中移除
            OldParent->remove(newNode);
            newNode->setParent(static_cast<ParentTy *>(this));
            list.replace_without_ref(node, newNode);
        } else {
            newNode->setParent(static_cast<ParentTy *>(this));
            list.replace(node, newNode);
        }
    }

    NodeListTy &getSubList() {
        return list;
    }

    const NodeListTy &getSubList() const {
        return list;
    }

protected:
    NodeListTy list;
};

#endif //DRAGONCOMPILER_INODE_H
