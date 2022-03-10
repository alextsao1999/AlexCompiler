//
// Created by Alex on 2021/10/22.
//

#ifndef DRAGONCOMPILER_RANGE_H
#define DRAGONCOMPILER_RANGE_H

template<typename Iter>
class IterRange {
    Iter _begin;
    Iter _end;
public:
    IterRange() {}
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
