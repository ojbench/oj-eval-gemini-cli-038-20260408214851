#include "vector.hpp"
#include <iostream>

int main() {
    sjtu::vector<int> v;
    int x;
    while (std::cin >> x) {
        v.push_back(x);
    }
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}
