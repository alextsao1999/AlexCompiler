//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_OPCODE_H
#define DRAGON_OPCODE_H

#define OPCODE_LIST(S)       \
S(Nop,    0, Instruction)    \
S(Alloca, 0, AllocaInst)     \
S(Cast,   1, CastInst)       \
S(Copy,   1, CopyInst)       \
S(Phi,    0, PhiInst)        \
S(Br,     1, BranchInst)     \
S(CondBr, 3, CondBrInst)     \
S(Unary,  1, UnaryInst)      \
S(Binary, 2, BinaryInst)     \
S(GEP,    2, Instruction)    \
S(Ret,    0, RetInst)        \
S(Call,   0, CallInst)       \
S(Load,   1, LoadInst)       \
S(Store,  2, StoreInst)

enum Opcode {
#define DEFINE_OPCODE(name, c, k) Opcode##name,
    OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
};

static const size_t OpcodeNum[] = {
#define DEFINE_OPCODE(name, c, k) c,
        OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
};

#endif //DRAGON_OPCODE_H
