#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(

        (var x 10)

        (def foo () x)

        (begin 
            (var y 100)
            (var q 300)
            q
            (+ y x)
            (begin
                (var z 200)
                z
                (def bar () (+ y z))
                (bar)
            )
        )
    
    )");

    // vm.dumpStack();

    std::cout << "\n";

    log(result);

    std::cout << "All Done!\n";

    return 0;
}