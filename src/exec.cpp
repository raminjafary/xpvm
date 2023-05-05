#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(
        (def add (x) (+ x x))

        (def sum (a b) 
            (begin 
                (var x 10)
                (+ x (+ a b))
            )
        )


        // (sum 2)

        // (def factorial (x) 
        //     (if (== x 1)
        //         1
        //         (*x (factorial (-x 1)))
        //     )   
        // )
        // (factorial 5)
    )");

    // vm.dumpStack();

    std::cout << "\n";

    log(result);

    std::cout << "All Done!\n";

    return 0;
}