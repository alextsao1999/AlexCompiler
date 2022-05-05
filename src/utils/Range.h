//
// Created by Alex on 2021/10/22.
//

#ifndef DRAGONCOMPILER_RANGE_H
#define DRAGONCOMPILER_RANGE_H
#include <type_traits>
template<typename Iter>
class IterRange {
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
};

///< Seldom used, this class can be removed.
template<typename Iter, typename F>
class IterWrapper {
public:
    using value_type = typename F::value_type;
    using reference = value_type *;
    using pointer = value_type *;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
private:
    Iter _iter;
public:
    IterWrapper(Iter iter) : _iter(iter) {}
    IterWrapper(const IterWrapper &r) : _iter(r._iter) {}
    IterWrapper &operator++() {
        ++_iter;
        return *this;
    }
    IterWrapper operator++(int) {
        IterWrapper tmp(*this);
        ++_iter;
        return tmp;
    }
    bool operator!=(const IterWrapper &r) {
        return _iter != r._iter;
    }
    bool operator==(const IterWrapper &r) {
        return _iter == r._iter;
    }

    difference_type operator-(const IterWrapper &r) {
        return _iter - r._iter;
    }

    pointer operator->() {
        return F()(*_iter);
    }

    reference operator*() {
        return F()(*_iter);
    }

};

template<typename Iter>
inline IterRange<Iter> iter(Iter begin, Iter end) {
    return IterRange<Iter>(begin, end);
};

template<typename Iter>
inline IterRange<Iter> iter(Iter iter) {
    return IterRange<Iter>(iter.begin(), iter.end());
};

template<typename Container>
inline decltype(auto) iter_reverse(const Container& c) {
    return IterRange<decltype(c.rbegin())>(c.rbegin(), c.rend());
};


#endif //DRAGONCOMPILER_RANGE_H
