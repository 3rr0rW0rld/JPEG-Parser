#pragma once

#include <unordered_map>

#include "ByteFileReader.h"

enum class MarkerType : uint8_t{
    SOI = 0xD8,
    S0F0 = 0xC0,
    S0F2 = 0xC2,
    DHT = 0xC4,
    DQT = 0xDB,
    DAC = 0xCC,
    DRI = 0xDD,
    SOS = 0xDA,
    RSTn = 0xD0,
    APPn = 0xE0,
    COM = 0xFE,
    EOI = 0xD9
};

struct ColorComponentSpecs {
    int id {};

    int acEntropyCodingSelector {};
    int dcEntropyCodingSelector {};

    int horizontalSamplingRatio {};
    int verticalSamplingRatio {};

    int quantizationTableIndex {};
};

struct QuantizationTable {
    int id {};
    ByteArray values {};
};

struct HuffmanTable {

    int id {};
    bool isAC {false};

    std::array<ByteArray, 16> codes {};

};



class JPEGParser {
private:
    ByteFileReader reader {};


    int width {};
    int height {};

    int samplePrecision {8};

    int nbColorComponents {3};

    int spectralSelectionStart {0};
    int spectralSelectionEnd {0};
    int precedingBitPosition {0};
    int currentBitPosition {0};

    std::unordered_map<int, ColorComponentSpecs> colorComponentSpecs {};

    std::vector<QuantizationTable> quantizationTables {};
    std::vector<HuffmanTable> huffmanTables {};

    bool onScan {false};


public:
    JPEGParser(std::string_view filePathName);
    ~JPEGParser() = default;

    void parseSegment();

    void parseStartOfScanHeader();
    void parseStartOfFrameHeader();
    void parseHuffmanTable();
    void parseQuantizationTable();

    void skipSegment();

    bool isOnScan() const {return onScan;}

    int getWidth() const {return width;}
    int getHeight() const {return height;}
    int getNbColorComponents() const {return nbColorComponents;}
    const ColorComponentSpecs& getComponentSpec(int id) {return colorComponentSpecs[id];}
    const QuantizationTable& getQuantizationTable(int index) {return quantizationTables[index];}
    const HuffmanTable& getHuffmanTable(int index) {return huffmanTables[index];}
};