#include "vector.hpp"
#include <iostream>

int main() {
    sjtu::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    std::cout << v.size() << std::endl;
    return 0;
}
