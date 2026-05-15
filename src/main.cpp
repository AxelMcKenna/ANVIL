#include <iostream>

#include "anvil/version.hpp"

int main() {
    std::cout << "ANVIL " << anvil::version() << '\n';
    return 0;
}

