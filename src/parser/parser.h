//
// Created by Alex
//

#ifndef TINYLALR_PARSER_H
#define TINYLALR_PARSER_H
#include <string>
#include <iostream>
#include <map>
#include <queue>
#include <cassert>
#include <functional>
#include <numeric>
#include <iomanip>
#define LR_ASSERT(x) assert(x)
#define LR_UNREACHED() assert(!"unreached here")
#define LR_TYPESPEC(TYPE) virtual TYPE

#define CONFLICT_NONE 0
#define CONFLICT_SHIFT_REDUCE 1
#define CONFLICT_REDUCE_REDUCE 2
#define SYMBOL_TYPE_TERMINAL 0
#define SYMBOL_TYPE_NONTERMINAL 1
#define TRANSITION_SHIFT 1
#define TRANSITION_REDUCE 2

enum ActionOpcode {
    OpcodeCreateObj,
    OpcodeCreateArr,
    OpcodePushValue, /// $n
    OpcodePushInt, /// int
    OpcodePushStr, /// str
    OpcodePushBool, /// bool
    OpcodePushToken, /// token
    OpcodePopSet,
    OpcodePopInsertArr,
    OpcodePopInsertObj,
};
struct ParserSymbol {
    int type;
    int symbol;
    const char *text;
};
struct LexerState;
struct LexerTransition {
    int begin;
    int end;
    LexerState *state;
};
struct LexerState {
    LexerTransition *transitions;
    int transition_count;
    int symbol;
    inline LexerTransition *begin() { return transitions; }
    inline LexerTransition *end() { return transitions + transition_count; }
    inline LexerTransition *find(int chr) {
        LexerTransition trans{0, chr};
        if (auto *iter = std::upper_bound(begin(), end(), trans, [](const LexerTransition &LHS, const LexerTransition &RHS) {
            return LHS.end < RHS.end;
        })) {
            if (iter != end() && iter->begin <= chr && chr < iter->end) {
                return iter;
            }
        }
        return nullptr;
    }
};

struct ReduceAction {
    int opcode;
    int index;
    const char *value;
};

struct ParserState;
struct ParserTransition {
    int type;
    int symbol;
    ParserState *state;
    int reduce_symbol;
    int reduce_length;
    int precedence;
    ReduceAction *actions;
    int action_count;
    inline bool accept() const { return reduce_symbol == 0 && type == TRANSITION_REDUCE; }
    inline bool error() const { return symbol == 2 && type == TRANSITION_SHIFT; }
};
struct ParserState {
    int index;
    ParserTransition *transitions;
    int transition_count;
    int conflict;
    ParserTransition *error;
    inline ParserTransition *begin() { return transitions; }
    inline ParserTransition *end() { return transitions + transition_count; }
    inline ParserTransition *find(int symbol) {
        ParserTransition trans{0, symbol};
        return std::lower_bound(begin(), end(), trans, [](const ParserTransition &LHS, const ParserTransition &RHS) {
            return LHS.symbol < RHS.symbol;
        });
    }
};

extern int LexerWhitespaceSymbol;
extern ParserSymbol ParserSymbols[];
extern LexerState LexerStates[];
extern LexerTransition LexerTransitions[];
extern ParserState ParserStates[];
extern ReduceAction ParserActions[];
extern ParserTransition ParserTransitions[];

struct Location {
    int line_start = 0;
    int line_end = 0;
    int column_start = 0;
    int column_end = 0;
    bool empty() const { return line_start == line_end && column_start == column_end; }
    Location merge(const Location &rhs) const {
        if (rhs.empty()) {
            return *this;
        }
        if (empty()) {
            return rhs;
        }
        return {
                std::min(line_start, rhs.line_start),
                std::max(line_end, rhs.line_end),
                std::min(column_start, rhs.column_start),
                std::max(column_end, rhs.column_end)
        };
    }
};
template <class iter_t = const char *,
        class char_t = typename std::iterator_traits<iter_t>::value_type,
        class char_traits = std::char_traits<char_t>>
class ParserLexer {
public:
    using string_t = std::basic_string<char_t, char_traits>;
private:
    LexerState *lexer_state /* = &LexerStates[0]*/;
    int whitespace /*= LexerWhitespaceSymbol*/;
    iter_t current;
    iter_t end;
    int line_ = 0;
    int line_start_position_ = 0;
    int position_ = 0;
    int token_line_start_ = 0;
    int token_column_start_ = 0;
    int token_symbol = 0;
    string_t lexeme_;
private:
    auto advance_symbol() {
        LexerState *state = lexer_state;
        lexeme_.clear();
        do {
            if (current == end) {
                return state->symbol;
            }
            if (auto *trans = state->find(*current)) {
                lexeme_ += *current;
                state = trans->state;
                ++position_;
                if (*current == '\n') {
                    ++line_;
                    line_start_position_ = position_;
                }
                ++current;
            } else {
                break;
            }
        } while (true);
        if (state == lexer_state && *current != '\0') {
            std::cout << "Unexpect char: " << *current << " line:" << line_end() << std::endl;
            ++current;
            return 2; // error symbol
        }
        return state->symbol;
    }
public:
    ParserLexer(LexerState *state, int whitespace = -1) : lexer_state(state), whitespace(whitespace) {}
    void reset(iter_t first, iter_t last) {
        current = first;
        end = last;
    }
    void advance() {
        do {
            token_line_start_ = line_end();
            token_column_start_ = column_end();
            token_symbol = advance_symbol();
        } while (token_symbol == whitespace);
    }
    int symbol() { return token_symbol; }
    inline int line_start() const { return token_line_start_; }
    inline int line_end() const { return line_; }
    inline int column_start() const { return token_column_start_; }
    inline int column_end() const { return position_ - line_start_position_; }
    inline Location location() const { return {line_start(), line_end(), column_start(), column_end()}; }
    inline string_t &lexeme() { return lexeme_; }
    void dump() {
        do {
            advance();
            if (token_symbol == 0) {
                std::cout << "EOF" << std::endl;
                break;
            }
            std::cout << std::setw(10) << lexeme_ << "\t" << std::setw(3) << token_symbol
                      << "[" << (line_start() + 1) << ", " << (column_start() + 1) << "]" << std::endl;
        } while (symbol() != 0);
        exit(0);
    }
};

extern int ParserMergeCreate;
extern int ParserMergeCreateCount;
extern int ParserMergeInsert;
extern int ParserMergeInsertCount;

#include "json.hpp"
using value_t = nlohmann::json;

enum {
    TYPE_NONE = 0,
    TYPE_ACCESS = 22,
    TYPE_ASSIGNSTMT = 10,
    TYPE_BINEXP = 23,
    TYPE_BLOCK = 12,
    TYPE_BREAKSTMT = 17,
    TYPE_COMPUNIT = 1,
    TYPE_CONSTDECL = 2,
    TYPE_CONSTDEF = 3,
    TYPE_CONSTINITVALLIST = 4,
    TYPE_CONTINUESTMT = 18,
    TYPE_DECLITERAL = 27,
    TYPE_DOWHILESTMT = 16,
    TYPE_EMPTYSTMT = 20,
    TYPE_EXPSTMT = 11,
    TYPE_FLOATLITEAL = 29,
    TYPE_FUNCCALL = 26,
    TYPE_FUNCDEF = 8,
    TYPE_FUNCPARAM = 9,
    TYPE_HEXFLOATLITEAL = 30,
    TYPE_HEXLITERAL = 28,
    TYPE_IFELSESTMT = 14,
    TYPE_IFSTMT = 13,
    TYPE_INITVALLIST = 7,
    TYPE_LVAL = 21,
    TYPE_RVAL = 25,
    TYPE_RETURNSTMT = 19,
    TYPE_UNAEXP = 24,
    TYPE_VARDECL = 5,
    TYPE_VARDEF = 6,
    TYPE_WHILESTMT = 15,
};
class JsonASTBase {
protected:
    value_t &value_;
public:
    using float_t = value_t::number_float_t;
    using int_t = value_t::number_integer_t;
    using uint_t = value_t::number_unsigned_t;
    using string_t = value_t::string_t;
    using iter_t = value_t::iterator;
    using const_iter_t = value_t::const_iterator;
    using size_t = value_t::size_type;
    JsonASTBase(value_t &value) : value_(value) {}
    int getID() { return value_["id"].get<int>(); }
    string_t &getKind() { return value_["kind"].get_ref<string_t &>(); }
    Location getLocation() {
        if (value_.contains("position")) {
            return {value_["position"]["lineStart"].get<int>(), value_["position"]["lineEnd"].get<int>(),
                    value_["position"]["columnStart"].get<int>(), value_["position"]["columnEnd"].get<int>()};
        }
        return {0, 0, 0, 0};
    }
    operator value_t &() { return value_; }
};
class Access : public JsonASTBase {
public:
    Access(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_ACCESS); }
    value_t &getIndex() { return value_["index"]; }
    value_t &getName() { return value_["name"]; }
};
class AssignStmt : public JsonASTBase {
public:
    AssignStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_ASSIGNSTMT); }
    value_t &getLval() { return value_["lval"]; }
    value_t &getValue() { return value_["value"]; }
};
class BinExp : public JsonASTBase {
public:
    BinExp(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_BINEXP); }
    value_t &getLeft() { return value_["left"]; }
    string_t &getOp() { return value_["op"].get_ref<string_t &>(); }
    value_t &getRight() { return value_["right"]; }
};
class Block : public JsonASTBase {
public:
    Block(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_BLOCK); }
    value_t &getStmts() { return value_["stmts"]; }
};
class BreakStmt : public JsonASTBase {
public:
    BreakStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_BREAKSTMT); }
};
class CompUnit : public JsonASTBase {
public:
    CompUnit(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_COMPUNIT); }
    value_t &getValue() { return value_["value"]; }
};
class ConstDecl : public JsonASTBase {
public:
    ConstDecl(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_CONSTDECL); }
    value_t &getDefs() { return value_["defs"]; }
    string_t &getType() { return value_["type"].get_ref<string_t &>(); }
};
class ConstDef : public JsonASTBase {
public:
    ConstDef(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_CONSTDEF); }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
    value_t &getValue() { return value_["value"]; }
};
class ConstInitValList : public JsonASTBase {
public:
    ConstInitValList(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_CONSTINITVALLIST); }
    value_t &getValue() { return value_["value"]; }
};
class ContinueStmt : public JsonASTBase {
public:
    ContinueStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_CONTINUESTMT); }
};
class DecLiteral : public JsonASTBase {
public:
    DecLiteral(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_DECLITERAL); }
    string_t &getValue() { return value_["value"].get_ref<string_t &>(); }
};
class DoWhileStmt : public JsonASTBase {
public:
    DoWhileStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_DOWHILESTMT); }
    value_t &getBody() { return value_["body"]; }
    value_t &getCond() { return value_["cond"]; }
};
class EmptyStmt : public JsonASTBase {
public:
    EmptyStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_EMPTYSTMT); }
};
class ExpStmt : public JsonASTBase {
public:
    ExpStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_EXPSTMT); }
    value_t &getValue() { return value_["value"]; }
};
class FloatLiteal : public JsonASTBase {
public:
    FloatLiteal(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_FLOATLITEAL); }
    string_t &getValue() { return value_["value"].get_ref<string_t &>(); }
};
class FuncCall : public JsonASTBase {
public:
    FuncCall(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_FUNCCALL); }
    value_t &getArgs() { return value_["args"]; }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
};
class FuncDef : public JsonASTBase {
public:
    FuncDef(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_FUNCDEF); }
    value_t &getBody() { return value_["body"]; }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
    value_t &getParams() { return value_["params"]; }
    string_t &getType() { return value_["type"].get_ref<string_t &>(); }
};
class FuncParam : public JsonASTBase {
public:
    FuncParam(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_FUNCPARAM); }
    value_t &getBound() { return value_["bound"]; }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
    string_t &getType() { return value_["type"].get_ref<string_t &>(); }
};
class HexFloatLiteal : public JsonASTBase {
public:
    HexFloatLiteal(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_HEXFLOATLITEAL); }
    value_t &getValue() { return value_["value"]; }
};
class HexLiteral : public JsonASTBase {
public:
    HexLiteral(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_HEXLITERAL); }
    string_t &getValue() { return value_["value"].get_ref<string_t &>(); }
};
class IfElseStmt : public JsonASTBase {
public:
    IfElseStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_IFELSESTMT); }
    value_t &getCond() { return value_["cond"]; }
    value_t &getElse() { return value_["else"]; }
    value_t &getThen() { return value_["then"]; }
};
class IfStmt : public JsonASTBase {
public:
    IfStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_IFSTMT); }
    value_t &getCond() { return value_["cond"]; }
    value_t &getThen() { return value_["then"]; }
};
class InitValList : public JsonASTBase {
public:
    InitValList(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_INITVALLIST); }
    value_t &getValue() { return value_["value"]; }
};
class LVal : public JsonASTBase {
public:
    LVal(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_LVAL); }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
};
class RVal : public JsonASTBase {
public:
    RVal(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_RVAL); }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
};
class ReturnStmt : public JsonASTBase {
public:
    ReturnStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_RETURNSTMT); }
    value_t &getValue() { return value_["value"]; }
};
class UnaExp : public JsonASTBase {
public:
    UnaExp(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_UNAEXP); }
    string_t &getOp() { return value_["op"].get_ref<string_t &>(); }
    value_t &getVal() { return value_["val"]; }
};
class VarDecl : public JsonASTBase {
public:
    VarDecl(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_VARDECL); }
    value_t &getDefs() { return value_["defs"]; }
    string_t &getType() { return value_["type"].get_ref<string_t &>(); }
};
class VarDef : public JsonASTBase {
public:
    VarDef(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_VARDEF); }
    value_t &getBound() { return value_["bound"]; }
    string_t &getName() { return value_["name"].get_ref<string_t &>(); }
    value_t &getValue() { return value_["value"]; }
};
class WhileStmt : public JsonASTBase {
public:
    WhileStmt(value_t &value) : JsonASTBase(value) { LR_ASSERT(value["id"] == TYPE_WHILESTMT); }
    value_t &getBody() { return value_["body"]; }
    value_t &getCond() { return value_["cond"]; }
};
template<typename SubTy, typename RetTy = void, typename...Args>
struct Visitor {
    RetTy visit(value_t &value, Args &&...args) {
        if (value.is_null()) {
            return RetTy();
        }
        if (value.is_array()) {
            for (auto &val : value) {
                visit(val, std::forward<Args>(args)...);
            }
            return RetTy();
        }
        switch (value["id"].get<int>()) {
            case TYPE_ACCESS:
                return static_cast<SubTy *>(this)->visitAccess(value, std::forward<Args>(args)...);
            case TYPE_ASSIGNSTMT:
                return static_cast<SubTy *>(this)->visitAssignStmt(value, std::forward<Args>(args)...);
            case TYPE_BINEXP:
                return static_cast<SubTy *>(this)->visitBinExp(value, std::forward<Args>(args)...);
            case TYPE_BLOCK:
                return static_cast<SubTy *>(this)->visitBlock(value, std::forward<Args>(args)...);
            case TYPE_BREAKSTMT:
                return static_cast<SubTy *>(this)->visitBreakStmt(value, std::forward<Args>(args)...);
            case TYPE_COMPUNIT:
                return static_cast<SubTy *>(this)->visitCompUnit(value, std::forward<Args>(args)...);
            case TYPE_CONSTDECL:
                return static_cast<SubTy *>(this)->visitConstDecl(value, std::forward<Args>(args)...);
            case TYPE_CONSTDEF:
                return static_cast<SubTy *>(this)->visitConstDef(value, std::forward<Args>(args)...);
            case TYPE_CONSTINITVALLIST:
                return static_cast<SubTy *>(this)->visitConstInitValList(value, std::forward<Args>(args)...);
            case TYPE_CONTINUESTMT:
                return static_cast<SubTy *>(this)->visitContinueStmt(value, std::forward<Args>(args)...);
            case TYPE_DECLITERAL:
                return static_cast<SubTy *>(this)->visitDecLiteral(value, std::forward<Args>(args)...);
            case TYPE_DOWHILESTMT:
                return static_cast<SubTy *>(this)->visitDoWhileStmt(value, std::forward<Args>(args)...);
            case TYPE_EMPTYSTMT:
                return static_cast<SubTy *>(this)->visitEmptyStmt(value, std::forward<Args>(args)...);
            case TYPE_EXPSTMT:
                return static_cast<SubTy *>(this)->visitExpStmt(value, std::forward<Args>(args)...);
            case TYPE_FLOATLITEAL:
                return static_cast<SubTy *>(this)->visitFloatLiteal(value, std::forward<Args>(args)...);
            case TYPE_FUNCCALL:
                return static_cast<SubTy *>(this)->visitFuncCall(value, std::forward<Args>(args)...);
            case TYPE_FUNCDEF:
                return static_cast<SubTy *>(this)->visitFuncDef(value, std::forward<Args>(args)...);
            case TYPE_FUNCPARAM:
                return static_cast<SubTy *>(this)->visitFuncParam(value, std::forward<Args>(args)...);
            case TYPE_HEXFLOATLITEAL:
                return static_cast<SubTy *>(this)->visitHexFloatLiteal(value, std::forward<Args>(args)...);
            case TYPE_HEXLITERAL:
                return static_cast<SubTy *>(this)->visitHexLiteral(value, std::forward<Args>(args)...);
            case TYPE_IFELSESTMT:
                return static_cast<SubTy *>(this)->visitIfElseStmt(value, std::forward<Args>(args)...);
            case TYPE_IFSTMT:
                return static_cast<SubTy *>(this)->visitIfStmt(value, std::forward<Args>(args)...);
            case TYPE_INITVALLIST:
                return static_cast<SubTy *>(this)->visitInitValList(value, std::forward<Args>(args)...);
            case TYPE_LVAL:
                return static_cast<SubTy *>(this)->visitLVal(value, std::forward<Args>(args)...);
            case TYPE_RVAL:
                return static_cast<SubTy *>(this)->visitRVal(value, std::forward<Args>(args)...);
            case TYPE_RETURNSTMT:
                return static_cast<SubTy *>(this)->visitReturnStmt(value, std::forward<Args>(args)...);
            case TYPE_UNAEXP:
                return static_cast<SubTy *>(this)->visitUnaExp(value, std::forward<Args>(args)...);
            case TYPE_VARDECL:
                return static_cast<SubTy *>(this)->visitVarDecl(value, std::forward<Args>(args)...);
            case TYPE_VARDEF:
                return static_cast<SubTy *>(this)->visitVarDef(value, std::forward<Args>(args)...);
            case TYPE_WHILESTMT:
                return static_cast<SubTy *>(this)->visitWhileStmt(value, std::forward<Args>(args)...);
            default:
                LR_UNREACHED();
        }
    }
    LR_TYPESPEC(RetTy) visitAccess(Access value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitAssignStmt(AssignStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitBinExp(BinExp value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitBlock(Block value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitBreakStmt(BreakStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitCompUnit(CompUnit value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitConstDecl(ConstDecl value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitConstDef(ConstDef value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitConstInitValList(ConstInitValList value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitContinueStmt(ContinueStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitDecLiteral(DecLiteral value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitDoWhileStmt(DoWhileStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitEmptyStmt(EmptyStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitExpStmt(ExpStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitFloatLiteal(FloatLiteal value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitFuncCall(FuncCall value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitFuncDef(FuncDef value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitFuncParam(FuncParam value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitHexFloatLiteal(HexFloatLiteal value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitHexLiteral(HexLiteral value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitIfElseStmt(IfElseStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitIfStmt(IfStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitInitValList(InitValList value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitLVal(LVal value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitRVal(RVal value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitReturnStmt(ReturnStmt value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitUnaExp(UnaExp value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitVarDecl(VarDecl value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitVarDef(VarDef value, Args &&...args) {
        return RetTy();
    }
    LR_TYPESPEC(RetTy) visitWhileStmt(WhileStmt value, Args &&...args) {
        return RetTy();
    }
};

template<bool Move = true, typename NodeGetter>
inline void HandleReduceAction(ReduceAction &action, std::vector<value_t> &arr, NodeGetter nodes) {
    switch (action.opcode) {
        default:
            LR_UNREACHED();
        case OpcodeCreateObj:
            if (action.index)
                arr.push_back(value_t::object_t(
                        {{"kind", value_t::string_t(action.value)},
                         {"id",   value_t::number_integer_t(action.index)}}));
            else
                arr.push_back(value_t::object_t());
            break;
        case OpcodeCreateArr:
            arr.push_back(value_t::array_t());
            break;
        case OpcodePushValue:
            arr.push_back(Move ? std::move(nodes[action.index].value) : nodes[action.index].value);
            break;
        case OpcodePushInt:
            arr.push_back(value_t::number_integer_t(action.index));
            break;
        case OpcodePushStr:
            arr.push_back(value_t::string_t(action.value));
            break;
        case OpcodePushBool:
            arr.push_back(value_t::boolean_t(action.index));
            break;
        case OpcodePushToken:
            arr.push_back(value_t::string_t(nodes[action.index].lexeme));
            break;
        case OpcodePopSet: {
            auto poped = std::move(arr.back());
            arr.pop_back();
            auto &top = arr.back();
            top[std::string(action.value)] = std::move(poped);
            break;
        }
        case OpcodePopInsertArr: {
            auto poped = std::move(arr.back());
            arr.pop_back();
            auto &top = arr.back();
            top.insert(top.end(), std::move(poped));
            break;
        }
        case OpcodePopInsertObj: {
            auto poped = std::move(arr.back());
            arr.pop_back();
            auto &top = arr.back()[std::string(action.value)];
            top.insert(top.end(), std::move(poped));
            break;
        }
    }
}

template<class iter_t = const char *,
        class char_t = typename std::iterator_traits<iter_t>::value_type,
        class char_traits = std::char_traits<char_t>>
class LRParser {
public:
    using Lexer = ParserLexer<iter_t>;
    using string_t = typename Lexer::string_t;
private:
    struct ParserNode {
        ParserState *state;
        int symbol = 0;
        value_t value;
        string_t lexeme;
        Location location;
        ParserNode(ParserState *state) : state(state) {}
        ParserNode(ParserState *state, int symbol, const value_t &value, Location loc) : state(state), symbol(symbol),
                                                                                         value(value), location(loc) {
#ifdef DEBUG
            lexeme = ParserSymbols[symbol].text;
#endif
        }
        ParserNode(ParserState *state, int symbol, const string_t &lexeme, Location loc) : state(state), symbol(symbol),
                                                                                           lexeme(lexeme), location(loc) {}
    };

    using Node = ParserNode;
public:
    ParserState *parser_state = &ParserStates[0];
    Lexer parser_lexer = Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    bool position = false;
    bool accepted = false;
    inline ParserTransition *find_trans(ParserState *state, int symbol) {
        auto *trans = state->find(symbol);
        return trans == state->end() ? nullptr : trans;
    }
public:
    std::vector<Node> stack;
    std::vector<value_t> values;
    LRParser() = default;
    explicit LRParser(bool position) : position(position) {}
    void reset(iter_t first, iter_t last = iter_t()) {
        parser_lexer.reset(first, last);
        accepted = false;
    }
    inline bool accept() const {
        return accepted;
    }
    void parse() {
        parser_lexer.advance();
        stack.reserve(32);
        stack.push_back(Node(parser_state));
        do {
            if (auto *trans = find_trans(stack.back().state, parser_lexer.symbol())) {
                if (trans->type == TRANSITION_SHIFT) { // Shift
                    shift(trans);
                } else { // Reduce
                    reduce(trans);
                    if (trans->accept()) {
                        accepted = true;
                        break;
                    }
                }
            } else {
                if (!handle_error()) {
                    break;
                }
            }
        } while (true);
    }
    value_t &value() { return stack[0].value; }

    inline void shift(ParserTransition *trans) {
        // debug_shift(trans);
        stack.emplace_back(trans->state, parser_lexer.symbol(), parser_lexer.lexeme(), parser_lexer.location());
        parser_lexer.advance();
    }
    inline void reduce(ParserTransition *trans) {
        values.clear();
        // if the reduce length is zero, the location is the lexer current location.
        Location loc = parser_lexer.location();
        value_t value;
        if (trans->reduce_length) {
            auto first = stack.size() - trans->reduce_length;
            // merge the locations
            loc = std::accumulate(stack.begin() + first, stack.end(), Location(), [](Location i, Node &node) {
                return i.merge(node.location);
            });
            // handle `reduce` action
            handle_action(trans->actions, trans->action_count, &stack[first]);
            // record the position
            value_t &reduce_value = values.back();
            if (position && reduce_value.is_object()) {
                reduce_value["position"] = {{"lineStart",   loc.line_start},
                                            {"columnStart", loc.column_start},
                                            {"lineEnd",     loc.line_end},
                                            {"columnEnd",   loc.column_end}};
            }
            stack.erase(stack.begin() + first, stack.end());
            if (trans->accept()) {
                stack.back().value = std::move(values.back());
                return;
            }
        }
        if (!values.empty()) {
            value = std::move(values.back());
            values.pop_back();
        }
        // goto a new state by the reduced symbol
        if (auto *Goto = find_trans(stack.back().state, trans->reduce_symbol)) {
            stack.emplace_back(Goto->state, trans->reduce_symbol, std::move(value), loc);
        } else {
            expect();
        }
    }

    inline void expect() {
        Node &node = stack.back();
        std::cout << "Shift Reduce Error "
                     "line: " << node.location.line_start + 1 << " "
                  << "column: " << node.location.column_start + 1 << " "
                  << "token: " << parser_lexer.lexeme()
                  << std::endl;
        std::cout << "Expect: ";
        for (auto &trans: *node.state) {
            std::cout << "\"" << ParserSymbols[trans.symbol].text << "\"";
            if (&trans != (node.state->end() - 1)) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
    inline void debug_shift(ParserTransition *trans) {
        std::cout << "shift: " << stack.size() << " "
                  << "[state " << (stack.back().state - ParserStates) << " -> " << (trans->state - ParserStates)
                  << "]  [" << ParserSymbols[parser_lexer.symbol()].text << "] "
                  << parser_lexer.lexeme()
                  << std::endl << std::endl;
    }
    inline void debug_reduce(ParserTransition *trans, int start) {
        ParserTransition *goto_trans = find_trans(stack[start - 1].state, trans->reduce_symbol);
        if (!goto_trans) {
            return;
        }
        std::cout << "reduce: "
                  << start << " "
                  << "[back to " /*<< (stack.back().state - ParserStates) << " -> "*/
                  << (stack[start - 1].state - ParserStates) << " -> "
                  << (goto_trans->state - ParserStates) << "] "
                  << ParserSymbols[trans->reduce_symbol].text << " <- ";
        for (int i = start; i < start + trans->reduce_length; ++i) {
            std::cout << "[" << stack[i].lexeme << "] " << stack[i].value << " | ";
        }
        std::cout << std::endl << std::endl;
    }

    inline void handle_action(ReduceAction *actions, int action_count, Node *nodes) {
        if (action_count == 0) {
            values.push_back(std::move(nodes->value));
            return;
        }

        struct Getter {
            Node *nodes;
            Getter(Node *nodes) : nodes(nodes) {}
            inline Node &operator[](size_t index) {
                return nodes[index];
            }
        } getter{nodes};

        for (int i = 0; i < action_count; ++i) {
            HandleReduceAction(actions[i], values, getter);
        }
    }
    inline bool handle_error() {
        auto *trans = find_trans(stack.back().state, 2);
        if (trans == nullptr) {
            expect();
            return false;
        }
        value_t value = value_t::array();
        ParserState *state = trans->state;
        do {
            value.push_back({{"lexeme",      parser_lexer.lexeme()},
                             {"symbol",      parser_lexer.symbol()},
                             {"lineStart",   parser_lexer.line_start()},
                             {"columnStart", parser_lexer.column_start()},
                             {"lineEnd",     parser_lexer.line_end()},
                             {"columnEnd",   parser_lexer.column_end()}});
            parser_lexer.advance();
            if (ParserTransition *Goto = find_trans(state, parser_lexer.symbol())) {
                stack.emplace_back(trans->state, 2, std::move(value), parser_lexer.location());
                if (Goto->type == TRANSITION_SHIFT) {
                    shift(Goto);
                } else {
                    reduce(Goto);
                }
                break;
            }
        } while (true);
        return true;
    }
};

template <class iter_t = const char *,
        class char_t = typename std::iterator_traits<iter_t>::value_type,
        class char_traits = std::char_traits<char_t>>
class GLRParser {
    using Lexer = ParserLexer<iter_t>;
    using string_t = typename Lexer::string_t;
    struct ParserGraphNode {
        using NodePtr = std::shared_ptr<ParserGraphNode>;
        static auto Create(ParserState *state) {
            return std::make_shared<ParserGraphNode>(state);
        }
        static auto Create(ParserState *state, const NodePtr &prev) {
            return std::make_shared<ParserGraphNode>(state, prev);
        }
        std::vector<NodePtr> prevs;
        ParserState *state = nullptr;
        int symbol = 0;
        value_t value;
        string_t lexeme;
        Location location;
        int depth = 0;
        int merge = 0;
        bool error = false;
        ParserGraphNode() {}
        ParserGraphNode(ParserState *state) : state(state) {}
        ParserGraphNode(ParserState *state, const NodePtr &prev) : state(state), prevs({prev}) {}
        void add_prev(const NodePtr &previous) {
            prevs.push_back(previous);
        }
        bool need_lr_reduce(ParserTransition *trans) {
            return state->conflict == CONFLICT_NONE && trans->reduce_length <= depth;
        }
    };
private:
    using Node = ParserGraphNode;
    using NodePtr = std::shared_ptr<Node>;
    struct ReduceNode {
        std::vector<NodePtr> paths;
        ParserTransition *trans;
        NodePtr prev;
        ReduceNode(const std::vector<NodePtr> &paths, ParserTransition *trans, const NodePtr &prev) : paths(paths.rbegin(), paths.rend()),
                                                                                trans(trans), prev(prev) {}
        inline bool operator<(const ReduceNode &rhs) const {
            return trans->precedence < rhs.trans->precedence;
        }
        inline NodePtr get_last() const {
            return paths.back();
        }
    };
    ParserState *parser_state = &ParserStates[0];
    Lexer lexer_ = Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    bool position = false;
    bool accepted = false;
    std::map<ParserState *, NodePtr> frontier;
    std::vector<NodePtr> shift_list;
    std::vector<value_t> values;
    std::priority_queue<ReduceNode> reduce_list;
public:
    GLRParser() = default;
    explicit GLRParser(bool position) : position(position) {}
    explicit GLRParser(iter_t first, iter_t last = iter_t()) {
        reset(first, last);
    }
    void set_position(bool sp) {
        position = sp;
    }
    void reset(iter_t first, iter_t last = iter_t()) {
        accepted = false;
        frontier.clear();
        lexer_.reset(first, last);
    }
    void parse() {
        frontier.insert(std::pair(parser_state, Node::Create(parser_state)));
        lexer_.advance();
        do {
            shift();
            reduce();
            if (frontier.size() == 0 || accepted) {
                break;
            }
        } while (true);
    }
    bool accept() {
        return accepted && frontier.size() > 0;
    }
    value_t &value() {
        return std::get<1>(*frontier.begin())->value;
    }

    void shift() {
        shift_list.clear();
        for (auto &[state, node] : frontier) {
            int shift_count = 0;
            for (auto *trans = state->find(lexer_.symbol()); trans < state->end(); trans++) {
                if (trans->symbol != lexer_.symbol()) {
                    break;
                }
                if (trans->type == TRANSITION_SHIFT) {
                    NodePtr shift_node = Node::Create(trans->state, node);
                    shift_node->symbol = lexer_.symbol();
                    shift_node->lexeme = lexer_.lexeme();
                    shift_node->location = lexer_.location();
                    shift_node->depth = node->depth + 1;
                    shift_list.push_back(shift_node);
                    shift_count++;
                }
            }
            if (state->error && shift_count == 0 || node->error) {
                do_error(node, state->error);
            }
        }
        if (shift_list.size() == 0) {
            handle_error();
        }
        frontier.clear();
        for (auto &node : shift_list) {
            if (frontier.count(node->state)) {
                // merge the previous nodes if they are the same state
                auto &prevs = frontier[node->state]->prevs;
                prevs.insert(prevs.end(), node->prevs.begin(), node->prevs.end());
                frontier[node->state]->depth = 0;
            } else {
                frontier.insert(std::pair(node->state, node));
            }
        }
        lexer_.advance();
    }
    void reduce() {
        for (auto &[state, node] : frontier) {
            do_reduce(node);
        }
        while (!reduce_list.empty()) {
            ReduceNode node = std::move(reduce_list.top());
            reduce_list.pop();
            do_goto(node);
        }
    }

    void do_goto(ReduceNode &node) {
        values.clear();
        // handle `reduce` action
        if (!node.paths.empty() && node.get_last()->need_lr_reduce(node.trans)) {
            handle_action<true>(node, node.trans->actions, node.trans->action_count);
            frontier.erase(node.get_last()->state);
            // del(node)
        } else {
            handle_action<false>(node, node.trans->actions, node.trans->action_count);
            // dup(node)
        }
        // record location
        Location loc = lexer_.location();
        value_t value;
        if (!values.empty()) {
            value = std::move(values.back());
            values.pop_back();
        }
        if (node.trans->reduce_length) {
            // merge the locations
            loc = std::accumulate(node.paths.begin(), node.paths.end(), Location(), [](Location loc, NodePtr &node) {
                return loc.merge(node->location);
            });
            if (position && value.is_object()) {
                value["position"] = {{"lineStart",   loc.line_start},
                                     {"columnStart", loc.column_start},
                                     {"lineEnd",     loc.line_end},
                                     {"columnEnd",   loc.column_end}};
            }
        }
        if (node.trans->accept()) {
            auto start = Node::Create(parser_state);
            start->value = std::move(value);
            frontier.insert(std::pair(parser_state, start));
            accepted = true;
            return;
        }

        for (auto *trans = node.prev->state->find(node.trans->reduce_symbol); trans < node.prev->state->end(); trans++) {
            if (trans->symbol != node.trans->reduce_symbol) {
                break;
            }
            if (trans->type == TRANSITION_SHIFT) {
                if (frontier.count(trans->state)) {
                    do_merge(frontier[trans->state], value);
                    frontier[trans->state]->add_prev(node.prev);
                    frontier[trans->state]->depth = 0;
                } else {
                    NodePtr shift = Node::Create(trans->state, node.prev);
                    shift->symbol = node.trans->reduce_symbol;
                    shift->value = std::move(value);
                    shift->lexeme = ParserSymbols[node.trans->reduce_symbol].text;
                    shift->location = loc;
                    shift->depth = node.prev->depth + 1;
                    do_reduce(shift);
                    frontier.insert({shift->state, shift});
                }
            }
        }
    }
    void do_reduce(NodePtr &node) {
        for (auto *trans = node->state->find(lexer_.symbol()); trans < node->state->end(); trans++) {
            if (trans->symbol != lexer_.symbol()) {
                break;
            }
            if (trans->type == TRANSITION_REDUCE) {
                enumerate_path(node, trans);
            }
        }
    }

    void enumerate_path(const NodePtr &start, ParserTransition *trans) {
        std::vector<NodePtr> path;
        std::function<void(const NodePtr &, unsigned)> DFS = [&](auto &node, unsigned length) {
            if (length-- == 0) {
                reduce_list.push(ReduceNode(path, trans, node));
                return;
            }
            path.push_back(node);
            for (auto &prev: node->prevs) {
                DFS(prev, length);
            }
            path.pop_back();
        };
        DFS(start, trans->reduce_length);
    }
    void do_merge(NodePtr node, value_t &value) {
        if (node->value == value) {
            return;
        }
        struct Getter {
            NodePtr node;
            Node inner;
            Getter(NodePtr node, value_t &value) : node(node) {
                inner.lexeme = node->lexeme;
                inner.value = std::move(value);
            }
            inline Node &operator[](size_t index) {
                if (index == 0) {
                    return *node;
                }
                return inner;
            }
        } getter{node, value};

        values.clear();
        if (node->merge) {
            if (ParserMergeInsertCount) {
                for (auto i = ParserMergeInsert; i < ParserMergeInsert + ParserMergeInsertCount; ++i) {
                    HandleReduceAction(ParserActions[i], values, getter);
                }
                node->value = std::move(values.back());
            } else {
                node->value["value"].push_back(std::move(value));
            }
        } else {
            if (ParserMergeCreateCount) {
                for (auto i = ParserMergeCreate; i < ParserMergeCreate + ParserMergeCreateCount; ++i) {
                    auto &Re = ParserActions[i];
                    HandleReduceAction(ParserActions[i], values, getter);
                }
                node->value = std::move(values.back());
            } else {
                value_t merge = {{"kind",  "merge"},
                                 {"value", value_t::array({std::move(node->value), value})}};
                node->value = std::move(merge);
            }
        }
        node->merge++;
    }
    void do_error(NodePtr node, ParserTransition *trans) {
        // shift error
        if (!node->error) {
            // there is error state, goto error state
            node = Node::Create(trans->state, node);
            node->symbol = trans->symbol;
            node->value = value_t::array();
            node->lexeme = ParserSymbols[node->symbol].text;
            node->error = true;
            node->depth = node->depth + 1;
        }
        node->location = node->location.merge(lexer_.location());
        node->value.push_back({{"lexeme", lexer_.lexeme()},
                               {"symbol", lexer_.symbol()},
                               {"lineStart", lexer_.line_start()},
                               {"columnStart", lexer_.column_start()},
                               {"lineEnd", lexer_.line_end()},
                               {"columnEnd", lexer_.column_end()}});
        shift_list.push_back(node);
    }

    template<bool Move = false>
    inline void handle_action(ReduceNode &node, ReduceAction *actions, int action_count) {
        if (action_count == 0) {
            if (!node.paths.empty() && Move) {
                values.push_back(node.paths[0]->value); // default action -> $1
            }
            return;
        }

        struct Getter {
            NodePtr *nodes;
            Getter(NodePtr *nodes) : nodes(nodes) {}
            inline Node &operator[](size_t index) {
                return *nodes[index];
            }
        } getter{node.paths.data()};

        for (int i = 0; i < action_count; ++i) {
            HandleReduceAction<Move>(actions[i], values, getter);
        }
    }
    inline void handle_error() {
        std::cout << "Unexpected Symbol '" << lexer_.lexeme() << "'"
                  << " line:" << lexer_.line_start() << " column:" << lexer_.column_start()
                  << std::endl;
        std::cout << "Expect: ";
        for (auto &[state, node] : frontier) {
            for (auto &trans : *state) {
                if (trans.type == TRANSITION_SHIFT && ParserSymbols[trans.symbol].type == SYMBOL_TYPE_NONTERMINAL) {
                    std::cout << "\"" << ParserSymbols[trans.symbol].text << "\"" << ", ";
                }
            }
        }
        std::cout << std::endl;
    }
};
#endif //TINYLALR_PARSER_H