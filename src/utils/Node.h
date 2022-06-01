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
    template <typename Ty> friend class NodeAccessor;
protected:
    NodeBase *prev = nullptr;
    NodeBase *next = nullptr;
    constexpr NodeBase() = default;
    constexpr NodeBase(NodeBase *prev, NodeBase *next) : prev(prev), next(next) {}

    inline void linkNext(NodeBase *N) {
        ASSERT(N);
        next = N;
        N->prev = this;
    }

    inline void linkPrev(NodeBase *P) {
        ASSERT(P);
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
    constexpr NodeSentinel() : NodeBase(this, this) {}

    ///< check if the linked list is empty
    inline constexpr bool empty() const {
        return getPrevNode() == this;
    }

    ///< reset the sentinel to empty state.
    inline constexpr void reset() {
        setNextNode(this);
        setPrevNode(this);
    }
};

template<typename Ty>
class NodeAccessor {
public:
    static inline Ty *getPrev(Ty *node) {
        return node->getPrevNode();
    }
    static inline Ty *getNext(Ty *node) {
        return node->getNextNode();
    }
    static inline void setNext(Ty *node, Ty *next) {
        node->setNextNode(next);
    }
    static inline void setPrev(Ty *node, Ty *prev) {
        node->setPrevNode(prev);
    }
    static inline void linkNext(Ty *node, Ty *next) {
        node->linkNext(next);
    }
    static inline void linkPrev(Ty *node, Ty *prev) {
        node->linkPrev(prev);
    }
};

template<typename T, typename ParentT>
class NodeWithParent : public Node<T> {
private:
    ParentT *parent = nullptr;
public:
    NodeWithParent() = default;
    NodeWithParent(ParentT *parent) {
        ASSERT(parent);
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
        ASSERT(parent);
        parent->replace(static_cast<T *>(this), node);
    }

    void eraseFromParent() {
        ASSERT(parent);
        parent->erase(static_cast<T *>(this));
    }

    void insertAfterThis(T *node) {
        ASSERT(node && parent);
        parent->insertAfter(static_cast<T *>(this), node);
    }

    void insertBeforeThis(T *node) {
        ASSERT(node && parent);
        parent->insertBefore(static_cast<T *>(this), node);
    }

    void moveAfter(T *node) {
        ASSERT(node && parent);
        parent->insertAfter(node, static_cast<T *>(this));
    }

    void moveBefore(T *node) {
        ASSERT(node && parent);
        parent->insertBefore(node, static_cast<T *>(this));
    }

    bool isSentinel() const {
        return parent->list->get_sentinel() == this;
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
        cursor = Reverse ? NodeAccessor<Ty>::getPrev(cursor) : NodeAccessor<Ty>::getNext(cursor);
        return *this;
    }

    NodeIter operator++(int) {
        auto back = *this;
        ++*this;
        return back;
    }

    NodeIter &operator--() {
        cursor = Reverse ? NodeAccessor<Ty>::getNext(cursor) : NodeAccessor<Ty>::getPrev(cursor);
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

    NodeIter operator+(int c) const {
        auto iter = *this;
        for (; c--; ++iter) {}
        return iter;
    }

    NodeIter next() const {
        auto iter = *this;
        ++iter;
        return iter;
    }

    NodeIter prev() const {
        auto iter = *this;
        --iter;
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
/*
template <typename T>
struct ListRefTrait : public ListNonAllocTrait<T> {
    static void refNode(T *V) {
        V->incRef();
    }
    static void derefNode(T *V) {
        V->decRef();
    }
};
*/
template <typename T>
struct StrongRefTrait : public ListAllocTrait<T> {
    static void refNode(T *V) {}
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
        NodeAccessor<NodeTy>::linkPrev(RHS.sentinel.getNextNode(), &sentinel);
        NodeAccessor<NodeTy>::linkNext(RHS.sentinel.getPrevNode(), &sentinel);
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
        ASSERT(!empty());
        erase(--end());
    }

    inline reference front() {
        ASSERT(!empty());
        return *begin();
    }

    inline reference back() {
        ASSERT(!empty());
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
        auto Next = NodeAccessor<NodeTy>::getNext(where.getPointer());
        auto Prev = NodeAccessor<NodeTy>::getPrev(where.getPointer());
        NodeAccessor<NodeTy>::setPrev(Next, Prev);
        NodeAccessor<NodeTy>::setNext(Prev, Next);
    }

    inline void remove(iterator first, iterator last) {
        auto Prev = NodeAccessor<NodeTy>::getPrev(first.getPointer());
        NodeAccessor<NodeTy>::linkNext(Prev, last.getPointer());
    }

    inline iterator erase(iterator where) {
        if (where == end())
            return where;
        auto Next = NodeAccessor<NodeTy>::getNext(where.getPointer());
        remove(where);
        Traits::derefNode(where.getPointer());
        return iterator(Next);
    }

    inline void erase(iterator first, iterator last) {
        remove(first, last);
        delete_range(first, last);
    }

    inline iterator insert_after_without_ref(iterator where, pointer node) {
        ASSERT(node);
        // no unlink
        auto Next = NodeAccessor<NodeTy>::getNext(where.getPointer());
        NodeAccessor<NodeTy>::setPrev(Next, node);
        NodeAccessor<NodeTy>::setPrev(node, where.getPointer());
        NodeAccessor<NodeTy>::setNext(node, Next);
        NodeAccessor<NodeTy>::setNext(where.getPointer(), node);
        return node;
    }

    inline iterator insert_before_without_ref(iterator where, pointer node) {
        ASSERT(node);
        // no unlink
        auto Prev = NodeAccessor<NodeTy>::getPrev(where.getPointer());
        NodeAccessor<NodeTy>::setNext(Prev, node);
        NodeAccessor<NodeTy>::setPrev(node, Prev);
        NodeAccessor<NodeTy>::setNext(node, where.getPointer());
        NodeAccessor<NodeTy>::setPrev(where.getPointer(), node);
        return node;
    }

    inline iterator insert_after(iterator where, pointer node) {
        ASSERT(node);
        // before insert, node must be unlinked
        if (node->getPrev() && node->getNext()) {
            remove(node);
        }
        insert_after_without_ref(where, node);
        Traits::refNode(node);
        return node;
    }

    inline iterator insert_before(iterator where, pointer node) {
        ASSERT(node);
        // before insert, node must be unlinked
        if (node->getPrev() && node->getNext()) {
            remove(node);
        }
        insert_before_without_ref(where, node);
        Traits::refNode(node);
        return node;
    }

    inline iterator replace_without_ref(iterator where, pointer node) {
        ASSERT(node);
        auto Ptr = where.getPointer();
        auto Prev = NodeAccessor<NodeTy>::getPrev(Ptr);
        auto Next = NodeAccessor<NodeTy>::getNext(Ptr);
        NodeAccessor<NodeTy>::setNext(Prev, node);
        NodeAccessor<NodeTy>::setPrev(Next, node);
        NodeAccessor<NodeTy>::setPrev(node, Prev);
        NodeAccessor<NodeTy>::setNext(node, Next);
        Traits::derefNode(Ptr);
        return node;
    }

    inline iterator replace(iterator where, pointer node) {
        if (NodeAccessor<NodeTy>::getPrev(node) && NodeAccessor<NodeTy>::getNext(node)) {
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
    template<typename T, typename ParentT> friend class NodeWithParent;
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
        ASSERT(node);
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
