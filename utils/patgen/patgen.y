%whitespace "([ \r\t\n]+)|(/\*.*\*/)|(//.*\n)";
%start Target;

Target -> 'target' Ident '{' Body '}' @Target{target: $2, body:$4} ;

Body -> Body BodyItem $1[$2]
      | BodyItem [$1]
      ;
BodyItem -> Regsiter $1
          | Instructions $1
          | Patterns $1
          ;

Regsiter -> 'register' Ident ':' Ident '{' RegisterList '}' @RegisterClass{name: $2, type: $4, lists:$6};
RegisterList -> RegisterList ',' Ident $1[$3]
              | Ident [$1]
              |
              ;

Instructions -> 'instructions' '{' InstructionList '}' @Instructions{list:$3} ;
InstructionList -> InstructionList InstructionItem $1[$2]
                 | InstructionItem [$1] ;
InstructionItem -> Arg Properties @Instruction{instrs:$1, properties:$2} ;

Patterns -> 'patterns' '{' PatternList '}' @Patterns{patterns: $3} ;
PatternList ->
        PatternList Pattern $1[$2]
        | Pattern [$1]
        ;
Pattern -> Arg '=' Arg Properties @Pattern{pattern: $1, rewriter: $3, properties: $4} ;

Rule -> Ident '(' Args ')' @Rule{name: $1, args: $3} ;

Args ->
    Arg [$1]
    | Args ',' Arg $1[$3]
    |
    ;

Arg -> VarName @ArgVar{var: $1, type: ""}
    | Ident ':' VarName @ArgVar{var: $3, type: $1}
    | Rule @ArgRule{rule: $1, type: "", name: ""}
    | Ident ':' VarName Rule @ArgRule{rule: $4, type: $1, name: $3}
    | VarName Rule @ArgRule{rule: $2, type: "", name: $1}
    | Ident ':' Rule @ArgRule{rule: $3, type: $1, name: ""}
    | Ident @ArgVar{var: "", type: $1}
    ;

Properties -> ';' []
          | '{' '}' []
          | '{' PropertyList '}' $2
          | '{' PropertyList '}' ';' $2
          ;
PropertyList -> PropertyList ',' Property $1[$2]
              | Property [$1]
              ;
Property -> Ident ':' Ident @Property{name: $1, value: $3}
          | Ident ':' Number @NumProperty{name: $1, value: $3}
          | Ident ':' String @StrProperty{name: $1, value: $3}
          ;

VarName -> "$[a-zA-Z0-9_]*" @1;
Ident -> "[a-zA-Z_][a-zA-Z0-9_]*" @1;
Number -> "[0-9]+" @1;
String -> "\".*\"" @1 | "\'.*\'" @1;
