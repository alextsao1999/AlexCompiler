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

    auto It = List.begin() + 1;
    auto ItEnd = NodeListImpl::iterator(It) + 3;
    List.extract(It, ItEnd);
    EXPECT_EQ(Join(List), "1,5");

    List.inject_before(List.begin() + 1, It.getPointer());
    EXPECT_EQ(Join(List), "1,2,3,4,5");

    List.extract(It, ItEnd);
    List.inject_after(List.begin(), It.getPointer());
    EXPECT_EQ(Join(List), "1,2,3,4,5");

    auto MoveOut = [&](size_t i, size_t n) {
        auto It = List.begin() + i;
        auto ItEnd = It + n;
        List.extract(It, ItEnd);
        return It;
    };

    NodeListImpl NewList;

    auto I = MoveOut(1, 2);
    EXPECT_EQ(Join(List), "1,4,5");

    NewList.inject_before(NewList.end(), I.getPointer());
    EXPECT_EQ(Join(NewList), "2,3");

    NewList.extract(I, NewList.end());
    EXPECT_EQ(Join(NewList), "");

    NewList.inject_after(NewList.end(), I.getPointer());
    EXPECT_EQ(Join(NewList), "2,3");

    NewList.extract(I, NewList.end());
    List.inject_after(List.begin(), I.getPointer());
    EXPECT_EQ(Join(List), "1,2,3,4,5");

    I = List.begin();
    List.extract(List.begin(), List.end());
    NewList.inject_after(NewList.end(), I.getPointer());
    EXPECT_EQ(Join(NewList), "1,2,3,4,5");

    I = NewList.begin();
    NewList.extract(I);
    List.inject_after(List.begin(), I.getPointer());
    EXPECT_EQ(Join(List), "1");
    EXPECT_EQ(Join(NewList), "2,3,4,5");

    I = NewList.begin();
    NewList.extract(I);
    List.inject_before(List.end(), I.getPointer());
    EXPECT_EQ(Join(List), "1,2");
    EXPECT_EQ(Join(NewList), "3,4,5");

}
