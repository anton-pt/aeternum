//
// Created by Anton Tcholakov on 2019-07-20.
//

#include <iostream>

#include "atom.h"

int main()
{
    const aeternum::atom hello("hello");
    const aeternum::atom goodbye("goodbye");

    std::cout << "Comparing two different atoms: " << (hello == goodbye) << std::endl;

    return 0;
}