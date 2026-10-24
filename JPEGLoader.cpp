#include "JPEGLoader.h"
#include "binary-converter.h"
#include "huffman.h"

#include <iostream>

void JPEGLoader::loadFile(const std::filesystem::path& path)
{
    currentFilePath = path;
    std::ifstream fileStream {currentFilePath, std::ios::binary};

    bool isMarker {false};

    if (fileStream.is_open()){
        while (!fileStream.eof()){
            unsigned char byte {};
            
            fileStream.read(reinterpret_cast<char*>(&byte), sizeof(byte));

            if (byte == 0xFF){
                isMarker = true;
            }
            else if (isMarker && byte != 0x00){
                unsigned char byteType {byte};


                if (0xD0 <= byteType && byteType <= 0xD7){
                    byteType = 0xD0;
                }
                else if (0xE0 <= byteType && byteType<= 0xEF){
                    byteType = 0xE0;
                }
                
                segments.push_back(Segment{static_cast<int>(bytes.size()+1), static_cast<SegmentType>(byteType)});
                isMarker = false;
            }
            else {
                isMarker = false;
            }

            bytes.push_back(byte);
        }
    }

    fileStream.close();

    // Update segments length
    for (auto& segment : segments){
        if (segment.segmentType != SegmentType::SOI && segment.segmentType != SegmentType::EOI){
            segment.length = binaryToDecimal(std::vector<uint8_t>{bytes[segment.byteIndex], bytes[segment.byteIndex+1]});
        }
        else {
            segment.length = 0;
        }
    }

    loadStartOfScanHeader();
    loadHuffmanTables();
    loadQuantizationTables();
}

void JPEGLoader::loadStartOfScanHeader() {

    const Segment& sosSegment {getSegment(SegmentType::SOS, 0)};
    auto sosBytes {getBytesFromSegment(sosSegment)};

    sosHeader.headerLength = sosSegment.length;
    sosHeader.nbComponents = sosBytes[2];
    for (int i{}; i < sosHeader.nbComponents; ++i){
        sosHeader.componentSpecsParams[i].id = sosBytes[3+2*i];
        uint8_t entropyCodingSelector {sosBytes[4+2*i]};
        sosHeader.componentSpecsParams[i].dcEntropyCodingSelector = (entropyCodingSelector & 0xF0) >> 4;
        sosHeader.componentSpecsParams[i].acEntropyCodingSelector = entropyCodingSelector & 0x0F;
    }
    
    sosHeader.spectralSelectionStart = sosBytes[3 + 2*sosHeader.nbComponents];
    sosHeader.spectralSelectionEnd = sosBytes[4 + 2*sosHeader.nbComponents];
    uint8_t bitPositions {sosBytes[5 + 2*sosHeader.nbComponents]};
    sosHeader.precedingBitPosition = (bitPositions & 0xF0) >> 4;
    sosHeader.currentBitPosition = (bitPositions & 0x0F);
}

void JPEGLoader::loadQuantizationTables(){
    std::vector<Segment> quantizationHeaders {getSegments(SegmentType::DQT)};

    for (int i {}; i < quantizationHeaders.size(); ++i){
        QuantizationTable newQuantizationTable {};

        std::vector<uint8_t> quantBytes {getBytesFromSegment(quantizationHeaders[i])};
        newQuantizationTable.id = (quantBytes[2] & 0x0F);
        
        int precision {(quantBytes[2] & 0xF0) >> 4};

        newQuantizationTable.nbQuantValues = (precision == 0) ? 8 : 16;

        int N {newQuantizationTable.nbQuantValues};
        for (int i {}; i < N*N; ++i){
            newQuantizationTable.quantizationValues[i] = quantBytes[3 + i];
        }
        quantizationTables.insert({newQuantizationTable.id, newQuantizationTable});
    }


}

void JPEGLoader::loadHuffmanTables(){
    auto huffmanSegments {getSegments(SegmentType::DHT)};

    for (const auto& segment : huffmanSegments){
        HuffmanTable table {};
        std::vector<uint8_t> huffmanBytes {getBytesFromSegment(segment)};

        table.length = binaryToDecimal(std::vector<uint8_t>{huffmanBytes[0], huffmanBytes[1]});
        uint8_t huffmanInfo {huffmanBytes[2]};

        table.isACTable = huffmanInfo & 0x10;
        table.tableNumber = huffmanInfo & 0x0F;

        for (int i{}; i < 16; ++i){
            table.occurrences[i] = huffmanBytes[2+i+1];
        }

        for (int i{19}; i < huffmanBytes.size(); ++i){
            table.values.push_back(huffmanBytes[i]);
        }
        
        huffmanTables.push_back(table);

        std::vector<HuffmanNode> huffmanNodes {};


        int occurrenceIndex {0};
        int sameOccurrenceCounter {0};
        for (uint8_t byte : table.values){
            while (table.occurrences[occurrenceIndex] <= sameOccurrenceCounter){
                ++occurrenceIndex;
                sameOccurrenceCounter = 0;
            }

            huffmanNodes.push_back(HuffmanNode{static_cast<int>(byte), occurrenceIndex+1});
            ++sameOccurrenceCounter;
        }

        std::unique_ptr<HuffmanNode> root {generateHuffmanTree(huffmanNodes)};
        std::vector<std::pair<EncodedHuffmanValue, HuffmanNode*>> stack {{EncodedHuffmanValue{}, root.get()}};


        while (!stack.empty()){
            auto currentNode {*(stack.end()-1)};
            stack.pop_back();

            if (currentNode.second->value != -1){
                currentNode.first.value = static_cast<int>(currentNode.second->value);
                table.codeMap.insert({currentNode.first.code, currentNode.first});
            }

            if (currentNode.second->left){
                EncodedHuffmanValue newEncodedVal {currentNode.first};
                ++newEncodedVal.length;
                newEncodedVal.code <<= 1;

                stack.push_back({newEncodedVal, currentNode.second->left});
            }

            if (currentNode.second->right){
                EncodedHuffmanValue newEncodedVal {currentNode.first};
                ++newEncodedVal.length;
                newEncodedVal.code = (newEncodedVal.code << 1) | 1;

                stack.push_back({newEncodedVal, currentNode.second->right});
            }
        }


    }
}

void JPEGLoader::printBytes(){
    std::cout << std::hex;
    int counter {0};
    for (auto byte : bytes){
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        std::cout << ' ';
        counter ++;
        if (counter == 16){
            counter = 0;
            std::cout << '\n';
        }
    }
    std::cout << '\n';
}

void JPEGLoader::printSegments(){
    std::cout << std::dec;
    std::cout << "Number of segments: " << segments.size() << '\n';
    for (const Segment& segment : segments){
        std::cout << segment.byteIndex << '\t' << segmentTypeToString(segment.segmentType) << ", " << segment.length << '\n';
    }
}

void JPEGLoader::printSegmentContent(int index){
    const Segment& segment {getSegment(index)};
    
    std::cout << "Segment type: " << segmentTypeToString(segment.segmentType);
    std::cout << ' ' <<  segment.byteIndex << '-' << segment.byteIndex+segment.length << '\n';

    std::vector<uint8_t> quantizationBytes {getBytesFromSegment(segment)};
    std::cout << std::hex;
    for (auto byte : quantizationBytes){
        std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(byte) << ' ';
    }
    std::cout << '\n';
}

void JPEGLoader::printSegmentContent(SegmentType segmentType, int index){
    const Segment& segment {getSegment(segmentType, index)};
    
    std::cout << "Segment type: " << segmentTypeToString(segment.segmentType);
    std::cout << ' ' <<  segment.byteIndex << '-' << segment.byteIndex+segment.length << '\n';

    std::vector<uint8_t> quantizationBytes {getBytesFromSegment(segment)};
    std::cout << std::hex;
    for (auto byte : quantizationBytes){
        std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(byte) << ' ';
    }
    std::cout << '\n';
}

std::vector<uint8_t> JPEGLoader::getBytesFromSegment(const Segment& segment){
    auto start = segment.byteIndex + bytes.begin();
    auto end = segment.byteIndex + segment.length + bytes.begin();

    std::vector<uint8_t> result (segment.length);
    std::copy(start, end, result.begin());
    return result;
}

std::vector<uint8_t> JPEGLoader::getBytesStreamData(){
    const Segment& sosSegment {getSegment(SegmentType::SOS, 0)};
    
    std::vector<uint8_t> result {};

    bool isMarker {false};
    auto it {sosSegment.byteIndex + sosSegment.length + bytes.begin()};
    while (it != bytes.end()){
        if ((*it) == 0xFF){
            isMarker = true;
        }
        else if (isMarker && (*it) == 0x00){
            isMarker = false;
            ++it;
            continue;
        }
        else if (isMarker){
            break;
        }

        result.push_back((*it));
        ++it;
    }

    if (*(result.end()-1) == 0xFF){
        result.pop_back();
    }

    return result;
}


std::vector<uint8_t> JPEGLoader::getBlock(int byteIndex, int length){
    
    std::vector<uint8_t> block {};
    int i {0};

    int currentIndex {byteIndex};
    int currentByte {bytes[byteIndex]};

    int currentBitIndex {0};

    while (i < length*length){
    }

}

std::vector<std::vector<uint8_t>> JPEGLoader::getBytesInBlocks(){

    std::vector<std::vector<uint8_t>> blocks {};
    std::vector<uint8_t> bytes {getBytesStreamData()};



}

const HuffmanTable& JPEGLoader::getHuffmanTable(int id, bool isAC){
    for (auto huffmanTable : huffmanTables){
        if (huffmanTable.tableNumber == id && huffmanTable.isACTable == isAC){
            return huffmanTable;
        }
    }

    return huffmanTables[0];
}


std::string_view segmentTypeToString(SegmentType segmentType){
    using enum SegmentType;

    switch (segmentType){
    case SOI:
        return "SOI";
    case S0F0:
        return "S0F0";
    case S0F2:
        return "S0F2";
    case DHT:
        return "DHT";
    case DQT:
        return "DQT";
    case DRI:
        return "DRI";
    case SOS:
        return "SOS";
    case RSTn:
        return "RSTn";
    case APPn:
        return "APPn";
    case COM:
        return "COM";
    case EOI:
        return "EOI";
    }

    return "Unknown...";
}