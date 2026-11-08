#include "ByteFileReader.h"

ByteFileReader::ByteFileReader(std::string_view filePathName){
    loadFile(filePathName);
}

void ByteFileReader::loadFile(const std::filesystem::path& filePath){
    currentPath = filePath;
    fileStream.open(currentPath);
}
void ByteFileReader::loadFile(std::string_view filePathName){
    currentPath = std::filesystem::path {filePathName};
    fileStream.open(currentPath);
}


bool ByteFileReader::readBit(){
    if (bitIndex >= 8){
        bitIndex = 0;

        char result{};
        fileStream.read(&result, 1);
        currentByte = static_cast<byte>(result);
    }

    return static_cast<bool>((currentByte >> (7-bitIndex++)) & 1);
}

ByteArray ByteFileReader::readBits(int count){
    ByteArray result {};

    for (int i{}; i < count; ++i){
        result.append(static_cast<byte>(readBit()));
    }

    return result;
}

byte ByteFileReader::readByte(){
    char result {};
    fileStream.read(&result, 1);
    currentByte = static_cast<byte>(result);
    bitIndex = 8;

    return currentByte;
}

ByteArray ByteFileReader::readBytes(int count){
    ByteArray result {};

    for (int i{}; i < count; ++i){
        result.append(readByte());
    }

    return result;
}

void ByteFileReader::skipToEndOfByte(){
    if (bitIndex > 0){
        bitIndex = 8;
    }
}