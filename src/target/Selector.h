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
    template<typename S, typename E, bool = contains_t<S, E>::value> struct remove_t {
        using type = S;
    };
    template<typename T, typename ...Ts, typename E> struct remove_t<set<T, Ts...>, E, false> {
        using type = prepend<typename remove_t<Ts...>::type, T>;
    };
    template<typename T, typename ...Ts> struct remove_t<set<T, Ts...>, T, false> {
        using type = set<Ts...>;
    };
    template<typename S, typename E>
    using remove = typename remove_t<S, E>::type;

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
    template <typename S, size_t Index> struct get_element_t {
        using type = void;
    };
    template <typename N, typename ...T, size_t Index> struct get_element_t<set<N, T...>, Index> {
        using type = if_v<Index == 0, N, typename get_element_t<set<T...>, Index - 1>::type>;
    };
    template<typename S, size_t Index>
    using get_element = typename get_element_t<S, Index>::type;

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
    template <typename N, typename E, int Index> struct set_element_t<set<N>, Index, E> {
        using type = std::enable_if_t<Index == 0, set<N>>;
    };

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

    /// for each set
    template<typename S, typename F> struct for_each_t {
        using type = S;
    };
    template<typename ...Ts, typename F> struct for_each_t<set<Ts...>, F> {
        using type = set<typename F::template apply<Ts>...>;
    };
    template<typename S, typename F>
    using for_each = typename for_each_t<S, F>::type;

    class SelectContext {
    public:
        std::vector<PatternNode *> nodes;
        std::vector<std::vector<PatternNode *>> scopes;
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
        PatternNode *operator[](size_t index) {
            return nodes[index];
        }
    };

    template<typename ...Args>
    class MatchOpt {
    public:
        std::tuple<Args...> matchers;
        constexpr MatchOpt(Args... matchers) : matchers(matchers...) {}
        constexpr auto operator()(PatternNode *node, SelectContext& context) const {
            return std::apply([&](auto ...args) {
                return (args(node, context) || ...);
            }, matchers);
        }

        /// get
        template<typename T>
        constexpr auto get() const {
            return std::get<T>(matchers);
        }
    };

    class MatchEpsilon {
    public:
        constexpr bool operator()(PatternNode *node, SelectContext& context) const {
            return false;
        }
    };

    constexpr auto operator|(auto lhs, auto rhs) {
        return MatchOpt(lhs, rhs);
    }

    template<typename OpTy, typename T>
    class MatchSelector {
    public:
        using Op = OpTy;
        constexpr static unsigned OPCODE = OpTy::OPCODE;
        constexpr static Pattern::Opcode OPC = (Pattern::Opcode) OPCODE;
    public:
        T matcher;
        constexpr MatchSelector() = default;
        constexpr MatchSelector(T matcher) : matcher(matcher) {}

        constexpr unsigned getOpcode() const {
            return OPCODE;
        }

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
            }
            std::cout << "Matching " << Pattern::dump(node->getOpcode()) << " matched" << std::endl;
            context.enter();
            auto R = matcher(node, context);;
            context.leave(R);
            return R;
        }
        constexpr auto operator()(PatternNode *node, SelectContext& context) const {
            return match(node, context);
        }
    };

    template<typename L, typename R>
    class MatchApplier {
        L matcher;
        R applier;
    public:
        constexpr MatchApplier(L matcher, R applier) : matcher(matcher), applier(applier) {}

        void apply(PatternNode *node, SelectContext &ctx, MachineBlock &mbb) {

        }
    };
    constexpr auto Emit(unsigned opcode, auto ...args) {
        return [=](SelectContext &ctx) -> MachineInstr * {
            //constexpr int Values[] = {args...};
            auto *Instr = new MachineInstr();
            std::vector<PatternNode *> Nodes;
            for (auto Arg : {args...}) {
                Nodes.push_back(ctx[Arg]);
            }
            Instr->opcode = opcode;
            return Instr;
        };
    }
    constexpr auto operator>>(auto lhs, auto rhs) {
        return MatchApplier(lhs, rhs);
    }

    template<typename OpTy>
    constexpr auto sel(auto t) {
        return MatchSelector<OpTy, decltype(t)>(t);
    }
    constexpr auto opt(auto ...args) {
        return MatchOpt(args...);
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
        return type<VirRegNode>(ty) | type<PhyRegNode>(ty) | type<CopyFromReg>(ty);
    }
    constexpr auto imm(MachineType type) {
        return sel<ConstantNode>([=](PatternNode *node, SelectContext &context) {
            context.push(node);
            return true;
        });
    }
    template<typename OpTy>
    constexpr auto unary(auto val) {
        return sel<OpTy>([=](PatternNode *node, SelectContext &context) {
            return val(node->getChild(0), context);
        });
    }
    template<typename OpTy>
    constexpr auto bin(auto lhs, auto rhs) {
        return sel<OpTy>([=](PatternNode *node, SelectContext &context) {
            if (!lhs(node->getChild(0), context)) {
                return false;
            }
            if (!rhs(node->getChild(1), context)) {
                return false;
            }
            return true;
        });
    }
    constexpr auto add(auto lhs, auto rhs) {
        return bin<AddNode>(lhs, rhs);
    }
    constexpr auto sub(auto lhs, auto rhs) {
        return bin<SubNode>(lhs, rhs);
    }
    constexpr auto mul(auto lhs, auto rhs) {
        return bin<MulNode>(lhs, rhs);
    }
    constexpr auto div(auto lhs, auto rhs) {
        return bin<DivNode>(lhs, rhs);
    }
    constexpr auto ret(auto val) {
        return unary<ReturnNode>(val);
    }

    constexpr auto IReg = reg(MachineI32);
    constexpr auto FReg = reg(MachineF32);
    constexpr auto Imm = imm(MachineI32);

    template<typename T>
    class OpSetGetter {};
    template<typename T>
    using get_opset = typename OpSetGetter<T>::opset;
    template<typename OpTy, typename T> struct OpSetGetter<MatchSelector<OpTy, T>> {
        ///< get opset for MatchSelector
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
    using find_op = typename FindForMatchOpt<MatchOpTy, OpTy>::type;
    template<typename OpTy, typename CurOpTy, typename T, typename ...Args>
    struct FindForMatchOpt<MatchOpt<MatchSelector<CurOpTy, T>, Args...>, OpTy> {
        using cur_type = MatchSelector<CurOpTy, T>;
        using others = MatchOpt<Args...>;
        using type = if_v<std::is_same_v<OpTy, CurOpTy>,
                prepend<find_op<others, OpTy>, cur_type>,
                find_op<others, OpTy>>;
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
    template<typename CurOpTy, typename T>
    struct FindSelectorForOp<MatchSelector<CurOpTy, T>, CurOpTy> {
        using selset = set<MatchSelector<CurOpTy, T>>;
    };
    template<typename CurOpTy, typename T, typename OpTy>
    struct FindSelectorForOp<MatchSelector<CurOpTy, T>, OpTy> {
        using selset = if_v<std::is_same_v<OpTy, CurOpTy>,
                set<MatchSelector<CurOpTy, T>>,
                set<>>;
    };

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
        constexpr auto operator()(auto lhs, auto rhs) {
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

    template<typename T, typename SelTy, typename ...>
    struct SelectorHelper {};
    template<typename SelTy, typename T, typename ...Ts>
    struct SelectorHelper<MatchOpt<T, Ts...>, SelTy> {
        using type = MatchOpt<T, Ts...>;
        inline static SelTy get(type val) {
            return SelectorHelper<type, SelTy, Ts...>::get(val);
        }
    };
    template<typename T, typename SelTy, typename CurOpTy, typename CurT, typename ...Ts>
    struct SelectorHelper<T, SelTy, MatchSelector<CurOpTy, CurT>, Ts...> {
        using CurTy = MatchSelector<CurOpTy, CurT>;
        inline static SelTy get(T val) {
            if (std::is_same_v<SelTy, CurTy>) {
                return val.template get<SelTy>();
            }
            return SelectorHelper<T, SelTy, Ts...>::get(val);
        }
    };
    template<typename T, typename SelTy, typename ...Es, typename ...Ts>
    struct SelectorHelper<T, SelTy, MatchOpt<Es...>, Ts...> {
        using CurTy = MatchOpt<Es...>;
        inline static SelTy get(T val) {
            if(contains<get_opset<CurTy>, typename SelTy::Op>) {
                return SelectorHelper<CurTy, SelTy>::get(val.template get<CurTy>());
            }
            return SelectorHelper<T, SelTy, Ts...>::get(val);
        }
    };

    template<typename T>
    constexpr auto get_selector(auto val) {
        return SelectorHelper<decltype(val), T>::get(val);
    }
    constexpr auto combine(auto opt) {
        return Combiner<decltype(opt)>(opt);
    }

    inline void test() {
        //constexpr auto A = combine(add(Imm, Imm) | add(IReg, Imm));
        /*constexpr auto A = combine(add(Imm, Imm) | ret(IReg));
        std::cout << typeid(get_element<decltype(A)::Matcher, 0>).name() << std::endl;
        std::cout << Pattern::dump(get_element<get_element<decltype(A)::Matcher, 0>, 0>::OPCODE) << std::endl;*/

        //find_sel<>


        /*constexpr auto A = add(Imm, Imm) | add(IReg, IReg) | add(IReg, Imm) | add(Imm, IReg) | ret(IReg) | ret(Imm);
        constexpr auto C = combine(A);
        using T = decltype(C)::Matcher;
        std::cout << typeid(T).name() << std::endl;
        using TT = get_element<T, 0>;
        using F = get_element<TT, 0>; // MatchSelector
        auto S = get_selector<F>(A);
        std::cout << Pattern::dump(get_element<TT, 0>::OPCODE) << std::endl;*/

        constexpr auto A = add(IReg, IReg);

        using AT = decltype(A);
        std::cout << typeid(AT).name() << std::endl;
        //using Sel = FindSelectorForOp<decltype(A), AddNode>::selset;


        using Sel = find_sel<AT, AddNode>;

        std::cout << typeid(Sel).name() << std::endl;

        std::cout << std::endl << std::endl;


        //using Opset = get_matcher<decltype(A)>;


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
