#include <iostream>
#include "./Logger.h"
#include "vm/xp.h"
#include "./vm/XPValue.h"

int main(int argc, char const *argv[])
{
    XPVM vm;

    auto result = vm.exec(R"(

        55555

    )");

    log(AS_NUMBER(result));

    std::cout << "All Done!\n";

    return 0;
}