//
// Created by Alex on 2022/3/10.
//

#include "patgen.h"

int main() {
    PatGen PG;
    PG.parse("C:\\Users\\Alex\\Desktop\\DragonIR\\utils\\patgen\\arm.target");

    RewriterGenerator Gen(PG);

    Gen.generate();

    return 0;
}