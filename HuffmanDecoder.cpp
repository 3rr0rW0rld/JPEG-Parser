#include "HuffmanDecoder.h"

#include <queue>
#include <print>

#include "utils.h"

class HuffmanCompare {
public:
    bool operator ()(HuffmanNode* first, HuffmanNode* second){
        return first->nbOccurrences > second->nbOccurrences;
    }
};

void HuffmanNode::insertLeft(HuffmanNode* node) {
    left = std::unique_ptr<HuffmanNode>(node);
    left->parent = this;
}

void HuffmanNode::insertRight(HuffmanNode* node) {
    right = std::unique_ptr<HuffmanNode>(node);
    right->parent = this;
}

HuffmanNode* HuffmanNode::rightLevelNode(){
    if (!parent){
        return nullptr;
    }

    if (parent->left.get() == this){
        return parent->right.get();
    }

    HuffmanNode* nextNode {parent->rightLevelNode()};

    if (nextNode){
        return nextNode->left.get();
    }
    
    return nullptr;

}

void HuffmanNode::print(int tabCount){

    for (int i{}; i < tabCount; ++i){
        std::print("\t");
    }
    std::println("[{} | {}]", nbOccurrences, value);

    if (left){
        left->print(tabCount+1);
    }
    else{
        for (int i{}; i < tabCount; ++i){
            std::print("\t");
        }
        std::print("()");
    }
    std::println();

    if (right){
        right->print(tabCount+1);
        std::println();
    }
    else{
        for (int i{}; i < tabCount; ++i){
            std::print("\t");
        }
        std::println("()");
    }
}

HuffmanDecoder::HuffmanDecoder(const HuffmanTable& huffmanTable):
    huffmanTable{huffmanTable}
{
    rootNode = std::unique_ptr<HuffmanNode>(new HuffmanNode{0, 0});

    rootNode->insertLeft(new HuffmanNode{0xFF,0});
    rootNode->insertRight(new HuffmanNode{0xFF,0});

    HuffmanNode* leftMost {rootNode->left.get()};
    
    for (int i{}; i < 16; ++i){
        HuffmanNode* current {leftMost};
        //std::println("{}: {}", i+1, huffmanTable.codes[i].size());
        if (huffmanTable.codes[i].size() == 0){

            while (current) {
                current->insertLeft(new HuffmanNode{0xFF,0});
                current->insertRight(new HuffmanNode{0xFF,0});
                current = current->rightLevelNode();
            }

            leftMost = leftMost->left.get();

        }
        else{
            for (int j{}; j < huffmanTable.codes[i].size(); ++j){
                leftMost->value = huffmanTable.codes[i].getByte(j);
                //std::println("{}", static_cast<int>(leftMost->value));
                //rootNode->print();
                leftMost = leftMost->rightLevelNode();
            }

            current = leftMost;

            while (current){
                current->insertLeft(new HuffmanNode{0xFF,0});
                current->insertRight(new HuffmanNode{0xFF,0});
                current = current->rightLevelNode();
            }

            leftMost = leftMost->left.get();


        }
    }

    currentLookupNode = rootNode.get();
    
    //rootNode->print();
}

byte HuffmanDecoder::getCode(){
    byte result {currentLookupNode->value};
    reset();

    return result;
}

bool HuffmanDecoder::findCode(bool bit){
    if (bit == 0){
        currentLookupNode = currentLookupNode->left.get();
    }
    else{
        currentLookupNode = currentLookupNode->right.get();
    }

    return hasReachedCode();
}

bool HuffmanDecoder::hasReachedCode() const {
    return !(currentLookupNode->left) && !(currentLookupNode->right);
}

void HuffmanDecoder::reset(){
    currentLookupNode = rootNode.get();
}