//
// Created by Alex on 2022/3/10.
//

#ifndef DRAGON_PATGEN_H
#define DRAGON_PATGEN_H

#include "parser.h"
#include <fstream>
#include <set>

///< The wrapper for parsing files on LR/GLR parser.
class StreamIter {
    std::istream *stream = nullptr;
    int chr = 0;
public:
    using iterator_category = typename std::input_iterator_tag;
    using value_type = char;
    using difference_type = long;
    using pointer = const char *;
    using reference = char &;
    constexpr StreamIter() = default;
    StreamIter(std::istream &os) : stream(&os) {
        ++*this;
    }
    inline bool empty() const { return stream == nullptr; }
    inline bool eof() const { return  !stream || (stream && stream->eof()); }
    inline const int operator*() const { return chr; }
    inline StreamIter&operator++() {
        chr = (!empty()) ? stream->get() : 0;
        return *this;
    }
    inline bool operator==(const StreamIter &rhs) {
        return rhs.empty() && eof();
    }
    inline bool operator!=(const StreamIter &rhs) {
        return !(*this == rhs);
    }
};
class FileStreamWrapper {
private:
    std::fstream file;
public:
    FileStreamWrapper(const char *path) {
        file.open(path, std::ios::in);
    }
    ~FileStreamWrapper() {
        if (file.is_open()) {
            file.close();
        }
    }
    StreamIter begin() {
        return StreamIter(this->file);
    }
    StreamIter end() { return StreamIter(); }
};

// override << for Location
std::ostream &operator<<(std::ostream &os, const Location &loc) {
    return os << "(" << loc.line_start << "," << loc.column_start << ")";
}

///< The Pattern of the Pattern Node.
class PatternOfNode {
public:
    // add(src: $1, dst: $2)
    ///< Is the pattern a leaf?
    bool isLeaf = false;
    ///< The matching arguments that start with $ like $1, $2, $3.
    std::string argName;
    ///< The opcode of the pattern node.
    std::string opcode;
    ///< The type of the pattern node.
    std::string type;
    ///< The children of the pattern node
    std::vector<PatternOfNode *> nodes;
    ///< The location.
    Location loc;
public:
    bool isPatternLeaf() const {
        return isLeaf;
    }

    bool isEmptyType() const {
        return type.empty();
    }

    void setArgName(const std::string &name) {
        argName = name;
    }

    inline std::string &getArgName() {
        return argName;
    }

    inline std::string &getOpcode() {
        return opcode;
    }

    inline std::string &getType() {
        return type;
    }

    void setType(const std::string &ty) {
        this->type = ty;
    }

    void setOpcode(const std::string &op) {
        opcode = op;
    }

    inline std::vector<PatternOfNode *> &getNodes() {
        return nodes;
    }

    inline size_t getNodeSize() const {
        return nodes.size();
    }

};

///< Register class.
class Registers {
public:
    ///< The regsiter class name.
    std::string name;
    ///< The register type.
    std::string type;
    ///< The name of the registers.
    std::vector<std::string> registers;

    ///< Constructor.
    Registers() = default;

public:

};
class Rewriter {
public:
    PatternOfNode *pattern = nullptr;
    PatternOfNode *rewriter = nullptr;;
    std::map<std::string, std::string> properties;

public:
    Rewriter() = default;
    Rewriter(PatternOfNode *pattern, PatternOfNode *rewriter) : pattern(pattern), rewriter(rewriter) {}

    PatternOfNode *getPattern() {
        return pattern;
    }

    PatternOfNode *getRewriter() {
        return rewriter;
    }

    std::map<std::string, std::string> &getProperties() {
        return properties;
    }

};
class MachineInstr {
public:
    PatternOfNode *node = nullptr;
    std::map<std::string, std::string> properties;
public:
    MachineInstr() = default;
    MachineInstr(PatternOfNode *node) : node(node) {}

    PatternOfNode *getNode() {
        return node;
    }

    std::map<std::string, std::string> &getProperties() {
        return properties;
    }
};

class PatGen : public Visitor<PatGen> {
    friend class RewriterGenerator;
private:
    ///< The target name.
    std::string targetName;
    ///< The register class.
    std::vector<Registers> registers;
    ///< The rewriters.
    std::vector<Rewriter> rewriters;
    ///< The machine instructions.
    std::vector<MachineInstr> machineInstrs;

    ///< The storage of the pattern.
    std::vector<std::unique_ptr<PatternOfNode>> storages;

    ///< The arg stack.
    std::vector<PatternOfNode *> argStack;
    ///< The temp properties.
    std::map<std::string, std::string> properties;

public:
    PatGen() {}

    PatternOfNode *createType(const std::string &type) {
        PatternOfNode *Node = new PatternOfNode();
        Node->isLeaf = true;
        Node->setType(type);
        storages.emplace_back(Node);
        return Node;
    }
    PatternOfNode *createLeaf(const std::string &argName,const std::string &type = "") {
        PatternOfNode *Node = new PatternOfNode();
        Node->isLeaf = true;
        Node->setType(type);
        Node->setArgName(argName);
        storages.emplace_back(Node);
        return Node;
    }
    PatternOfNode *createNode(const std::string &opcode, const std::vector<PatternOfNode *> &nodes, const std::string &type = "") {
        PatternOfNode *Node = new PatternOfNode();
        Node->isLeaf = false;
        Node->nodes = nodes;
        Node->setType(type);
        Node->setOpcode(opcode);
        storages.emplace_back(Node);
        return Node;
    }

    ///< Parse the patterns from the file.
    bool parse(const char *file, bool dump = false) {
        LRParser<StreamIter> Parser(true);
        FileStreamWrapper Stream(file);
        Parser.reset(Stream.begin(), Stream.end());
        Parser.parse();
        if (!Parser.accept()) {
            return false;
        }
        visit(Parser.value());
        if (dump) {
            std::cout << Parser.value().dump(4);
        }
        return true;
    }

    PatternOfNode *pop_back() {
        auto *Node = argStack.back();
        argStack.pop_back();
        return Node;
    }

    void visitTarget(Target value) override {
        targetName = value.getTarget();
        visit(value.getBody());
    }

    void visitRegisterClass(RegisterClass value) override {
        auto &Reg = registers.emplace_back();
        Reg.name = value.getName();
        Reg.type = value.getType();
        for (auto &RegName: value.getLists()) {
            Reg.registers.push_back(RegName);
        }
    }

    void visitInstructions(Instructions value) override {
        visit(value.getList());
    }

    void visitInstruction(Instruction value) override {
        visit(value.getInstrs());
        visit(value.getProperties());
        machineInstrs.emplace_back(pop_back());
        machineInstrs.back().properties = std::move(properties);
    }

    void visitArgRule(ArgRule value) override {
        visit(value.getRule());
        assert(!argStack.empty());
        argStack.back()->setType(value.getType());
        argStack.back()->loc = value.getLocation();
        //auto *Node = createNode()
    }

    void visitArgVar(ArgVar value) override {
        argStack.push_back(createLeaf(value.getVar(), value.getType()));
        argStack.back()->loc = value.getLocation();
    }

    void visitRule(Rule value) override {
        auto BeforeSize = argStack.size();
        visit(value.getArgs());
        auto Size = argStack.size() - BeforeSize;
        std::vector<PatternOfNode *> Nodes(argStack.end() - Size, argStack.end());
        auto *Node = createNode(value.getName(), Nodes);
        argStack.erase(argStack.end() - Size, argStack.end());
        argStack.push_back(Node);
        argStack.back()->loc = value.getLocation();
    }

    void visitPatterns(Patterns value) override {
        visit(value.getPatterns());
    }

    void visitPattern(Pattern value) override {
        visit(value.getPattern());
        auto *Pattern = pop_back();
        visit(value.getRewriter());
        auto *Rewriter = pop_back();
        rewriters.emplace_back(Pattern, Rewriter);
        visit(value.getProperties());
        rewriters.back().properties = std::move(properties);
    }

    void visitNumProperty(NumProperty value) override {
        properties[value.getName()] = value.getValue();
    }

    void visitProperty(Property value) override {
        properties[value.getName()] = value.getValue();
    }

    void visitStrProperty(StrProperty value) override {
        properties[value.getName()] = value.getValue();
    }

};

///< The DFA State of the Pattern Node.
class PatternApplyer {
public:
    ///< The current pattern.
    PatternOfNode *pattern;
    ///< The rewriter that will be applied.
    Rewriter *rewriter;

    PatternApplyer(PatternOfNode *pattern, Rewriter *rewriter)
        : pattern(pattern), rewriter(rewriter) {}

    PatternOfNode *getPattern() const {
        return pattern;
    }

    Rewriter *getRewriter() const {
        return rewriter;
    }

    bool isMatched() const {
        return pattern->isLeaf;
    }

    PatternApplyer enter(int index) const {
        return PatternApplyer(pattern->nodes[index], rewriter);
    }

    bool operator<(const PatternApplyer &other) const {
        //return pattern < other.pattern;
        return std::tie(pattern, rewriter) < std::tie(other.pattern, other.rewriter);
    }

    bool operator==(const PatternApplyer &other) const {
        return pattern == other.pattern && rewriter == other.rewriter;
    }

};

class State;
class Transition {
public:
    enum TransitionType {
        None,
        MatchOpcode,
        MatchLeaf,
        Enter,
    };
    TransitionType kind = None;
    std::string opcode;
    PatternOfNode *node = nullptr;
    Rewriter *rewriter = nullptr;
    State *goTo = nullptr;
    int index = -1;

    Transition(TransitionType kind, int index) : kind(kind), index(index) {}
    Transition(TransitionType kind, const std::string &opcode, State *goTo) : kind(kind), opcode(opcode), goTo(goTo) {}
    Transition(TransitionType kind, int index, State *goTo) : kind(kind), goTo(goTo), index(index) {}
    Transition(TransitionType kind, PatternOfNode *node, Rewriter *rewriter) : kind(kind), node(node),
                                                                               rewriter(rewriter) {}
};
class State {
public:
    int index = 0;
    bool isMatch = false;
    std::set<PatternApplyer> gotos;
    std::vector<Transition> transitions;
    State(const std::set<PatternApplyer> &gotos) : gotos(gotos) {}

};

class RewriterGenerator {
public:
    PatGen &pg;
    std::vector<std::unique_ptr<State>> states;
    std::map<std::string, PatternOfNode *> machineInstrs;
    RewriterGenerator(PatGen &pg) : pg(pg) {}

    std::ostream &error() {
        return std::cout << "ERROR: ";
    }
public:
    void generate() {
        //inferTypes();
        generateRewriters();
        generateOpcodes();
    }

    void inferTypes() {
        for (auto &MI: pg.machineInstrs) {
            if (MI.getNode()->getOpcode().empty()) {
                error() << "Empty instruction name " << MI.getNode()->loc << std::endl;
                continue;
            }
            machineInstrs[MI.getNode()->getOpcode()] = MI.getNode();
        }

        for (auto &Rewriter: pg.rewriters) {
            inferTypeForRewriter(Rewriter);
        }

        reportTypeErrorForMachineInstr();
    }

    bool isMachineInstr(const std::string &opcode) {
        return machineInstrs.find(opcode) != machineInstrs.end();
    }
    const std::string &getMachineInstrType(const std::string &opcode, size_t index) {
        auto MI = machineInstrs.find(opcode);
        if (MI == machineInstrs.end()) {
            error() << "Unknown instruction " << opcode << std::endl;
        }
        if (index >= machineInstrs[opcode]->getNodeSize()) {
            error() << "Instruction " << opcode << " has no " << index << "th argument" << std::endl;
        }
        return machineInstrs[opcode]->nodes[index]->getType();
    }
    const std::string &getMachineInstrRetType(const std::string &opcode) {
        return machineInstrs[opcode]->getType();
    }

    void reportTypeErrorForMachineInstr() {
        for (auto &[opcode, pattern]: machineInstrs) {
            if (pattern->isPatternLeaf()) {
                error() << "The Instruction is not a valid! " << pattern->loc << std::endl;
                return;
            }
            for (int I = 0; I < pattern->getNodeSize(); ++I) {
                auto *Node = pattern->nodes[I];
                if (!Node->isPatternLeaf()) {
                    error() << "The Instruction arg is not a valid! " << Node->loc << std::endl;
                    continue;
                }

                if (Node->isEmptyType()) {
                    error() << "The Instruction arg must have a type! " << Node->loc << std::endl;
                }
            }
        }
    }

    void inferTypeForRewriter(Rewriter &rewriter) {
        std::map<std::string, std::vector<PatternOfNode *>> TypeForNode;
        std::map<std::string, std::string> Types;

        std::function<void(PatternOfNode *)> InferPattern = [&](PatternOfNode *node) {
            if (!node->isPatternLeaf()) {
                if (isMachineInstr(node->getOpcode())) {
                    ///< If the node is a machine instruction,
                    ///< set the type of the node to the type of the machine instruction.
                    ///< If the type is different, report error.
                    if (node->isEmptyType()) {
                        node->setType(getMachineInstrRetType(node->getOpcode()));
                    }
                    for (int I = 0; I < node->getNodeSize(); ++I) {
                        auto *Node = node->nodes[I];
                        auto &TypeDef = getMachineInstrType(node->getOpcode(), I);
                        if (Node->isEmptyType()) {
                            Node->setType(TypeDef);
                        } else {
                            ///< Check if the type is equal to the definition of the instruction
                            if (Node->getType() != TypeDef) {
                                error() << "The type is not matched with the definition! " << Node->loc << std::endl;
                                continue;
                            }
                        }
                    }
                }
                for (auto *Child: node->nodes) {
                    InferPattern(Child);
                }
            }

            if (node->isPatternLeaf()) {
                if (node->isEmptyType()) {
                    TypeForNode[node->getArgName()].push_back(node);
                } else {
                    if (Types.count(node->getArgName())) {
                        if (Types[node->getArgName()] != node->getType()) {
                            error() << "The type is not matched with the definition! " << node->loc << std::endl;
                        }
                    } else {
                        Types[node->getArgName()] = node->getType();
                    }
                }
            }
        };
        InferPattern(rewriter.getRewriter());
        InferPattern(rewriter.getPattern());

        for (auto &[Name, Nodes]: TypeForNode) {
            auto &Type = Types[Name];
            if (Type.empty()) {
                error() << "The type is not defined! " << Nodes[0]->loc << std::endl;
                continue;
            }
            for (auto *Node: Nodes) {
                Node->setType(Type);
            }
        }

        if (rewriter.getPattern()->isEmptyType()) {
            rewriter.getPattern()->setType(rewriter.getRewriter()->getType());
        }

    }

    void generateOpcodes() {

    }

    void generateRewriters() {
        std::set<PatternApplyer> Nodes;
        for (auto &Rewriter: pg.rewriters) {
            Nodes.insert(PatternApplyer(Rewriter.getPattern(), &Rewriter));
        }
        auto *Start = generateState(Nodes);
        int VisitCount = 0;
        while (VisitCount < states.size()) {
            generateTransition(states[VisitCount++].get());
        }
    }

    void generateTransition(State *state) {
        if (state->isMatch) {
            return;
        }
        ///< Generate match leaf
        for (auto &PG: state->gotos) {
            if (PG.isMatched()) {
                state->transitions.emplace_back(Transition::MatchLeaf, PG.getPattern(), PG.getRewriter());
            }
        }

        ///< Generate match opcode
        std::set<std::string> Opcodes;
        for (auto &PA: state->gotos) {
            Opcodes.insert(PA.getPattern()->getOpcode());
        }

        for (auto &Opcode: Opcodes) {
            if (Opcode.empty()) {
                continue;
            }
            auto *State = generateMatch(state, Opcode);
            state->transitions.emplace_back(Transition::MatchOpcode, Opcode, State);
        }

    }

    State *generateMatch(State *state, const std::string &opcode) {
        std::set<PatternApplyer> Gotos;
        size_t MaxEnterIndex = 0;
        for (auto &PA: state->gotos) {
            if (PA.getPattern()->getOpcode() == opcode) {
                MaxEnterIndex = std::max(MaxEnterIndex, PA.getPattern()->getNodeSize());
                Gotos.insert(PA);
            }
        }

        ///< Generate enter
        auto *State = generateState(Gotos);
        for (int I = 0; I < MaxEnterIndex; ++I) {
            auto *EnterState = generateEnter(State, I);
            State->transitions.emplace_back(Transition::Enter, I, EnterState);
        }

        ///< Generate match
        /*for (auto &PG: Gotos) {
            if (PG.isMatched()) {
                State->transitions.emplace_back(Transition::MatchLeaf, PG.getPattern(), PG.getRewriter());
            }
        }*/
        State->isMatch = true;
        return State;
    }

    State *generateEnter(State *state, int index) {
        std::set<PatternApplyer> Gotos;
        for (auto &PA: state->gotos) {
            if (index < PA.getPattern()->getNodeSize()) {
                Gotos.insert(PA.enter(index));
            }
        }
        auto *State = generateState(Gotos);
        return State;
    }

    State *generateState(std::set<PatternApplyer> &gotos) {
        for (auto &State: states) {
            if (State->gotos == gotos) {
                return State.get();
            }
        }
        auto *NewState = new State(gotos);
        NewState->index = states.size();
        states.emplace_back(NewState);
        return NewState;
    }

};

#endif //DRAGON_PATGEN_H
