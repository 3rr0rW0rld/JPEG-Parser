#pragma once

#include "ByteFileReader.h"

#include <unordered_map>
#include <memory>

struct HuffmanTable {

    int id {};
    bool isAC {false};

    std::array<ByteArray, 16> codes {};

};

struct HuffmanNode {
    byte value {};
    int nbOccurrences {};

    std::unique_ptr<HuffmanNode> left{nullptr};
    std::unique_ptr<HuffmanNode> right{nullptr};

    HuffmanNode* parent {nullptr};

    HuffmanNode(byte value, int nbOccurrences):
        value{value}, nbOccurrences{nbOccurrences}
    {}

    void insertLeft(HuffmanNode* node);
    void insertRight(HuffmanNode* node);

    HuffmanNode* rightLevelNode();

    void print(int tabCount=0);
};

class HuffmanDecoder {
private:
    HuffmanTable huffmanTable;
    std::unique_ptr<HuffmanNode> rootNode {};

    HuffmanNode* currentLookupNode {};

public:
    HuffmanDecoder(const HuffmanTable& huffmanTable);
    const HuffmanTable& getHuffmanTable() const {return huffmanTable;}

    bool isACTable() const {return getHuffmanTable().isAC;}
    int getID() const {return getHuffmanTable().id;}

    byte getCode();
    bool findCode(bool bit);
    bool hasReachedCode() const;

    void reset();

    void print() {rootNode->print();}
};