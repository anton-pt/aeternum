//
// Created by Anton Tcholakov on 2019-07-20.
//

#include <iostream>
#include "immer/set.hpp"


#include "atom.h"
#include "tagged.h"

const aeternum::atom apples("apples");
const aeternum::atom oranges("oranges");

int fruit_count(const aeternum::tagged_untyped& tagged)
{
    if (auto as = tagged.match<apples, int>())
    {
        return *as;
    }
    if (auto os = tagged.match<oranges, int>())
    {
        return *os;
    }

    return 0;
}

int main()
{
    const auto four_apples = aeternum::make_tagged(apples, 4);
    const auto four_oranges = aeternum::make_tagged(oranges, 4);
    const auto five_apples = aeternum::make_tagged(apples, 5);
    const auto four_more_oranges = aeternum::make_tagged(oranges, 4);

    std::cout << "The hash of 4 apples is: " << std::hash<aeternum::tagged_untyped>{}(four_apples) << std::endl;
    std::cout << "The hash of 4 oranges is: " << std::hash<aeternum::tagged_untyped>{}(four_oranges) << std::endl;
    std::cout << "The hash of 5 apples is: " << std::hash<aeternum::tagged_untyped>{}(five_apples) << std::endl;
    std::cout << "The hash of 4 more oranges is: " << std::hash<aeternum::tagged_untyped>{}(four_more_oranges) << std::endl;

    std::cout << std::endl;

    std::cout << "Comparing 4 apples and 4 oranges: " << (four_apples == four_oranges) << std::endl;
    std::cout << "Comparing 4 apples and 5 apples: " << (four_apples == five_apples) << std::endl;
    std::cout << "Comparing 4 oranges and 4 more oranges: " << (four_oranges == four_more_oranges) << std::endl;

    std::cout << std::endl;

    immer::set<aeternum::tagged_untyped> fruit;
    fruit = fruit.insert(four_apples);
    fruit = fruit.insert(four_oranges);
    fruit = fruit.insert(five_apples);
    fruit = fruit.insert(four_more_oranges);

    for (auto& f : fruit)
    {
        std::cout << "Set contains " << fruit_count(f) << " " << f.get_tag().name << std::endl;
    }

    return 0;
}