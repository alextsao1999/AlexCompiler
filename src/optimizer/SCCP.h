//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_SCCP_H
#define DRAGONIR_SCCP_H

struct LatticeValue {
    enum {
        Undefined,
        Constant,
        NotConstant,
    } type;
};

class SCCP {

};


#endif //DRAGONIR_SCCP_H
