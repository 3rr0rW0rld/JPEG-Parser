#pragma once

#include <vector>
#include <iostream>

template <typename T>
int binaryToDecimal(const std::vector<T>& bytes){
    int result {0};

    int multiplier = 1;
    for (auto it {bytes.rbegin()}; it != bytes.rend(); ++it){
        result += (*it) * multiplier;
        multiplier *= 256;
    }

    return result;
}

template <typename T>
void printNumberInBinary(T n, int nbBits=-1){
    std::string result {};

    if (nbBits <= 0){
        while (n > 0){
            result = ((n&1) == 0 ? '0' : '1') + result;
            n >>= 1;
        }
    }
    else {

        for (int i{}; i < nbBits; ++i){
            result = ((n&1) == 0 ? '0' : '1') + result;
            n >>= 1;
        }

    }
    std::cout << result;
}