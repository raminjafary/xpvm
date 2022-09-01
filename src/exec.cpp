#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(
        (var x 5)
        (set x (+ x 10))
        
        x

        (begin 
            (var z 100)
            (set x 1000)
            (begin 
                (var x 200)
            z)
        x)

        x

    )");

    std::cout << "\n";

    log(result);

    std::cout << "All Done!\n";

    return 0;
}