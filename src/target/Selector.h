//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_SELECTOR_H
#define DRAGON_SELECTOR_H

#include "Type.h"
#include "PatternNode.h"
#include "MachineInstr.h"
#include "MachineBlock.h"
#include <functional>
namespace Matcher {
    template<bool Cond, typename Then, typename Else>
    using if_v = typename std::conditional_t<Cond, Then, Else>;

    template<typename ...T> struct set {
        constexpr static std::size_t size = sizeof ...(T);
        constexpr static bool empty = size == 0;
    };

    //// index_of
    template<typename S, typename E, int Index = 0> struct index_of {
        constexpr static int value = -1;
    };
    template<typename T, typename ...Ts, typename E, int Index> struct index_of<set<T, Ts...>, E , Index> {
        constexpr static int value = std::is_same_v<E, T> ? Index : index_of<set<Ts...>, E, Index + 1>::value;
    };

    //// contains
    template<typename S, typename E> struct contains_t {
        constexpr static bool value = false;
    };
    template<typename ...T, typename E> struct contains_t<set<T...>, E> {
        constexpr static bool value = (std::is_same<E, T>::value || ... || false);
    };
    template<typename S, typename E> constexpr bool contains = contains_t<S, E>::value;

    //// append
    template<typename S, typename E, bool = contains_t<S, E>::value> struct add_t {
        using type = S;
    };
    template<typename ...Ts, typename E>  struct add_t<set<Ts...>, E, false> {
        using type = set<Ts..., E>;
    };
    template<typename S, typename E>
    using add_element = typename add_t<S, E>::type;

    //// prepend
    template<typename S, typename E> struct prepend_t {};
    template<typename ...Ts, typename E>  struct prepend_t<set<Ts...>, E> {
        using type = set<E, Ts...>;
    };
    template<typename S, typename E>
    using prepend = typename prepend_t<S, E>::type;

    //// remove
    template<typename S, typename E> struct remove_t {
        using type = S;
    };
    template<typename S, typename E>
    using remove_element = typename remove_t<S, E>::type;
    template<typename T, typename ...Ts, typename E> struct remove_t<set<T, Ts...>, E> {
        using type = prepend<remove_element<set<Ts...>, E>, T>;
    };
    template<typename E> struct remove_t<set<>, E> {
        using type = set<>;
    };
    template<typename T, typename ...Ts> struct remove_t<set<T, Ts...>, T> {
        using type = set<Ts...>;
    };

    //// join
    template<typename S1, typename S2> struct join_t {};
    template<typename S1, typename S2> using join = typename join_t<S1, S2>::type;
    template<typename S, typename T, typename ...Ts>
    struct join_t<S, set<T, Ts...>>{
        using type = join<add_element<S, T>, set<Ts...>>;
    };
    template<typename S>
    struct join_t<S, set<>>{
        using type = S;
    };
    template<typename S1, typename S2>
    using join = typename join_t<S1, S2>::type;

    //// get_element
    template <typename S, size_t Index> struct get_element_t;
    template<typename S, size_t Index>
    using get_element = typename get_element_t<S, Index>::type;
    template <typename N, typename ...T, size_t Index> struct get_element_t<set<N, T...>, Index> {
        using type = get_element<set<T...>, Index - 1>;
    };
    template <typename N, typename ...T> struct get_element_t<set<N, T...>, 0> {
        using type = N;
    };

    //// set_element
    template<typename S, size_t Index, typename E> struct set_element_t {
        using type = S;
    };
    template<typename S, size_t Index, typename E>
    using set_element = typename set_element_t<S, Index, E>::type;
    template <typename N, typename ...Ts, size_t Index, typename E> struct set_element_t<set<N, Ts...>, Index, E> {
        using type = prepend<set_element<set<Ts...>, Index - 1, E>, N>;
    };
    template <typename N, typename ...Ts, typename E> struct set_element_t<set<N, Ts...>, 0, E> {
        using type = set<E, Ts...>;
    };
    /*template <typename N, typename E> struct set_element_t<set<N>, 0, E> {
        using type = set<N>;
    };*/

    /// map
    template<typename S, typename F> struct map_t {};
    template<typename S, typename F> using map = typename map_t<S, F>::type;
    template<typename S, typename ...Ts, typename F>
    struct map_t<set<S, Ts...>, F> {
        using type = prepend<map<set<Ts...>, F>, typename F::template apply<S>::type>;
    };
    template<typename S, typename F>
    struct map_t<set<S>, F> {
        using type = set<typename F::template apply<S>::type>;
    };
    template<typename F>
    struct map_t<set<>, F> {
        using type = set<>;
    };

    class SelectContext {
    public:
        Context *context;
        SelectContext(Context *context) : context(context) {}

        Context *getContext() {
            return context;
        }
        std::vector<PatternNode *> nodes;
        std::vector<std::vector<PatternNode *>> scopes;
        void clear() {
            nodes.clear();
            scopes.clear();
        }
        void enter() {
            scopes.push_back(nodes);
        }
        void leave(bool matched) {
            if (!matched) {
                nodes = scopes.back();
            }
            scopes.pop_back();
        }
        void restore() {
            nodes = scopes.back();
        }
        void push(PatternNode *node) {
            nodes.push_back(node);
        }

        RegisterNode *getReg(size_t index) {
            return nodes[index]->as<RegisterNode>();
        }

        ConstantNode *getConst(size_t index) {
            return nodes[index]->as<ConstantNode>();
        }

        PatternNode *operator[](size_t index) {
            return nodes[index];
        }

        auto begin() {
            return nodes.begin();
        }
        auto end() {
            return nodes.end();
        }
    };

    template<typename ...Args>
    class MatchOpt {
    public:
        using types = set<Args...>;
        std::tuple<Args...> matchers;
        constexpr MatchOpt(Args... matchers) : matchers(matchers...) {}
        constexpr auto operator()(PatternNode *node, SelectContext& context) const {
            return match(node, context);
        }
        inline constexpr bool match(PatternNode *node, SelectContext& context) const;
    };
    class MatchEpsilon {
    public:
        constexpr bool operator()(PatternNode *node, SelectContext& context) const {
            return false;
        }
    };
    template<typename OpTy, typename T>
    class MatchSelector {
    public:
        using Op = OpTy;
        constexpr static unsigned OPCODE = OpTy::OPCODE;
        constexpr static Pattern::Opcode OPC = (Pattern::Opcode) OPCODE;
    public:
        T matcher;
        constexpr MatchSelector() {}
        constexpr MatchSelector(T matcher) : matcher(matcher) {}
        constexpr unsigned getOpcode() const { return OPCODE; }

        constexpr bool match(PatternNode *node, SelectContext &context) const {
            if (node == nullptr) {
                std::cout << "The node is null" << std::endl;
                return false;
            }
            if (OPCODE != Pattern::None) {
                if (node->getOpcode() != OPCODE) {
                    std::cout << "Matching " << Pattern::dump(OPCODE) << " for node "
                              << Pattern::dump(node->getOpcode()) << " not matched" << std::endl;
                    return false;
                }
                std::cout << "Matching " << Pattern::dump(node->getOpcode()) << " matched" << std::endl;
            } else {
                std::cout << "Matching Any for node " << Pattern::dump(node->getOpcode()) << std::endl;
            }
            context.enter();
            auto R = matcher(node, context);;
            context.leave(R);
            return R;
        }
        constexpr auto operator()(PatternNode *node, SelectContext& context) const {
            return match(node, context);
        }
    };
    template<typename OpTy, typename ...Args>
    class MatchNode {
    public:
        using Op = OpTy;
        constexpr static unsigned OPCODE = OpTy::OPCODE;
        constexpr static Pattern::Opcode OPC = (Pattern::Opcode) OPCODE;
    public:
        using Ty = std::tuple<Args...>;
        template<size_t Index, typename ...Es>
        struct MatchNodeHelper {
            inline static constexpr bool match(Ty children, PatternNode *node, SelectContext &context) {
                return true;
            }
        };
        template<size_t Index, typename E, typename ...Es>
        struct MatchNodeHelper<Index, E, Es...> {
            inline static constexpr bool match(Ty children, PatternNode *node, SelectContext &context) {
                if (auto *Child = node->getChild(Index)) {
                    auto M = std::get<Index>(children);
                    if (!M(Child, context)) {
                        return false;
                    }
                    return MatchNodeHelper<Index + 1, Es...>::match(children, node, context);
                }
                assert("The node is null" && false);
                return false;
            }
        };
        std::tuple<Args...> childrens;
        constexpr MatchNode(Args... matchers) : childrens(matchers...) {}
        constexpr auto operator()(PatternNode *node, SelectContext& context) const {
            if (node->isShared()) {
                // FIXME: We can't select shared node.
                return false;
            }
            return match(node, context);
        }
        inline constexpr bool match(PatternNode *node, SelectContext& context) const {
            if (OPCODE != Pattern::None) {
                if (node->getOpcode() != OPCODE) {
                    std::cout << "Matching " << Pattern::dump(OPCODE) << " for node "
                              << Pattern::dump(node->getOpcode()) << " not matched" << std::endl;
                    return false;
                }
                std::cout << "Matching " << Pattern::dump(node->getOpcode()) << " matched" << std::endl;
            } else {
                std::cout << "Matching Any for node " << Pattern::dump(node->getOpcode()) << std::endl;
            }
            return MatchNodeHelper<0, Args...>::match(childrens, node, context);
        }
    };

    template<typename L, typename R>
    constexpr auto operator|(L lhs, R rhs) {
        return MatchOpt<L, R>(lhs, rhs);
    }

    template<typename L, typename R>
    class MatchApplier {
        L matcher;
        R applier;
    public:
        constexpr MatchApplier(L matcher, R applier) : matcher(matcher), applier(applier) {}
        inline constexpr bool apply(PatternNode *node, SelectContext &ctx, MachineBlock &mbb) const {
            if (matcher(node, ctx)) {
                applier(ctx, mbb);
                return true;
            }
            return false;
        }
    };
    template<typename L, typename R>
    class MatchApplierCat {
        L applier1;
        R applier2;
    public:
        constexpr MatchApplierCat(L applier1, R applier2) : applier1(applier1), applier2(applier2) {}
        inline constexpr bool apply(PatternNode *node, SelectContext &ctx, MachineBlock &mbb) const {
            return applier1.apply(node, ctx, mbb) || applier2.apply(node, ctx, mbb);
        }
    };
    template<typename L, typename R, typename T>
    constexpr auto operator||(MatchApplierCat<L, R> lhs, T rhs) {
        return MatchApplierCat<MatchApplierCat<L, R>, T>(lhs, rhs);
    }
    template<typename L, typename R, typename T>
    constexpr auto operator||(MatchApplier<L, R> lhs, T rhs) {
        return MatchApplierCat<MatchApplier<L, R>, T>(lhs, rhs);
    }
    template<typename L, typename R>
    constexpr auto operator>(L lhs, R rhs) {
        return MatchApplier<L, R>(lhs, rhs);
    }

    template<typename ...Args>
    constexpr auto Emit(unsigned opc, Args ...args) {
        return [=](SelectContext &ctx, MachineBlock &mbb) -> MachineInstr * {
            auto *Instr = new MachineInstr();
            std::vector<PatternNode *> Nodes;
            static int Values[] = {args...};
            for (auto Arg : Values) {
                Nodes.push_back(ctx[Arg]);
            }
            Instr->opcode = opc;
            mbb.append(Instr);
        };
    }


    /*template<size_t Index = 0, typename ...Args>
    struct MatchHelper {
        constexpr static inline bool match(auto tuple, PatternNode *node, SelectContext &context) {
            return true;
        }
    };
    template<size_t Index, typename Arg, typename ...Args>
    struct MatchHelper<Index, Arg, Args...> {
        constexpr static inline bool match(auto tuple, PatternNode *node, SelectContext &context) {
            auto R = std::get<Index>(tuple);
            if (!R(node->getChild(Index), context)) {
                return false;
            }
            return MatchHelper<Index + 1, Args...>::match(tuple, node, context);
        }
    };
    template<typename Node, typename ...Args>
    constexpr auto rule(Args...args) {
        auto Tuple = std::make_tuple(args...);
        return sel<Node>([=](PatternNode *node, SelectContext &context) {
            return MatchHelper<0, Args...>::match(Tuple, node, context);
        });
    }*/
    template<typename Node, typename ...Args>
    constexpr auto rule(Args...args) {
        return MatchNode<Node, Args...>(args...);
    }
    template<typename OpTy, typename T>
    constexpr auto sel(T t) {
        return MatchSelector<OpTy, T>(t);
    }
    template<typename ...Ts>
    constexpr auto opt(Ts ...args) {
        return MatchOpt<Ts...>(args...);
    }
    template<typename OpTy>
    constexpr auto type(MachineType type) {
        return sel<OpTy>([=](PatternNode *node, SelectContext &context) {
            if (node->getType()->getMachineType() == type) {
                context.push(node);
                return true;
            }
            context.push(node);
            return true;
        });
    }
    constexpr auto reg(MachineType ty) {
        return type<RegisterNode>(ty) |
               sel<CopyFromReg>([=](PatternNode *node, SelectContext &context) {
                   if (node->getType()->getMachineType() == ty) {
                       context.push(node);
                       return true;
                   }
                   context.push(node);
                   return true;
               });
    }
    constexpr auto imm(MachineType type) {
        return sel<ConstantNode>([=](PatternNode *node, SelectContext &context) {
            context.push(node);
            return true;
        });
    }
    template<typename OpTy, typename T>
    constexpr auto unary(T val) {
        /*return sel<OpTy>([=](PatternNode *node, SelectContext &context) {
            return val(node->getChild(0), context);
        });*/
        return rule<OpTy>(val);
    }
    template<typename OpTy, typename L, typename R>
    constexpr auto bin(L lhs, R rhs) {
        /*return sel<OpTy>([=](PatternNode *node, SelectContext &context) {
            if (!lhs(node->getChild(0), context)) {
                return false;
            }
            if (!rhs(node->getChild(1), context)) {
                return false;
            }
            return true;
        });*/
        return rule<OpTy>(lhs, rhs);
    }
#define DEF_UNARY(NAME, OPTY) template<typename L> constexpr auto NAME(L val) { \
        return unary<OPTY>(val); \
    }
#define DEF_BINARY(NAME, OPTY) template<typename L, typename R> constexpr auto NAME(L lhs, R rhs) { \
        return bin<OPTY>(lhs, rhs); \
    }
    DEF_BINARY(add, AddNode);
    DEF_BINARY(sub, SubNode);
    DEF_BINARY(mul, MulNode);
    DEF_BINARY(div, DivNode);
    DEF_UNARY(ret, ReturnNode);

    constexpr auto same(int index) {
        return sel<PatternNode>([=](PatternNode *node, SelectContext &context) {
            return context[index] == node;
        });
    }

    constexpr auto IReg = reg(MachineI32);
    constexpr auto FReg = reg(MachineF32);
    constexpr auto Imm = imm(MachineI32);
    constexpr auto Any = sel<PatternNode>([](PatternNode *node, SelectContext &context) {
        context.push(node);
        return true;
    });

    // Tune the priority of the patterns, match any finally
    template<typename T>
    using tune_opset = if_v<contains<T, PatternNode>, add_element<remove_element<T, PatternNode>, PatternNode>, T>;

    template<typename T>
    class OpSetGetter {};
    template<typename T>
    using get_opset = tune_opset<typename OpSetGetter<std::remove_const_t<T>>::opset>;
    template<typename OpTy, typename T> struct OpSetGetter<MatchSelector<OpTy, T>> {
        ///< get opset for MatchSelector
        using opset = set<OpTy>;
    };
    template<typename OpTy, typename ...Ts> struct OpSetGetter<MatchNode<OpTy, Ts...>> {
        ///< get opset for MatchNode
        using opset = set<OpTy>;
    };
    template<typename Cur, typename ...Args>
    struct OpSetGetter<MatchOpt<Cur, Args...>> {
        ///< get opset for MatchOpt
        using opset = join<get_opset<Cur>, get_opset<MatchOpt<Args...>>>;
    };
    template<> struct OpSetGetter<MatchOpt<>> {
        ///< get opset for MatchOpt
        using opset = set<>;
    };
    template<typename T, typename ...Ts> struct OpSetGetter<set<T, Ts...>> {
        ///< get opset for set of selector
        using opset = join<get_opset<T>, get_opset<set<Ts...>>>;
    };
    template<> struct OpSetGetter<set<>> {
        ///< get opset for set of selector
        using opset = set<>;
    };

    template<typename MatchOptTy, typename OpTy>
    struct FindForMatchOpt {};
    template<typename MatchOpTy, typename OpTy>
    using find_sel_by_op_for_opt = typename FindForMatchOpt<std::remove_const_t<MatchOpTy>, OpTy>::type;
    template<typename OpTy, typename CurOpTy, typename T, typename ...Args>
    struct FindForMatchOpt<MatchOpt<MatchSelector<CurOpTy, T>, Args...>, OpTy> {
        using cur_type = MatchSelector<CurOpTy, T>;
        using others = MatchOpt<Args...>;
        using type = if_v<std::is_same_v<OpTy, CurOpTy>,
                prepend<find_sel_by_op_for_opt<others, OpTy>, cur_type>,
                find_sel_by_op_for_opt<others, OpTy>>;
    };
    template<typename OpTy>
    struct FindForMatchOpt<MatchOpt<>, OpTy> {
        ///< not found
        using type = set<>;
    };

    template<typename SelTy, typename OpTy>
    struct FindSelectorForOp {};
    template<typename SelTy, typename OpTy>
    using find_sel = typename FindSelectorForOp<SelTy, OpTy>::selset;
    template<typename T, typename ...Ts, typename OpTy>
    struct FindSelectorForOp<MatchOpt<T, Ts...>, OpTy> {
        using selset = join<find_sel<T, OpTy>, find_sel<MatchOpt<Ts...>, OpTy>>;
    };
    template<typename OpTy>
    struct FindSelectorForOp<MatchOpt<>, OpTy> {
        using selset = set<>;
    };
    template<typename CurOpTy, typename T, typename OpTy>
    struct FindSelectorForOp<MatchSelector<CurOpTy, T>, OpTy> {
        using selset = if_v<std::is_same_v<OpTy, CurOpTy>,
                set<MatchSelector<CurOpTy, T>>,
                set<>>;
    };
    template<typename CurOpTy, typename ...Ts, typename OpTy>
    struct FindSelectorForOp<MatchNode<CurOpTy, Ts...>, OpTy> {
        using selset = if_v<std::is_same_v<OpTy, CurOpTy>,
                set<MatchNode<CurOpTy, Ts...>>,
                set<>>;
    };

    ///< This helper class is used to get the selector value whose type is SelTy
    template<typename T, typename SelTy, size_t Index = 0, typename ...>
    struct SelectorHelper {};
    template<typename SelTy, size_t Index, typename ...Ts>
    struct SelectorHelper<MatchOpt<Ts...>, SelTy, Index> {
        using T = MatchOpt<Ts...>;
        constexpr inline static SelTy get(T val) {
            return SelectorHelper<T, SelTy, 0, Ts...>::get(val);
        }
    };
    template<typename T, typename SelTy, size_t Index, typename ...Es, typename ...Ts>
    struct SelectorHelper<T, SelTy, Index, MatchOpt<Es...>, Ts...> {
        using CurTy = MatchOpt<Es...>;
        template<typename ...Args>
        constexpr inline static SelTy get(MatchOpt<Args...> val) {
            if constexpr (contains<find_sel<CurTy, typename SelTy::Op>, SelTy>) {
                return SelectorHelper<CurTy, SelTy>::get(std::get<Index>(val.matchers));
            }
            return SelectorHelper<T, SelTy, Index + 1, Ts...>::get(val);
        }
    };
    template<typename SelTy, size_t Index, typename OpTy, typename T>
    struct SelectorHelper<MatchSelector<OpTy, T>, SelTy, Index> {
        using type = MatchSelector<OpTy, T>;
        constexpr inline static SelTy get(type val) {
            static_assert(std::is_same_v<SelTy, type>, "Selector type mismatch");
            return val;
        }
    };
    template<typename SelTy, size_t Index, typename OpTy, typename ...Ts>
    struct SelectorHelper<MatchNode<OpTy, Ts...>, SelTy, Index> {
        using type = MatchNode<OpTy, Ts...>;
        constexpr inline static SelTy get(type val) {
            static_assert(std::is_same_v<SelTy, type>, "Selector type mismatch");
            return val;
        }
    };
    template<typename T, typename SelTy, size_t Index, typename CurOpTy, typename CurT, typename ...Ts>
    struct SelectorHelper<T, SelTy, Index, MatchSelector<CurOpTy, CurT>, Ts...> {
        using CurTy = MatchSelector<CurOpTy, CurT>;
        template<typename ...Args>
        constexpr inline static SelTy get(MatchOpt<Args...> val) {
            if constexpr (std::is_same_v<SelTy, CurTy>) {
                return std::get<Index>(val.matchers);
            }
            return SelectorHelper<T, SelTy, Index + 1, Ts...>::get(val);
        }
    };
    template<typename T, typename SelTy, size_t Index, typename CurOpTy, typename ...CurTs, typename ...Ts>
    struct SelectorHelper<T, SelTy, Index, MatchNode<CurOpTy, CurTs...>, Ts...> {
        using CurTy = MatchNode<CurOpTy, CurTs...>;
        template<typename ...Args>
        constexpr inline static SelTy get(MatchOpt<Args...> val) {
            if constexpr (std::is_same_v<SelTy, CurTy>) {
                return std::get<Index>(val.matchers);
            }
            return SelectorHelper<T, SelTy, Index + 1, Ts...>::get(val);
        }
    };

    ///< The helper class is used to call the matching function for the selector value by specifying op/opset.
    template<typename OpTy, typename T, typename = find_sel<T, OpTy>>
    struct SelMatchForOpHelper {
        constexpr inline static auto match(T val, PatternNode *node, SelectContext &ctx) {
            return false;
        }
    };
    template<typename OpTy, typename T, typename SelTy, typename ...Args>
    struct SelMatchForOpHelper<OpTy, T, set<SelTy, Args...>> {
        constexpr inline static auto match(T val, PatternNode *node, SelectContext &ctx) {
            auto Sel = SelectorHelper<T, SelTy>::get(val);
            return Sel(node, ctx) || SelMatchForOpHelper<OpTy, T, set<Args...>>::match(val, node, ctx);
        }
    };
    template <typename OpSet, typename T>
    struct SelMatchForOpSetHelper {
        constexpr inline static auto match(T val, PatternNode *node, SelectContext &ctx) {
            return false;
        }
    };
    template <typename Op, typename ...Ops, typename T>
    struct SelMatchForOpSetHelper<set<Op, Ops...>, T> {
        constexpr inline static auto match(T val, PatternNode *node, SelectContext &ctx) {
            if (node->getOpcode() == Op::OPCODE || Op::OPCODE == PatternNode::OPCODE) {
                return SelMatchForOpHelper<Op, T>::match(val, node, ctx);
            }
            return SelMatchForOpSetHelper<set<Ops...>, T>::match(val, node, ctx);
        }
    };

    template<typename OpTy, typename T>
    constexpr inline auto match_for_op(T val, PatternNode *node, SelectContext &ctx) {
        return SelMatchForOpHelper<OpTy, T>::match(val, node, ctx);
    }
    template<typename OpSet, typename T>
    constexpr inline auto match_for_opset(T val, PatternNode *node, SelectContext &ctx) {
        return SelMatchForOpSetHelper<OpSet, T>::match(val, node, ctx);
    }

    template<typename... Args>
    constexpr bool MatchOpt<Args...>::match(PatternNode *node, SelectContext &context) const {
        using opset = get_opset<MatchOpt<Args...>>;
        return match_for_opset<opset>(*this, node, context);
    }

    ///< Test codes
    template<typename Ty> struct OpToSelMapper {
        // Ty -> SelTy
        template<typename Op> struct apply {
            using type = find_sel<Ty, Op>;
        };
    };
    template<typename Ty, typename CurTy = Ty>
    class Combiner {
    public:
        constexpr Combiner() {}
    };
    template<typename Ty>
    class Combiner<Ty, MatchOpt<>> {
    public:
        using OpSet = set<>;
    };
    template<typename Ty, typename OpTy, typename T, typename ...Args>
    class Combiner<Ty, MatchOpt<MatchSelector<OpTy, T>, Args...>> {
    public:
        using CurTy = MatchOpt<MatchSelector<OpTy, T>, Args...>;
        using OpSet = get_opset<CurTy>;
        using Matcher = map<OpSet, OpToSelMapper<Ty>>;
        Ty matcher;
        constexpr Combiner(Ty matcher) : matcher(matcher) {}
        template<typename L, typename R>
        constexpr auto operator()(L lhs, R rhs) {
            return matcher(lhs, rhs);
        }
    };
    template<typename Ty, typename ...Opts, typename ...Args>
    class Combiner<Ty, MatchOpt<MatchOpt<Opts...>, Args...>> {
    public:
        using CurTy = MatchOpt<Opts...>;
        using OtherOptTy = MatchOpt<Args...>;
        using OtherOptSet = typename Combiner<OtherOptTy>::OpSet;
        using OpSet = join<get_opset<CurTy>, OtherOptSet>;
        using Matcher = map<OpSet, OpToSelMapper<Ty>>;
        Ty matcher;
        constexpr Combiner(Ty matcher) : matcher(matcher) {}
    };
    template<typename T>
    using get_matcher = map<get_opset<T>, OpToSelMapper<T>>;
    inline void test() {
        //constexpr auto A = add(Imm, Imm) | add(IReg, IReg) | add(IReg, Imm) | add(Imm, IReg) | ret(IReg) | ret(Imm);
        constexpr auto A = add(IReg, IReg) | add(IReg, Imm) | add(Imm, IReg);

        using AT = std::remove_const_t<decltype(A)>;
        using opset = get_opset<AT>;
        using Sel = find_sel<AT, ReturnNode>;

        using Mt = get_matcher<AT>;
        using AddSet = get_element<Mt, 0>;
        using AddRR = get_element<AddSet, 0>;
        using AddRR1 = get_element<AddSet, 1>;
        using AddRR2 = get_element<AddSet, 2>;
        std::cout << typeid(AT).name() << std::endl;
        std::cout << typeid(AddRR).name() << std::endl;

        //constexpr auto V = get_selector<AddRR2>(A);
        //constexpr auto C = SelectorHelper<AT, AddRR1>::get(A);

        //SelMatchForOpHelper<AddNode>::
        //SelectContext Ctx;
        //match_for_op<AddNode>(A, nullptr, Ctx);
        //std::cout << Pattern::dump(C.OPCODE) << std::endl;

        std::cout << std::endl << std::endl;

        //using matcher = get_matcher<T>;
        //std::cout << typeid(get_element<T, 0>).name() << std::endl;


        /*using T = get_matcher<decltype(A)>;
        using Add = get_element<T, 0>;

        std::cout << typeid(get_element<Add, 0>).name() << std::endl;
        std::cout << typeid(get_element<Add, 1>).name() << std::endl;

        auto add = get_selector<get_element<Add, 0>>(A);*/

        //std::cout << typeid(get_opset<decltype(A)>).name() << std::endl;

    }

} // namespace Matcher

#endif //DRAGON_SELECTOR_H
