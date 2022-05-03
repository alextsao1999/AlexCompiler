//
// Created by Alex on 2022/3/10.
//

#include "patgen.h"

int main() {
    PatGen PG;
    PG.parse("C:\\Users\\Alex\\Desktop\\DragonIR\\utils\\patgen\\arm.target");

/*    PG.parseFromString(R"(
target Test {
  // Define registers
  register ireg : i32 { SP, R0, R1, R2, R3, R4, R5, R6, R7 }
  // Define the machine instructions
  instructions {
    ireg: add_rr(ireg, ireg);
    ireg: add_ri(ireg, imm);
  }

  // Define the rewriter patterns
  patterns {
    Add($1, $2) = add_rr(ireg: $1, ireg: $2);
  }
}

)");*/
    RewriterGenerator Gen(PG);

    Gen.generate();
    Gen.print();

    return 0;
}