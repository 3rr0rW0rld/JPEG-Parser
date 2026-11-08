#include <iostream>
#include <print>

#include "JPEGParser.h"

int main(){
    JPEGParser parser {"images/input/profile.jpg"};
    
    while (!parser.isOnScan()){
        parser.parseSegment();
    }

    std::println("JPEG INFO: ");
    std::println("Image dimensions: {}x{}", parser.getWidth(), parser.getHeight());

    for (int id{1}; id <= parser.getNbColorComponents(); ++id){
        const ColorComponentSpecs& spec {parser.getComponentSpec(id)};
        std::println();
        std::println("id={} dc={} ac={} h={} v={} qtId={}", spec.id, spec.dcEntropyCodingSelector, spec.acEntropyCodingSelector, spec.horizontalSamplingRatio, spec.verticalSamplingRatio, spec.quantizationTableIndex);
    }

    for (int i{}; i < 2; ++i){
        const QuantizationTable& quantTable {parser.getQuantizationTable(i)};
        std::println();
        std::println("ID={}", quantTable.id);
        for (int i{}; i < 64; ++i){
            std::print("{0:#x} ", quantTable.values.getByte(i));
        }
    }



    parser.parseScan();

    parser.dequantize();

    //for (int i{}; i < 4; ++i){
        //const HuffmanTable& huffTable {parser.getHuffmanDecoder(i).getHuffmanTable()};

        //std::println();
        //std::println("ID={} TYPE={}", huffTable.id, (huffTable.isAC ? "AC" : "DC"));
        //for (int i{}; i < 16; ++i){
            //std::print("{} : ", i+1);
            //for (int j{}; j < huffTable.codes[i].size(); ++j){
                //std::print("{0:#x} ", huffTable.codes[i].getByte(j));
            //}
            //std::println();
        //}
    //}

    std::println();


}