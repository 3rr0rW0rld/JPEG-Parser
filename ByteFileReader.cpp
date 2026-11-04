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

byte ByteFileReader::readByte(){
    char result {};
    fileStream.read(&result, 1);

    return static_cast<byte>(result);
}

ByteArray ByteFileReader::readBytes(int count){
    ByteArray result {};

    for (int i{}; i < count; ++i){
        result.append(readByte());
    }

    return result;
}