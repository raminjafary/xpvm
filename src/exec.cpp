#include "vm/xp.h"
#include "./Logger.h"
#include "./vm/XPValue.h"

#include <iostream>

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(

        42

    )");

    log(AS_NUMBER(result));

    std::cout << "All Done!\n";

    return 0;
}