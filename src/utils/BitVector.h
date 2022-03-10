//
// Created by Alex on 2021/11/10.
//

#ifndef DRAGONCOMPILER_BITVECTOR_H
#define DRAGONCOMPILER_BITVECTOR_H

#include <memory>
#include <cstring>
#include <cstdlib>
#include <Common.h>
class BitVector {
    using unit_t = uint64_t;
    using small_t = uint32_t;
    unit_t *Bits = reinterpret_cast<unit_t *>(&SmallBits);
    union {
        size_t Size;
        struct {
            small_t SmallBits = 0;
            uint8_t SmallSize = 0;
        };
    };
    bool Small = true; // is Small Bits?

    class BitIterator {
        BitVector *Vec;
        size_t Index;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        BitIterator(BitVector *vec, size_t index) : Vec(vec), Index(index) {}

        bool operator==(const BitIterator &RHS) {
            return Vec == RHS.Vec && Index == RHS.Index;
        }

        bool operator!=(const BitIterator &RHS) {
            return !operator==(RHS);
        }

        pointer operator->() { return &Index; }
        reference operator*() { return *operator->(); }

        BitIterator &operator++() {
            Index = Vec->next(Index + 1);
            return *this;
        }

        BitIterator operator++(int) {
            auto back = *this;
            ++*this;
            return back;
        }
    };

public:
    BitVector() {
        resize(0);
    }

    BitVector(size_t size) {
        resize(size);
    }

    BitVector(const char *str) {
        auto len = strlen(str);
        resize(len);
        for (int i = 0; i < len; ++i) {
            if (str[i] == '1') {
                set(i);
            }
        }
    }

    ~BitVector() {
        resize(0);
    }

    BitVector(BitVector &&RHS) {
        Bits = RHS.Bits;
        Size = RHS.Size;
        Small = RHS.Small;
        RHS.Small = true;
        RHS.resize(0);
    }

    BitVector(const BitVector& RHS) {
        if (RHS.size()) {
            resize(RHS.size());
            std::memcpy(Bits, RHS.Bits, BitCountToByte(size()));
        } else {
            resize(0);
        }
    }

    BitIterator begin() {
        return BitIterator(this, next(0));
    }

    BitIterator end() {
        return BitIterator(this, size());
    }

    size_t next(size_t Index) {
        while (!get(Index) && Index < size()) {
            Index++;
        }
        return Index;
    }

    void resize(size_t newSize) {
        if (size() == newSize) {
            return;
        }
        if (newSize <= BitCount<small_t>()) {
            if (!Small) {
                auto fitSize = std::min(size(), newSize);
                SmallBits = 0;
                move_bits(&SmallBits, Bits, fitSize);
                free(Bits);
            }
            SmallSize = newSize;
            Bits = (unit_t *) &SmallBits;
            Small = true;
        } else {
            auto *newBits = (unit_t *) calloc(BitCountToUnitLength(newSize), sizeof(unit_t));
            auto fitSize = std::min(size(), newSize);
            move_bits(newBits, Bits, fitSize);
            if (!Small) {
                free(Bits);
            }
            Size = newSize;
            Bits = newBits;
            Small = false;
        }
    }

    bool get(size_t i) const {
        return Bits[i / BitCount<unit_t>()] & (unit_t(1) << (i % BitCount<unit_t>()));
    }

    void set(size_t i) {
        Bits[i / BitCount<unit_t>()] |= (unit_t(1) << (i % BitCount<unit_t>()));
    }

    void clear(size_t i) {
        Bits[i / BitCount<unit_t>()] &= ~(unit_t(1) << (i % BitCount<unit_t>()));
    }

    inline size_t size() const {
        return Small ? SmallSize : Size;
    }

    inline void fill(bool bit) {
        std::memset(Bits, bit ? uint8_t(-1) : uint8_t(0), BitCountToByte(size()));
    }

    inline void flip() {
        if (Small) {
            auto margin = BitCount<small_t>() - SmallSize;
            SmallBits = ~SmallBits & (small_t(-1) >> margin);
        } else {
            auto i = 0;
            while (i < BitCountToUnitLength(Size) - 1)
                Bits[i] = ~Bits[i++];
            auto margin = BitCount<unit_t>() - (Size % BitCount<unit_t>());
            Bits[i] = ~Bits[i] & (unit_t(-1) >> margin);
        }
    }

    inline BitVector operator-(BitVector RHS) {
        RHS.flip();
        RHS &= *this;
        return RHS;
    }

    inline BitVector operator&(const BitVector &RHS) {
        BitVector Ret = *this;
        Ret &= RHS;
        return Ret;
    }

    inline BitVector operator|(const BitVector &RHS) {
        BitVector Ret = *this;
        Ret |= RHS;
        return Ret;
    }

    inline BitVector &operator&=(const BitVector &RHS) {
        if (size() == RHS.size()) {
            if (Small) {
                SmallBits &= RHS.SmallBits;
            } else {
                for (auto i = 0; i < BitCountToUnitLength(Size); i++) {
                    Bits[i] &= RHS.Bits[i];
                }
            }
        } else {
            unreachable();
        }
        return *this;
    }

    inline BitVector &operator|=(const BitVector &RHS) {
        if (size() == RHS.size()) {
            if (Small) {
                SmallBits |= RHS.SmallBits;
            } else {
                for (auto i = 0; i < BitCountToUnitLength(Size); i++) {
                    Bits[i] |= RHS.Bits[i];
                }
            }
        } else {
            unreachable();
        }
        return *this;
    }

    inline BitVector &operator&=(const unit_t &Val) {
        auto margin = BitCount<unit_t>() > size() ? (BitCount<unit_t>() - size()) : 0;
        Bits[0] = Bits[0] & (Val & (unit_t(-1) >> margin));
        return *this;
    }

    inline BitVector &operator|=(const unit_t &Val) {
        auto margin = BitCount<unit_t>() > size() ? (BitCount<unit_t>() - size()) : 0;
        Bits[0] = Bits[0] | (Val & (unit_t(-1) >> margin));
        return *this;
    }

    inline BitVector operator~() const {
        auto vec = *this;
        vec.flip();
        return vec;
    }

    inline BitVector &operator=(const BitVector &RHS) {
        if (!RHS.size()) {
            resize(0);
            return *this;
        }
        resize(RHS.size());
        std::memcpy(Bits, RHS.Bits, BitCountToByte(size()));
        return *this;
    }

    inline BitVector &operator=(BitVector &&RHS) {
        if (!RHS.Small) {
            Bits = RHS.Bits;
        }
        Size = RHS.Size;
        Small = RHS.Small;
        RHS.Small = true;
        RHS.resize(0);
        return *this;
    }

    inline bool operator==(const BitVector &RHS) const {
        if (size() == RHS.size()) {
            return std::memcmp(Bits, RHS.Bits, BitCountToByte(size())) == 0;
        } else {
            unreachable();
        }
        return false;
    }

    inline bool operator!=(const BitVector &RHS) const {
        return !(*this == RHS);
    }

    inline bool operator[](size_t i) const {
        return get(i);
    }

private:
    inline void move_bits(void *dest, void *src, size_t n_bits) {
        auto byte = n_bits / 8;
        auto reminder = n_bits % 8;
        std::memcpy(dest, src, byte);
        if (reminder) {
            *((uint8_t *) dest + byte) = *((uint8_t *) src + byte) & (uint8_t(-1) >> reminder);
        }

    }

    template<typename Ty>
    static constexpr size_t BitCount() {
        return sizeof(Ty) * 8;
    }

    static inline size_t BitCountToUnitLength(size_t size) {
        return (size - 1) / BitCount<unit_t>() + 1;
    }

    static inline size_t BitCountToByte(size_t size) {
        return (size - 1) / 8 + 1;
    }

};

inline std::ostream &operator<<(std::ostream &os, const BitVector &vec) {
    os << vec.size();
    os << '(';
    for (auto i = 0; i < vec.size(); ++i)
        os << vec.get(i) ;
    os << ')';
    return os;
}


#endif //DRAGONCOMPILER_BITVECTOR_H
