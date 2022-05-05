//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_SELECTOR_H
#define DRAGON_SELECTOR_H

#include "Type.h"
#include "PatternNode.h"
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
    template<unsigned Opcode, typename T>
    class MatchSelector {
    public:
        constexpr static unsigned OPCODE = Opcode;
        T matcher;
        constexpr MatchSelector() = default;
        constexpr MatchSelector(T matcher) : matcher(matcher) {}

        constexpr unsigned getOpcode() const {
            return Opcode;
        }

        constexpr bool match(PatternNode *node, SelectContext &context) const {
            if (node == nullptr) {
                std::cout << "The node is null" << std::endl;
                return false;
            }
            if (Opcode != Pattern::None && node->getOpcode() != Opcode) {
                std::cout << "Matching " << Pattern::dump(node->getOpcode()) << " -> " << Pattern::dump(Opcode)
                          << " not matched" << std::endl;
                return false;
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
            if (matcher(node, ctx)) {
                auto *Instr = applier(ctx);
                mbb.append(Instr);
            }
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

    /*template<typename T>
    class Combiner {
    public:
        static_assert(false, "Combiner is not implemented");
        constexpr Combiner() {}
    };
    template<typename T>
    class Combiner<T> {

    };
    template<unsigned Opcode, typename T, typename ...Args>
    class Combiner<MatchOpt<MatchSelector<Opcode, T>, Args...>> {
    public:
        using CurTy = MatchSelector<Opcode, T>;
        using OtherTy = MatchOpt<Args...>;
        using Ty = MatchOpt<CurTy, Args...>;
        constexpr static unsigned Opcodes[] = {Opcode};

        Ty matcher;
        constexpr Combiner(Ty matcher) : matcher(matcher) {}
        void match(PatternNode *node, SelectContext &context) const {
            //auto M = matcher.matchers.get<CurTy>().matcher;

        }
    };*/

    template<unsigned Opcode>
    constexpr auto sel(auto t) {
        return MatchSelector<Opcode, decltype(t)>(t);
    }
    constexpr auto opt(auto ...args) {
        return MatchOpt(args...);
    }
    template<unsigned Opcode>
    constexpr auto type(MachineType type) {
        return sel<Opcode>([=](PatternNode *node, SelectContext &context) {
            if (node->getType()->getMachineType() == type) {
                context.push(node);
                return true;
            }
            context.push(node);
            return true;
        });
    }
    constexpr auto reg(MachineType ty) {
        return type<Pattern::VirRegister>(ty) | type<Pattern::PhyRegister>(ty) | type<Pattern::CopyFromReg>(ty);
    }
    constexpr auto imm(MachineType type) {
        return sel<Pattern::Constant>([=](PatternNode *node, SelectContext &context) {
            context.push(node);
            return true;
        });
    }
    template<unsigned Opcode>
    constexpr auto unary(auto val) {
        return sel<Opcode>([=](PatternNode *node, SelectContext &context) {
            return val(node->getChild(0), context);
        });
    }
    template<unsigned Opcode>
    constexpr auto bin(auto lhs, auto rhs) {
        return sel<Opcode>([=](PatternNode *node, SelectContext &context) {
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
        return bin<Pattern::Add>(lhs, rhs);
    }
    constexpr auto sub(auto lhs, auto rhs) {
        return bin<Pattern::Sub>(lhs, rhs);
    }
    constexpr auto mul(auto lhs, auto rhs) {
        return bin<Pattern::Mul>(lhs, rhs);
    }
    constexpr auto div(auto lhs, auto rhs) {
        return bin<Pattern::Div>(lhs, rhs);
    }
    constexpr auto ret(auto val) {
        return unary<Pattern::Return>(val);
    }

    constexpr auto IReg = reg(MachineI32);
    constexpr auto FReg = reg(MachineF32);
    constexpr auto Imm = imm(MachineI32);

    constexpr void test() {
        constexpr  auto B = Imm | Imm;
        //constexpr auto A = Combiner(B);


    }


} // namespace Matcher

#endif //DRAGON_SELECTOR_H
