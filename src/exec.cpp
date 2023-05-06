#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(

    //    (squaree 2)

    //     (def sum (a b) 
    //         (begin 
    //             (var x 10)
    //             (+ x (+ a b))
    //         )
    //     )


    //     (sum 2 4)

    //     (def factorial (x) 
    //         (if (== x 1)
    //             1
    //             (* x (factorial (- x 1)))
    //         )   
    //     )
    //     (factorial 5)

    //     (lambda (x) (* x x))
    //     (var sqaure (lambda (x) (* x x)))

    //     ((lambda (x) (* x x)) 2)

    (def sum (x) (* x x))

    // (var x 1)
    // (var z 2)
    // (var y (+ x 1))

    // (begin
    //     (var a 10)
    //     (var b 20)
    //     (set a 100)
    //     (+ a b)
    // )

    )");

    // vm.dumpStack();

    std::cout << "\n";

    log(result);

    std::cout << "All Done!\n";

    return 0;
}