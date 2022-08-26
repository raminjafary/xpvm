#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(

        // (+ 1 (+ 5 4))
        // true
        // false
        // (< 5 6)
        (if (< 5 10) 1 2)

    )");

    log(result);

    std::cout << "All Done!\n";

    return 0;
}