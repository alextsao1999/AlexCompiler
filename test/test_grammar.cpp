//
// Created by Alex on 2022/3/12.
//
#include "test_common.h"

#define CHECK_OR_DUMP_JSON(V, JSON) if (isStrEmpty(JSON)) { \
    std::cout << ParseCode(V).dump(4) << std::endl; \
} else { \
    EXPECT_EQ(ParseCode(V), JSON##_json); \
}

TEST(Grammar, Expr) {
    const char *VarDeclare = "int main() {\n"
                             "  int a = 1;\n"
                             "  int b = 2;\n"
                             "  int c = 3;\n"
                             "  int d_44 = 4;\n"
                             "}";

    CHECK_OR_DUMP_JSON(VarDeclare, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "a",
                            "value": {
                                "id": 27,
                                "kind": "DecLiteral",
                                "value": "1"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "b",
                            "value": {
                                "id": 27,
                                "kind": "DecLiteral",
                                "value": "2"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "c",
                            "value": {
                                "id": 27,
                                "kind": "DecLiteral",
                                "value": "3"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "d_44",
                            "value": {
                                "id": 27,
                                "kind": "DecLiteral",
                                "value": "4"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");

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

    CHECK_OR_DUMP_JSON(InvokeTest, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "a"
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "id": 10,
                    "kind": "AssignStmt",
                    "lval": {
                        "id": 21,
                        "kind": "LVal",
                        "name": "a"
                    },
                    "value": {
                        "args": null,
                        "id": 26,
                        "kind": "FuncCall",
                        "name": "test"
                    }
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "a",
                            "value": {
                                "args": [
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "1"
                                    }
                                ],
                                "id": 26,
                                "kind": "FuncCall",
                                "name": "test"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "b",
                            "value": {
                                "args": [
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "4"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "5"
                                    }
                                ],
                                "id": 26,
                                "kind": "FuncCall",
                                "name": "test"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "c",
                            "value": {
                                "args": [
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "7"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "8"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "9"
                                    }
                                ],
                                "id": 26,
                                "kind": "FuncCall",
                                "name": "test"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "d",
                            "value": {
                                "args": [
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "10"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "11"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "12"
                                    },
                                    {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "13"
                                    }
                                ],
                                "id": 26,
                                "kind": "FuncCall",
                                "name": "test"
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "value",
                            "value": {
                                "id": 23,
                                "kind": "BinExp",
                                "left": {
                                    "id": 23,
                                    "kind": "BinExp",
                                    "left": {
                                        "id": 25,
                                        "kind": "RVal",
                                        "name": "a"
                                    },
                                    "op": "+",
                                    "right": {
                                        "id": 25,
                                        "kind": "RVal",
                                        "name": "b"
                                    }
                                },
                                "op": "+",
                                "right": {
                                    "id": 25,
                                    "kind": "RVal",
                                    "name": "c"
                                }
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "id": 19,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 27,
                        "kind": "DecLiteral",
                        "value": "0"
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");

    const char *MoreExprTest = "int main() {\n"
                               "    int value = 1 + 2 * 3 / 4 - 5;\n"
                               "    int eq = 1 == 2;\n"
                               "    int lt = 1 < 2;\n"
                               "    int gt = 1 > 2;\n"
                               "    return 0;\n"
                               "}";

    CHECK_OR_DUMP_JSON(MoreExprTest, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "value",
                            "value": {
                                "id": 23,
                                "kind": "BinExp",
                                "left": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "1"
                                },
                                "op": "+",
                                "right": {
                                    "id": 23,
                                    "kind": "BinExp",
                                    "left": {
                                        "id": 23,
                                        "kind": "BinExp",
                                        "left": {
                                            "id": 27,
                                            "kind": "DecLiteral",
                                            "value": "2"
                                        },
                                        "op": "*",
                                        "right": {
                                            "id": 23,
                                            "kind": "BinExp",
                                            "left": {
                                                "id": 27,
                                                "kind": "DecLiteral",
                                                "value": "3"
                                            },
                                            "op": "/",
                                            "right": {
                                                "id": 27,
                                                "kind": "DecLiteral",
                                                "value": "4"
                                            }
                                        }
                                    },
                                    "op": "-",
                                    "right": {
                                        "id": 27,
                                        "kind": "DecLiteral",
                                        "value": "5"
                                    }
                                }
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "eq",
                            "value": {
                                "id": 23,
                                "kind": "BinExp",
                                "left": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "1"
                                },
                                "op": "==",
                                "right": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "2"
                                }
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "lt",
                            "value": {
                                "id": 23,
                                "kind": "BinExp",
                                "left": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "1"
                                },
                                "op": "<",
                                "right": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "2"
                                }
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "defs": [
                        {
                            "id": 6,
                            "kind": "VarDef",
                            "name": "gt",
                            "value": {
                                "id": 23,
                                "kind": "BinExp",
                                "left": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "1"
                                },
                                "op": ">",
                                "right": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "2"
                                }
                            }
                        }
                    ],
                    "id": 5,
                    "kind": "VarDecl",
                    "type": "int"
                },
                {
                    "id": 19,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 27,
                        "kind": "DecLiteral",
                        "value": "0"
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");

}

TEST(Grammar, IfStmt) {
    const char *VarDeclare = "int main() {\n"
                             "  if (1 < 2) {\n"
                             "    return 0;\n"
                             "  }\n"
                             "  return 1;\n"
                             "}";
    CHECK_OR_DUMP_JSON(VarDeclare, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "cond": {
                        "id": 23,
                        "kind": "BinExp",
                        "left": {
                            "id": 27,
                            "kind": "DecLiteral",
                            "value": "1"
                        },
                        "op": "<",
                        "right": {
                            "id": 27,
                            "kind": "DecLiteral",
                            "value": "2"
                        }
                    },
                    "id": 13,
                    "kind": "IfStmt",
                    "then": {
                        "id": 12,
                        "kind": "Block",
                        "stmts": [
                            {
                                "id": 19,
                                "kind": "ReturnStmt",
                                "value": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "0"
                                }
                            }
                        ]
                    }
                },
                {
                    "id": 19,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 27,
                        "kind": "DecLiteral",
                        "value": "1"
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");

}

TEST(Grammar, IfElseStmt) {
    const char *VarDeclare = "int main() {\n"
                             "  if (1 < 2) {\n"
                             "    return 0;\n"
                             "  }\n"
                             "  else {\n"
                             "    return 1;\n"
                             "  }\n"
                             "}";
    CHECK_OR_DUMP_JSON(VarDeclare, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "cond": {
                        "id": 23,
                        "kind": "BinExp",
                        "left": {
                            "id": 27,
                            "kind": "DecLiteral",
                            "value": "1"
                        },
                        "op": "<",
                        "right": {
                            "id": 27,
                            "kind": "DecLiteral",
                            "value": "2"
                        }
                    },
                    "else": {
                        "id": 12,
                        "kind": "Block",
                        "stmts": [
                            {
                                "id": 19,
                                "kind": "ReturnStmt",
                                "value": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "1"
                                }
                            }
                        ]
                    },
                    "id": 14,
                    "kind": "IfElseStmt",
                    "then": {
                        "id": 12,
                        "kind": "Block",
                        "stmts": [
                            {
                                "id": 19,
                                "kind": "ReturnStmt",
                                "value": {
                                    "id": 27,
                                    "kind": "DecLiteral",
                                    "value": "0"
                                }
                            }
                        ]
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");

}

TEST(Grammar, Params) {
    const char *VarDeclare = "int main(int a, int b, int c) {\n"
                             "  return a + b;\n"
                             "}";
    CHECK_OR_DUMP_JSON(VarDeclare, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "id": 19,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 23,
                        "kind": "BinExp",
                        "left": {
                            "id": 25,
                            "kind": "RVal",
                            "name": "a"
                        },
                        "op": "+",
                        "right": {
                            "id": 25,
                            "kind": "RVal",
                            "name": "b"
                        }
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": [
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "a",
                    "type": "int"
                },
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "b",
                    "type": "int"
                },
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "c",
                    "type": "int"
                }
            ],
            "type": "int"
        }
    ]
}
)");

    const char *VarDeclare1 = "int main(int a, int b, int c, int d) {\n"
                              "  return 0;"
                             "}";
    CHECK_OR_DUMP_JSON(VarDeclare1, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "id": 19,
                    "kind": "ReturnStmt",
                    "value": {
                        "id": 27,
                        "kind": "DecLiteral",
                        "value": "0"
                    }
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": [
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "a",
                    "type": "int"
                },
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "b",
                    "type": "int"
                },
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "c",
                    "type": "int"
                },
                {
                    "id": 9,
                    "kind": "FuncParam",
                    "name": "d",
                    "type": "int"
                }
            ],
            "type": "int"
        }
    ]
}
)");

}

TEST(Grammar, While) {
    const char *Test = "int main() {"
                       "  while (true) {"
                       "    int a = 10;"
                       "  }"
                       "}";

    CHECK_OR_DUMP_JSON(Test, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "body": {
                        "id": 12,
                        "kind": "Block",
                        "stmts": [
                            {
                                "defs": [
                                    {
                                        "id": 6,
                                        "kind": "VarDef",
                                        "name": "a",
                                        "value": {
                                            "id": 27,
                                            "kind": "DecLiteral",
                                            "value": "10"
                                        }
                                    }
                                ],
                                "id": 5,
                                "kind": "VarDecl",
                                "type": "int"
                            }
                        ]
                    },
                    "cond": {
                        "id": 25,
                        "kind": "RVal",
                        "name": "true"
                    },
                    "id": 15,
                    "kind": "WhileStmt"
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");
}

TEST(Grammar, DoWhile) {
    const char *Test = "int main() {"
                       "  do {"
                       "    int a = 10;"
                       "  } while (1);"
                       "}";
    CHECK_OR_DUMP_JSON(Test, R"(
{
    "id": 1,
    "kind": "CompUnit",
    "value": [
        {
            "body": [
                {
                    "body": {
                        "id": 12,
                        "kind": "Block",
                        "stmts": [
                            {
                                "defs": [
                                    {
                                        "id": 6,
                                        "kind": "VarDef",
                                        "name": "a",
                                        "value": {
                                            "id": 27,
                                            "kind": "DecLiteral",
                                            "value": "10"
                                        }
                                    }
                                ],
                                "id": 5,
                                "kind": "VarDecl",
                                "type": "int"
                            }
                        ]
                    },
                    "cond": {
                        "id": 27,
                        "kind": "DecLiteral",
                        "value": "1"
                    },
                    "id": 16,
                    "kind": "DoWhileStmt"
                }
            ],
            "id": 8,
            "kind": "FuncDef",
            "name": "main",
            "params": null,
            "type": "int"
        }
    ]
}
)");
}
