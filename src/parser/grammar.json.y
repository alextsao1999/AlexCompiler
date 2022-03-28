%right '=' '->';
%left '||' '&&' '|' '^' '&' '==' '!=' '<' '<=' '>' '>=' '+' '-' '*' '/' '%' '!';
%right ':' '?';
%none '(' ')' ;
%right 'else'  '&';
%left '.' '[';
%whitespace "([ \r\t\n]+)|(/\*.*\*/)|(//.*\n)";
%start CompUnit;

CompUnit -> CompUnit CompUnitItem @CompUnit $1[$2]
	  | CompUnitItem [$1]
CompUnitItem -> Decl $1 | FuncDef $1;

Decl -> ConstDecl $1 | VarDecl $1;

ConstDecl -> 'const' BType ConstDefList ';' @ConstDecl{type: $2, defs: $3};

BType -> 'int' @1 | 'float' @1 | 'void' @1;

ConstDefList -> ConstDefList ',' ConstDef $1[$3]
	      | ConstDef [$1];

ConstDef -> Ident @ConstDef{name: $1}
	  | Ident '=' ConstExp @ConstDef{name: $1, value: $3};

ConstExp -> Exp $1;

ConstInitVal -> ConstExp $1
		| '{' ConstInitValList '}' @ConstInitValList{value: $2};

ConstInitValList -> ConstInitValList ',' ConstInitVal $1[$3] | ConstInitVal [$1];

VarDecl -> BType VarDefList ';' @VarDecl{type: $1, defs: $2};
VarDefList -> VarDefList ',' VarDef $1[$3] | VarDef [$1];
VarDef -> Ident @VarDef{name: $1}
        | Ident Bounds @VarDef{name: $1, bound: $2}
        | Ident '=' InitVal @VarDef{name: $1, value: $3}
        | Ident Bounds '=' InitVal @VarDef{name: $1, bound: $2, value: $4}
        ;

InitVal -> Exp $1 | '{' InitValList '}' @InitValList{value: $2};
InitValList -> InitValList ',' InitVal $1[$3] | InitVal [$1];

FuncDef -> BType Ident '(' FuncParams ')' Block @FuncDef{type: $1, name: $2, params: $4, body: $6};
FuncParams -> FuncParams ',' FuncParam $1[$3]
	    | FuncParam [$1]
	    |
	    ;

FuncParam -> BType Ident @FuncParam{type: $1, name: $2}
	   | BType Ident Bounds @FuncParam{type: $1, name: $2, bound: $3}
	   ;
Bounds -> '[' Exp ']' [$2]
	| Bounds '[' Exp ']' $1[$3]
	;


Block -> '{' BlockStmtList '}' @Block{stmts: $2};
BlockStmtList -> BlockStmtList BlockItem $1[$2] | BlockItem [$1];
BlockItem -> Stmt $1 | Decl $1;

Stmt -> LVal '=' Exp ';' @AssignStmt{lval: $1, value: $3}
      | Exp ';' @ExpStmt{value: $1}
      | Block $1
      | 'if' '(' Exp ')' Stmt @IfStmt{cond: $3, then: $5}
      | 'if' '(' Exp ')' Stmt 'else' Stmt @IfElseStmt{cond: $3, then: $5, else: $7}
      | 'while' '(' Exp ')' Stmt @WhileStmt{cond: $3, body: $5}
      | 'break' ';' @BreakStmt{}
      | 'continue' ';' @ContinueStmt{}
      | 'return' Exp ';' @ReturnStmt{value: $2}
      | 'return' ';' @ReturnStmt{}
      | ';' @EmptyStmt{}
      ;

LVal -> Ident @LVal{name: $1}
      | Ident Bounds @Access{name: $1, index: $2}
      ;

Exp -> Exp '+' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '-' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '*' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '/' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '%' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '&' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '|' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '<' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '>' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '==' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '!=' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '<=' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '>=' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '&&' Exp @BinExp{op: @2, left: $1, right: $3}
     | Exp '||' Exp @BinExp{op: @2, left: $1, right: $3}
     | '-' Exp @UnaExp{op: @2, val: $1}
     | '!' Exp @UnaExp{op: @2, val: $1}
     | '(' Exp ')' $1
     | Ident @RVal{name: $1}
     | Number $1
     ;

Number -> "[0-9]+" @DecLiteral{value: @1}
        | "0[xX][0-9a-fA-F]+" @HexLiteral{value:@1}
        | "([0-9]*\.[0-9]+|[0-9]+\.|[0-9]+)([eE](\+|\-)?[0-9]+)?[fFlL]?" @FloatLiteal{value: @1}
        | "0[xX]([0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.|[0-9a-fA-F]+)([pP](\+|\-)?[0-9]+)?[fFlL]?" @HexFloatLiteal{value: @1}
        ;

Ident -> "[a-zA-Z_][a-zA-Z0-9_]*" @1;
