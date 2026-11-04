#include "JPEGParser.h"

#include <print>

JPEGParser::JPEGParser(std::string_view filePathName){
    reader.loadFile(filePathName);
}

void JPEGParser::skipSegment() {
    int headerLength {reader.readBytes(2)};
    reader.readBytes(headerLength-2);
}

void JPEGParser::parseSegment(){
    ByteArray markerBytes {reader.readBytes(2)};
    std::println("{0:#x}", static_cast<int>(markerBytes));

    MarkerType markerType {markerBytes.getByte(1)};

    switch (markerType){

    case MarkerType::SOS:
        parseStartOfScanHeader();
        break;

    case MarkerType::S0F0:
        parseStartOfFrameHeader();
        break;

    case MarkerType::DQT:
        parseQuantizationTable();
        break;

    case MarkerType::DHT:
        parseHuffmanTable();
        break;

    case MarkerType::SOI:
        break;
    case MarkerType::EOI:
        break;

    default:
        skipSegment();
        
    };
}

void JPEGParser::parseHuffmanTable(){
    int headerLength {reader.readBytes(2)};

    int tableInfo {reader.readByte()};
    bool isAC {static_cast<bool>(tableInfo >> 4)};
    int id {tableInfo & 0x0F};
    
    ByteArray codeByLength {reader.readBytes(16)};

    std::array<ByteArray, 16> codes {};
    for (int i {}; i < 16; ++i){
        codes[i] = reader.readBytes(codeByLength.getByte(i));
    }
    
    huffmanTables.push_back(HuffmanTable {id, isAC, codes});

}

void JPEGParser::parseQuantizationTable(){
    int headerLength {reader.readBytes(2)};

    int id {reader.readByte()};
    ByteArray quantizationValues {reader.readBytes(64)};

    quantizationTables.push_back(QuantizationTable {id, quantizationValues});
}

void JPEGParser::parseStartOfScanHeader(){
    onScan = true;

    int headerLength {reader.readBytes(2)};

    nbColorComponents = reader.readByte();

    for (int i {}; i < nbColorComponents; ++i){
        int id {reader.readByte()};
        if (colorComponentSpecs.find(id) == colorComponentSpecs.end()){
            colorComponentSpecs.insert({id, ColorComponentSpecs{id}});
        }

        byte entropyCodingSelector {reader.readByte()};
        colorComponentSpecs[id].dcEntropyCodingSelector = entropyCodingSelector >> 4;
        colorComponentSpecs[id].acEntropyCodingSelector = entropyCodingSelector & 0x0F;
    }

    spectralSelectionStart = reader.readByte();
    spectralSelectionEnd = reader.readByte();
    byte bitPosition {reader.readByte()};

    precedingBitPosition = bitPosition >> 4;
    currentBitPosition = bitPosition & 0x0F;

}

void JPEGParser::parseStartOfFrameHeader(){
    int headerLength {reader.readBytes(2)};
 
    samplePrecision = reader.readByte();
    height = reader.readBytes(2);
    width = reader.readBytes(2);
    nbColorComponents = reader.readByte();

    for (int i {}; i < nbColorComponents; ++i){
        int id {reader.readByte()};
        if (colorComponentSpecs.find(id) == colorComponentSpecs.end()){
            colorComponentSpecs.insert({id, ColorComponentSpecs{id}});
        }

        byte samplingRatio {reader.readByte()};
        colorComponentSpecs[id].horizontalSamplingRatio = samplingRatio >> 4;
        colorComponentSpecs[id].verticalSamplingRatio = samplingRatio & 0x0F;

        colorComponentSpecs[id].quantizationTableIndex = reader.readByte();
    }
    
}