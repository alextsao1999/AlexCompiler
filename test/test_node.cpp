//
// Created by Alex on 2022/3/12.
//
#include "gtest/gtest.h"
#include "Node.h"

class Element : public Node<Element> {
public:
    int value = 0;
    Element(int value) : value(value) {}
    ~Element() {
        //std::cout << "~Element() " << value << std::endl;
    }
};

using NodeListImpl = INodeListImpl<Element, ListAllocTrait<Element>>;

size_t findstr(const std::string &str, const std::string &pattern) {
    if ((pattern.size() == 0) || (str.size() < pattern.size()))
        return std::string::npos;
    int *Nexts = (int *) alloca(sizeof(int) * pattern.size());
    Nexts[0] = 0;
    unsigned Cursor = 0, Matching = 1;
    while (Matching < pattern.size()) {
        while (Cursor && pattern[Cursor] != pattern[Matching]) {
            Cursor = Nexts[Cursor - 1];
        }
        if (pattern[Cursor] == pattern[Matching])
            Cursor++;
        Nexts[Matching++] = Cursor;
    }
    Matching = 0;
    for (Cursor = 0; Cursor < str.size(); ++Cursor) {
        while (Matching && str[Cursor] != pattern[Matching]) {
            Matching = Nexts[Matching - 1]; // backtrack
        }
        if (str[Cursor] == pattern[Matching])
            Matching++;
        if (Matching == pattern.size()) {
            return Cursor - pattern.size() + 1;
        }
    }
    return std::string::npos;
}

TEST(NodeList, Test) {
    NodeListImpl List;
    List.push_back(new Element(1));
    List.push_back(new Element(2));
    List.push_back(new Element(3));
    List.push_back(new Element(4));
    List.push_back(new Element(5));

    auto Join = [&](auto &list) {
        std::stringstream SS;
        int I = 0;
        for (auto &E : list) {
            if (I++ > 0)
                SS << ",";
            SS << E.value;
        }
        return SS.str();
    };

    EXPECT_EQ(Join(List), "1,2,3,4,5");

    // test iterator
    auto Iter = List.begin();
    EXPECT_EQ(Iter->value, 1);
    EXPECT_EQ((++Iter)->value, 2);
    EXPECT_EQ((++Iter)->value, 3);
    EXPECT_EQ((++Iter)->value, 4);
    EXPECT_EQ((++Iter)->value, 5);
    EXPECT_EQ(++Iter, List.end());

    Iter = List.begin();
    EXPECT_EQ((Iter + 0)->value, 1);
    EXPECT_EQ((Iter + 1)->value, 2);
    EXPECT_EQ((Iter + 2)->value, 3);
    EXPECT_EQ((Iter + 3)->value, 4);
    EXPECT_EQ((Iter + 4)->value, 5);
    EXPECT_EQ(Iter + 5, List.end());

    Iter = List.end();
    EXPECT_EQ((--Iter)->value, 5);
    EXPECT_EQ((--Iter)->value, 4);
    EXPECT_EQ((--Iter)->value, 3);
    EXPECT_EQ((--Iter)->value, 2);
    EXPECT_EQ((--Iter)->value, 1);
    EXPECT_EQ(Iter, List.begin());

    Iter = List.begin() + 2;
    List.erase(Iter);
    EXPECT_EQ(Join(List), "1,2,4,5");

    List.erase(List.begin(), List.end());
    EXPECT_EQ(Join(List), "");

}
