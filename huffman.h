#pragma once

#include <memory>
#include <queue>


struct HuffmanNode {

    int value {};
    int nbOccurrences {};

    HuffmanNode* left {nullptr};
    HuffmanNode* right {nullptr};

    HuffmanNode(int val, int freq, HuffmanNode* l = nullptr, HuffmanNode* r = nullptr):
        value {val}, nbOccurrences{freq}, left{l}, right{r}
    {}


};

class HuffmanCompare {
public:
    bool operator() (const HuffmanNode& nodeA, const HuffmanNode& nodeB){
        return nodeA.value <= nodeB.value;
    }
};

HuffmanNode* generateHuffmanTree(std::vector<HuffmanNode> huffmanNodes){ 

    std::priority_queue<HuffmanNode, std::vector<HuffmanNode>, HuffmanCompare> nodesQueue {};

    for (auto node : huffmanNodes){
        nodesQueue.push(node);
    }


    while (nodesQueue.size() > 1){
        
        HuffmanNode* nodeLeft {new HuffmanNode{nodesQueue.top()}};
        nodesQueue.pop();
        HuffmanNode* nodeRight {new HuffmanNode{nodesQueue.top()}};
        nodesQueue.pop();

        HuffmanNode sumNode {-1, nodeLeft->nbOccurrences + nodeRight->nbOccurrences, nodeLeft, nodeRight};
        nodesQueue.push(sumNode);
    }

    return new HuffmanNode {nodesQueue.top()};
}