#pragma once

#include <filesystem>
#include <fstream>

#include <string>
#include <vector>
#include <print>

using byte = uint8_t;
using byte_list = std::vector<byte>;

struct ByteArray {

private:
    byte_list bytes {};

public:
    byte getByte(int index) const { return bytes[index]; }
    const byte_list& getBytes() const {return bytes;}

    void append(byte b) {bytes.push_back(b);}
    int size() const {return bytes.size();}

    void print(){
        for (byte b : bytes){
            std::print("{0:#x} ", b);
        }
        std::println();
    }

    operator int(){
        int result {};
        int shiftCount {0};

        for (auto it {bytes.rbegin()}; it != bytes.rend(); ++it){
            result |= (*it) << shiftCount;
            shiftCount += 8;
        }
        
        return result;

    }
};

class ByteFileReader{
private:
    std::filesystem::path currentPath{};
    std::ifstream fileStream {};

    byte currentByte {};
    int bitIndex {};

public:
    ByteFileReader() = default;
    ByteFileReader(std::string_view filePathName);

    ~ByteFileReader() = default;

    void loadFile(std::string_view filePathName);
    void loadFile(const std::filesystem::path& filePath);

    std::string getFileName() const { return currentPath.filename().string(); }
    bool isOpen() const { return fileStream.is_open(); }
    bool hasEnded() const { return fileStream.eof(); }
    
    bool readBit();
    ByteArray readBits(int count);

    byte readByte();
    ByteArray readBytes(int count);

    void skipToEndOfByte();
};