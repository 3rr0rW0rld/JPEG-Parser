#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>

#include <string_view>

enum class SegmentType {
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

std::string_view segmentTypeToString(SegmentType segmentType);

struct Segment {
    int byteIndex {};
    SegmentType segmentType {};

    int length {};
};

struct ComponentSpecs {
    int id {};

    int dcEntropyCodingSelector {};
    int acEntropyCodingSelector {};
};

struct SOSHeader {
    int headerLength {};
    int nbComponents {3};

    std::array<ComponentSpecs, 4> componentSpecsParams {};

    int spectralSelectionStart {};
    int spectralSelectionEnd {};

    int precedingBitPosition {};
    int currentBitPosition {};
};

struct QuantizationTable {
    int id {};
    int nbQuantValues {};
    std::array<uint8_t, 256> quantizationValues {};
};

struct EncodedHuffmanValue {
    int value {};
    unsigned int code {};
    int length {};
};

struct HuffmanTable {
    int length{};
    bool isACTable {true};
    int tableNumber {0};

    std::array<int, 16> occurrences {};
    std::vector<uint8_t> values {};

    std::unordered_map<unsigned int, EncodedHuffmanValue> codeMap {};

    bool hasValue(unsigned int code) const {return codeMap.find(code) != codeMap.end();}
    int getCode(unsigned int code) const {return codeMap.find(code)->second.value;}
};



class JPEGLoader {
private:
    std::filesystem::path currentFilePath {};
    std::vector<uint8_t> bytes {};

    std::vector<Segment> segments {};

    SOSHeader sosHeader {};

    std::unordered_map<int, QuantizationTable> quantizationTables {};
    std::vector<HuffmanTable> huffmanTables {};

public:
    JPEGLoader() = default;
    ~JPEGLoader() = default;

    void loadFile(const std::filesystem::path& path);
    void printBytes();
    void printSegments();
    void printSegmentContent(int index);
    void printSegmentContent(SegmentType type, int index);

    std::vector<uint8_t> getBytesFromSegment(const Segment& segment);
    std::vector<uint8_t> getBytesStreamData();
    std::vector<std::vector<uint8_t>> getBytesInBlocks();
    std::vector<uint8_t> getBlock(int byteIndex, int length);
    const HuffmanTable& JPEGLoader::getHuffmanTable(int index, bool isAC);

    const Segment& getSegment(int index) const {return segments[index];}
    const Segment& getSegment(SegmentType type, int index) const {
        auto it = segments.begin();

        int counter {};

        while (it != segments.end()){
            if ((*it).segmentType == type){
                if (counter == index){
                    return *it;
                }
                else{
                    ++counter;
                }
            }
            ++it;
        }

        return *it;
    }

    std::vector<Segment> getSegments(SegmentType type){
        std::vector<Segment> result {};

        for (const auto& segment : segments){
            if (segment.segmentType == type){
                result.push_back(segment);
            }
        }

        return result;
    }

    void loadStartOfScanHeader();
    void loadQuantizationTables();
    void loadHuffmanTables();

};