//
// Created by Alex on 2022/3/10.
//

#ifndef DRAGON_PATGEN_H
#define DRAGON_PATGEN_H

#include "parser.h"
#include <fstream>

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
public:
    bool isPatternLeaf() const {
        return isLeaf;
    }

    bool isEmptyType() const {
        return type.empty();
    }

    inline std::string &getArgName() {
        return argName;
    }

    inline std::string &getOpcode() {
        return opcode;
    }

    inline std::vector<PatternOfNode *> &getNodes() {
        return nodes;
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

///< The DFA State of the Pattern Node.
class StateSet {

};
class ItemSet {

};

class PatGen : public Visitor<PatGen> {
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
        Node->type = type;
        storages.emplace_back(Node);
        return Node;
    }
    PatternOfNode *createLeaf(const std::string &argName,const std::string &type = "") {
        PatternOfNode *Node = new PatternOfNode();
        Node->isLeaf = true;
        Node->type = type;
        Node->argName = argName;
        storages.emplace_back(Node);
        return Node;
    }
    PatternOfNode *createNode(const std::string &opcode, const std::vector<PatternOfNode *> &nodes, const std::string &type = "") {
        PatternOfNode *Node = new PatternOfNode();
        Node->isLeaf = false;
        Node->type = type;
        Node->opcode = opcode;
        Node->nodes = nodes;
        storages.emplace_back(Node);
        return Node;
    }

    ///< Parse the patterns from the file.
    bool parse(const char *file) {
        GLRParser<StreamIter> Parser;
        FileStreamWrapper Stream(file);
        Parser.reset(Stream.begin(), Stream.end());
        Parser.parse();
        if (!Parser.accept()) {
            return false;
        }
        visit(Parser.value());
        std::cout << Parser.value().dump(4);
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
        argStack.back()->type = value.getType();
        //auto *Node = createNode()
    }

    void visitArgVar(ArgVar value) override {
        argStack.push_back(createLeaf(value.getVar(), value.getType()));
    }

    void visitRule(Rule value) override {
        visit(value.getArgs());
        auto *Node = createNode(value.getName(), argStack);
        argStack.clear();
        argStack.push_back(Node);
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

#endif //DRAGON_PATGEN_H
