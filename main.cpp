#include <iostream>
#include <filesystem>
#include <fstream>

#include "JPEGLoader.h"

int main(){
    JPEGLoader jpegLoader {};
    const std::filesystem::path imagePath {"images/input/pexels-image.jpg"};
    jpegLoader.loadFile(imagePath);
    std::cout << '\n';
    jpegLoader.printSegments();


}