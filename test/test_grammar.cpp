//
// Created by Alex on 2022/3/12.
//
#include "lest.hpp"
#include "parser.h"

value_t ParseCode(const char *str) {
    GLRParser<> Parser(false);
    Parser.reset(str, str + strlen(str));
    Parser.parse();
    return Parser.value();
}

const lest::test Specification[] = {
        CASE("Expression") {
            const char *ExprTest = "import aaa.bbb.ccc;\n"
                                   "int<int, value> main() {\n"
                                   "  a = 1 + 2 * 5;\n"
                                   "}";
            GLRParser<> Parser(false);
            Parser.reset(ExprTest, ExprTest + strlen(ExprTest));
            Parser.parse();
            EXPECT(Parser.accept());
            auto &Value = Parser.value();
            EXPECT(Value == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "id": 2,
            "items": [
                "aaa",
                "bbb",
                "ccc"
            ],
            "kind": "Import"
        },
        {
            "block": [
                {
                    "id": 13,
                    "kind": "ExprStmt",
                    "value": {
                        "id": 27,
                        "kind": "AssignExpr",
                        "left": {
                            "id": 31,
                            "kind": "VariableExpr",
                            "name": "a"
                        },
                        "right": {
                            "id": 25,
                            "kind": "BinaryExpr",
                            "left": {
                                "id": 36,
                                "kind": "Number",
                                "string": "1"
                            },
                            "op": "+",
                            "right": {
                                "id": 25,
                                "kind": "BinaryExpr",
                                "left": {
                                    "id": 36,
                                    "kind": "Number",
                                    "string": "2"
                                },
                                "op": "*",
                                "right": {
                                    "id": 36,
                                    "kind": "Number",
                                    "string": "5"
                                }
                            }
                        }
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "args": [
                    {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    },
                    {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "value"
                    }
                ],
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}
)json"_json);

            const char *VarDeclare = "int main() {\n"
                                     "  int a = 1;\n"
                                     "  int b = 2;\n"
                                     "  int c = 3;\n"
                                     "  int d_44 = 4;\n"
                                     "}";

            EXPECT(ParseCode(VarDeclare) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "id": 23,
                    "init": {
                        "id": 36,
                        "kind": "Number",
                        "string": "1"
                    },
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 36,
                        "kind": "Number",
                        "string": "2"
                    },
                    "kind": "VariableDeclare",
                    "name": "b",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 36,
                        "kind": "Number",
                        "string": "3"
                    },
                    "kind": "VariableDeclare",
                    "name": "c",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 36,
                        "kind": "Number",
                        "string": "4"
                    },
                    "kind": "VariableDeclare",
                    "name": "d_44",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}

)json"_json);

            const char *NewTest = "int main() {\n"
                                  "  int a = new int(1);\n"
                                  "  int b = new int(2);\n"
                                  "  int c = new int(3);\n"
                                  "  int d_44 = new int(4);\n"
                                  "}";
            EXPECT(ParseCode(NewTest) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "1"
                            }
                        ],
                        "id": 30,
                        "kind": "NewExpr",
                        "type": {
                            "id": 24,
                            "kind": "TypeSpecifier",
                            "type": "int"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "2"
                            }
                        ],
                        "id": 30,
                        "kind": "NewExpr",
                        "type": {
                            "id": 24,
                            "kind": "TypeSpecifier",
                            "type": "int"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "b",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "3"
                            }
                        ],
                        "id": 30,
                        "kind": "NewExpr",
                        "type": {
                            "id": 24,
                            "kind": "TypeSpecifier",
                            "type": "int"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "c",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "4"
                            }
                        ],
                        "id": 30,
                        "kind": "NewExpr",
                        "type": {
                            "id": 24,
                            "kind": "TypeSpecifier",
                            "type": "int"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "d_44",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}
)json"_json);

            const char *InvokeTest = "int main() {\n"
                                     "    int a;\n"
                                     "    a = test();\n"
                                     "    int a = test(1);\n"
                                     "    int b = test(4, 5);\n"
                                     "    int c = test(7, 8, 9);\n"
                                     "    int d = test(10, 11, 12, 13);\n"
                                     "    int value = a + b + c;"
                                     "    return 0;\n"
                                     "}";

            EXPECT(ParseCode(InvokeTest) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 13,
                    "kind": "ExprStmt",
                    "value": {
                        "id": 27,
                        "kind": "AssignExpr",
                        "left": {
                            "id": 31,
                            "kind": "VariableExpr",
                            "name": "a"
                        },
                        "right": {
                            "args": null,
                            "id": 34,
                            "kind": "InvokeExpr",
                            "name": "test"
                        }
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "1"
                            }
                        ],
                        "id": 34,
                        "kind": "InvokeExpr",
                        "name": "test"
                    },
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "4"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "5"
                            }
                        ],
                        "id": 34,
                        "kind": "InvokeExpr",
                        "name": "test"
                    },
                    "kind": "VariableDeclare",
                    "name": "b",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "7"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "8"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "9"
                            }
                        ],
                        "id": 34,
                        "kind": "InvokeExpr",
                        "name": "test"
                    },
                    "kind": "VariableDeclare",
                    "name": "c",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "args": [
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "10"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "11"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "12"
                            },
                            {
                                "id": 36,
                                "kind": "Number",
                                "string": "13"
                            }
                        ],
                        "id": 34,
                        "kind": "InvokeExpr",
                        "name": "test"
                    },
                    "kind": "VariableDeclare",
                    "name": "d",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 25,
                            "kind": "BinaryExpr",
                            "left": {
                                "id": 31,
                                "kind": "VariableExpr",
                                "name": "a"
                            },
                            "op": "+",
                            "right": {
                                "id": 31,
                                "kind": "VariableExpr",
                                "name": "b"
                            }
                        },
                        "op": "+",
                        "right": {
                            "id": 31,
                            "kind": "VariableExpr",
                            "name": "c"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "value",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 15,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 36,
                        "kind": "Number",
                        "string": "0"
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}
)json"_json);

            const char *MoreExprTest = "int main() {\n"
                                       "    int int_value = 0;\n"
                                       "    int value = 1 + 2 * 3 / 4 - 5;\n"
                                       "    bool eq = 1 == 2;\n"
                                       "    bool lt = 1 < 2;\n"
                                       "    bool gt = 1 > 2;\n"
                                       "    bool and = 1 && 2;\n"
                                       "    bool or = 1 || 2;\n"
                                       "    return 0;\n"
                                       "}";

            EXPECT(ParseCode(MoreExprTest) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "id": 23,
                    "init": {
                        "id": 36,
                        "kind": "Number",
                        "string": "0"
                    },
                    "kind": "VariableDeclare",
                    "name": "int_value",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": "+",
                        "right": {
                            "id": 25,
                            "kind": "BinaryExpr",
                            "left": {
                                "id": 25,
                                "kind": "BinaryExpr",
                                "left": {
                                    "id": 36,
                                    "kind": "Number",
                                    "string": "2"
                                },
                                "op": "*",
                                "right": {
                                    "id": 25,
                                    "kind": "BinaryExpr",
                                    "left": {
                                        "id": 36,
                                        "kind": "Number",
                                        "string": "3"
                                    },
                                    "op": "/",
                                    "right": {
                                        "id": 36,
                                        "kind": "Number",
                                        "string": "4"
                                    }
                                }
                            },
                            "op": "-",
                            "right": {
                                "id": 36,
                                "kind": "Number",
                                "string": "5"
                            }
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "value",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": "==",
                        "right": {
                            "id": 36,
                            "kind": "Number",
                            "string": "2"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "eq",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "bool"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": "<",
                        "right": {
                            "id": 36,
                            "kind": "Number",
                            "string": "2"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "lt",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "bool"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": ">",
                        "right": {
                            "id": 36,
                            "kind": "Number",
                            "string": "2"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "gt",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "bool"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": "&&",
                        "right": {
                            "id": 36,
                            "kind": "Number",
                            "string": "2"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "and",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "bool"
                    }
                },
                {
                    "id": 23,
                    "init": {
                        "id": 25,
                        "kind": "BinaryExpr",
                        "left": {
                            "id": 36,
                            "kind": "Number",
                            "string": "1"
                        },
                        "op": "||",
                        "right": {
                            "id": 36,
                            "kind": "Number",
                            "string": "2"
                        }
                    },
                    "kind": "VariableDeclare",
                    "name": "or",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "bool"
                    }
                },
                {
                    "id": 15,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 36,
                        "kind": "Number",
                        "string": "0"
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}
)json"_json);

        },
        CASE("Class") {
            const char *ClassMemberTest = "class A {"
                                          "  int a;"
                                          "  int b;"
                                          "  int cc;"
                                          "  int dd;"
                                          "  List<int> list;"
                                          "}";
            EXPECT(ParseCode(ClassMemberTest) == R"(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "body": [
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "b",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "cc",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "dd",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                },
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "list",
                    "type": {
                        "args": [
                            {
                                "id": 24,
                                "kind": "TypeSpecifier",
                                "type": "int"
                            }
                        ],
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "List"
                    }
                }
            ],
            "id": 4,
            "kind": "ClassDeclare",
            "name": "A",
            "super": null,
            "template": null
        }
    ]
}
)"_json);
        },
        CASE("BaseClass") {
            const char *ClassDefineTest = "class A<A,B,C> : B {\n"
                                          "  int a;"
                                          "}";

            EXPECT(ParseCode(ClassDefineTest) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "body": [
                {
                    "id": 23,
                    "kind": "VariableDeclare",
                    "name": "a",
                    "type": {
                        "id": 24,
                        "kind": "TypeSpecifier",
                        "type": "int"
                    }
                }
            ],
            "id": 4,
            "kind": "ClassDeclare",
            "name": "A",
            "super": "B",
            "template": [
                "A",
                "B",
                "C"
            ]
        }
    ]
}

)json"_json);

        },
        CASE("While") {
            const char *Test = "int main() {"
                               "  while (true) {"
                               "    int a = 10;"
                               "  }"
                               "}";
            EXPECT(ParseCode(Test) == R"json(
{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "body": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": [
                            {
                                "id": 23,
                                "init": {
                                    "id": 36,
                                    "kind": "Number",
                                    "string": "10"
                                },
                                "kind": "VariableDeclare",
                                "name": "a",
                                "type": {
                                    "id": 24,
                                    "kind": "TypeSpecifier",
                                    "type": "int"
                                }
                            }
                        ]
                    },
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "id": 19,
                    "kind": "WhileStmt"
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}


)json"_json);
        },
        CASE("Do While") {
            const char *Test = "int main() {"
                               "  do {"
                               "    int a = 10;"
                               "  } while(true);"
                               "}";
            EXPECT(ParseCode(Test) == R"json(

{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "body": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": [
                            {
                                "id": 23,
                                "init": {
                                    "id": 36,
                                    "kind": "Number",
                                    "string": "10"
                                },
                                "kind": "VariableDeclare",
                                "name": "a",
                                "type": {
                                    "id": 24,
                                    "kind": "TypeSpecifier",
                                    "type": "int"
                                }
                            }
                        ]
                    },
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "id": 20,
                    "kind": "DoWhileStmt"
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}


)json"_json);
        },
        CASE("While With Break") {
            const char *Test = "int main() {"
                               "  while (true) {"
                               "    break;"
                               "    continue;"
                               "  }"
                               "}";
            //std::cout << ParseCode(Test).dump(4) << std::endl << std::endl;
            EXPECT(ParseCode(Test) == R"json(

{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "body": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": [
                            {
                                "id": 17,
                                "kind": "BreakStmt"
                            },
                            {
                                "id": 16,
                                "kind": "ContinueStmt"
                            }
                        ]
                    },
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "id": 19,
                    "kind": "WhileStmt"
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}


)json"_json);
        },
        CASE("If") {
            const char *Test = "int main() {"
                               "  if (true) {"
                               "  }"
                               "}";
            EXPECT(ParseCode(Test) == R"json(

{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "id": 18,
                    "kind": "IfStmt",
                    "then": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": null
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}

)json"_json);
        },
        CASE("If Else") {
            const char *Test = "int main() {"
                               "  if (true) {"
                               "  } else {"
                               "  }"
                               "}";
            //std::cout << ParseCode(Test).dump(4) << std::endl << std::endl;
            EXPECT(ParseCode(Test) == R"json(

{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "else": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": null
                    },
                    "id": 18,
                    "kind": "IfStmt",
                    "then": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": null
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}
)json"_json);
        },
        CASE("If Else If") {
            const char *Test = "int main() {"
                               "  if (true) {"
                               "  } else if (false) {"
                               "  }"
                               "}";
            EXPECT(ParseCode(Test) == R"json(

{
    "id": 1,
    "kind": "Program",
    "value": [
        {
            "block": [
                {
                    "condition": {
                        "id": 31,
                        "kind": "VariableExpr",
                        "name": "true"
                    },
                    "else": {
                        "condition": {
                            "id": 31,
                            "kind": "VariableExpr",
                            "name": "false"
                        },
                        "id": 18,
                        "kind": "IfStmt",
                        "then": {
                            "id": 14,
                            "kind": "BlockStmt",
                            "value": null
                        }
                    },
                    "id": 18,
                    "kind": "IfStmt",
                    "then": {
                        "id": 14,
                        "kind": "BlockStmt",
                        "value": null
                    }
                }
            ],
            "id": 11,
            "kind": "FunctionDeclare",
            "name": "main",
            "params": null,
            "type": {
                "id": 24,
                "kind": "TypeSpecifier",
                "type": "int"
            }
        }
    ]
}

)json"_json);
        },

};

int main(int argc, char *argv[]) {
    return lest::run(Specification, argc, argv, std::cout);
}
