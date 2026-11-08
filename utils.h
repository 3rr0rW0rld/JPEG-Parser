#pragma once

#include <print>
#include <string>

std::string formatBinary(int byte, int length){
    std::string output {};
    for (int i{}; i < length; ++i){
        output = (((byte & 0b1) == 0) ? '0' : '1') + output;
        byte >>= 1;
    }
    return "0b" + output;
}