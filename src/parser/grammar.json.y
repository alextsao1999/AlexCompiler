%right '=' '->';
%left '||' '&&' '|' '^' '&' '==' '!=' '<' '<=' '>' '>=' '+' '-' '*' '/' '%' '!';
%right ':' '?';
%none '(' ')' ;
%right 'else'  '&';
%left '.' '[';
%whitespace "([ \r\t\n]+)|(/\*.*\*/)|(//.*\n)";
%start CompUnit;

CompUnit -> CompUnit CompUnitItem  @CompUnit $1{value: [#2]}
	  | CompUnitItem @CompUnit{value: [$1]}
CompUnitItem -> Decl $1 | FuncDef $1;

Decl -> ConstDecl $1 | VarDecl $1;

ConstDecl -> 'const' BType ConstDefList ';' @ConstDecl{@string_t& type: $2, defs: $3};

BType -> 'int' @1 | 'float' @1 | 'void' @1;

ConstDefList -> ConstDefList ',' ConstDef $1[$3]
	      | ConstDef [$1];

ConstDef -> Ident @ConstDef{@string_t& name: $1}
	  | Ident '=' ConstExp @ConstDef{name: $1, value: $3};

ConstExp -> Exp $1;

ConstInitVal -> ConstExp $1
		| '{' ConstInitValList '}' @ConstInitValList{value: $2};

ConstInitValList -> ConstInitValList ',' ConstInitVal $1[$3] | ConstInitVal [$1];

VarDecl -> BType VarDefList ';' @VarDecl{@string_t& type: $1, defs: $2};
VarDefList -> VarDefList ',' VarDef $1[$3] | VarDef [$1];
VarDef -> Ident @VarDef{@string_t& name: $1}
        | Ident Bounds @VarDef{name: $1, bound: $2}
        | Ident '=' InitVal @VarDef{name: $1, value: $3}
        | Ident Bounds '=' InitVal @VarDef{name: $1, bound: $2, value: $4}
        ;

InitVal -> Exp $1 | '{' InitValList '}' @InitValList{value: $2};
InitValList -> InitValList ',' InitVal $1[$3] | InitVal [$1];

FuncDef -> BType Ident '(' FuncParams ')' Block @FuncDef{@string_t& type: $1, @string_t& name: $2, params: $4, body: $6};
FuncParams -> FuncParams ',' FuncParam $1[$3]
	    | FuncParam [$1]
	    |
	    ;

FuncParam -> BType Ident @FuncParam{@string_t& type: $1, @string_t& name: $2}
	   | BType Ident Bounds @FuncParam{type: $1, name: $2, bound: $3}
	   ;
Bounds -> '[' Exp ']' [$2]
	| Bounds '[' Exp ']' $1[$3]
	;


Block -> '{' BlockStmtList '}' $2;
BlockStmtList -> BlockStmtList BlockItem $1[$2] | BlockItem [$1];
BlockItem -> Stmt $1 | Decl $1;

Stmt -> LVal '=' Exp ';' @AssignStmt{lval: $1, value: $3}
      | Exp ';' @ExpStmt{value: $1}
      | Block  @Block{stmts: $1}
      | 'if' '(' Exp ')' Stmt @IfStmt{cond: $3, then: $5}
      | 'if' '(' Exp ')' Stmt 'else' Stmt @IfElseStmt{cond: $3, then: $5, else: $7}
      | 'while' '(' Exp ')' Stmt @WhileStmt{cond: $3, body: $5}
      | 'do' Stmt 'while' '(' Exp ')' ';' @DoWhileStmt{cond: $5, body: $2}
      | 'break' ';' @BreakStmt{}
      | 'continue' ';' @ContinueStmt{}
      | 'return' Exp ';' @ReturnStmt{value: $2}
      | 'return' ';' @ReturnStmt{}
      | ';' @EmptyStmt{}
      ;

LVal -> Ident @LVal{@string_t& name: $1}
      | Ident Bounds @Access{name: $1, index: $2}
      ;

Exp -> Exp '+' Exp @BinExp{@string_t& op: @2, left: $1, right: $3}
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
     | '-' Exp @UnaExp{@string_t& op: @1, val: $2}
     | '!' Exp @UnaExp{op: @1, val: $2}
     | '(' Exp ')' $1
     | Ident @RVal{@string_t& name: $1}
     | Ident '(' FuncCallArgs ')' @FuncCall{@string_t& name: $1, args: $3}
     | Number $1
     ;

FuncCallArgs -> FuncCallArgs ',' Exp $1[$3] | Exp [$1] | [];

Number -> "[0-9]+" @DecLiteral{@string_t& value: @1}
        | "0[xX][0-9a-fA-F]+" @HexLiteral{@string_t& value:@1}
        | "([0-9]*\.[0-9]+|[0-9]+\.|[0-9]+)([eE](\+|\-)?[0-9]+)?[fFlL]" @FloatLiteal{@string_t& value: @1}
        | "0[xX]([0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.|[0-9a-fA-F]+)([pP](\+|\-)?[0-9]+)?[fFlL]?" @HexFloatLiteal{value: @1}
        ;

Ident -> "[a-zA-Z_][a-zA-Z0-9_]*" @1;
