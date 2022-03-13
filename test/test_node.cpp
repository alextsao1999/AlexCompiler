//
// Created by Alex on 2022/3/12.
//
#include "lest.hpp"
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

const lest::test Specification[] = {
    CASE("NodeList") {
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

            EXPECT(Join(List) == "1,2,3,4,5");

            auto It = List.begin() + 1;
            auto ItEnd = NodeListImpl::iterator(It) + 3;
            List.remove(It, ItEnd);
            EXPECT(Join(List) == "1,5");

            List.inject_before(List.begin() + 1, It.getPointer());
            EXPECT(Join(List) == "1,2,3,4,5");

            List.remove(It, ItEnd);
            List.inject_after(List.begin(), It.getPointer());
            EXPECT(Join(List) == "1,2,3,4,5");

            auto MoveOut = [&](size_t i, size_t n) {
                auto It = List.begin() + i;
                auto ItEnd = It + n;
                List.remove(It, ItEnd);
                return It;
            };

            NodeListImpl NewList;

            auto I = MoveOut(1, 2);
            EXPECT(Join(List) == "1,4,5");

            NewList.inject_before(NewList.end(), I.getPointer());
            EXPECT(Join(NewList) == "2,3");

            NewList.remove(I, NewList.end());
            EXPECT(Join(NewList) == "");

            NewList.inject_after(NewList.end(), I.getPointer());
            EXPECT(Join(NewList) == "2,3");

            NewList.remove(I, NewList.end());
            List.inject_after(List.begin(), I.getPointer());
            EXPECT(Join(List) == "1,2,3,4,5");

            I = List.begin();
            List.remove(List.begin(), List.end());
            NewList.inject_after(NewList.end(), I.getPointer());
            EXPECT(Join(NewList) == "1,2,3,4,5");

            I = NewList.begin();
            NewList.remove(I);
            List.inject_after(List.begin(), I.getPointer());
            EXPECT((Join(List) == "1" && Join(NewList) == "2,3,4,5"));

            I = NewList.begin();
            NewList.remove(I);
            List.inject_before(List.end(), I.getPointer());
            EXPECT(Join(List) == "1,2");
            EXPECT(Join(NewList) == "3,4,5");

        },

};

int main(int argc, char *argv[]) {
    return lest::run(Specification, argc, argv, std::cout);
}