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
    
    huffmanDecoders.push_back(HuffmanDecoder{ HuffmanTable{id, isAC, codes} });

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

HuffmanDecoder& JPEGParser::getHuffmanDecoder(bool isAC, int id){
    for (auto& decoder : huffmanDecoders){
        if (decoder.isACTable() == isAC && decoder.getID() == id){
            return decoder;
        }
    }

    return huffmanDecoders[0];
}

ByteArray JPEGParser::getBlock(int acId, int dcId, int prevDCCoeff=0){
    ByteArray block {};

    HuffmanDecoder& dcDecoder {getHuffmanDecoder(false, dcId)};
    HuffmanDecoder& acDecoder {getHuffmanDecoder(true, acId)};
    
    while (!(dcDecoder.hasReachedCode())){
        bool bit {reader.readBit()};
        dcDecoder.findCode(bit);
    }
    
    block.append(dcDecoder.getCode()+prevDCCoeff);

    while (block.size() < 64){

        while (!(acDecoder.hasReachedCode())){
            bool bit {reader.readBit()};
            acDecoder.findCode(bit);
        }

        byte code {acDecoder.getCode()};

        if (code == 0x00){
            int remainingZeroes {64 - block.size()};
            for (int k{}; k < remainingZeroes; ++k){
                block.append(0x0);
            }
        }
        else if (code == 0xF0){
            for (int k{}; k < 16; ++k){
                block.append(0x0);
            }
        }
        else{
            int run {code >> 4};
            int size {code & 0x0F};

            for (int k{}; k < run; ++k){
                block.append(0x0);
            }
            block.append(size);


        }
    }
    
    return block;

}

void JPEGParser::parseScan(){
    
    while (!reader.hasEnded()){

        for (int c{1}; c <= nbColorComponents; ++c){
            int id {colorComponentSpecs[c].id};
            for (int i{}; i < colorComponentSpecs[c].horizontalSamplingRatio * colorComponentSpecs[c].verticalSamplingRatio; ++i){
                auto block {getBlock(colorComponentSpecs[c].acEntropyCodingSelector, colorComponentSpecs[c].dcEntropyCodingSelector, 0)};
                reader.skipToEndOfByte();
                mcus[id-1].push_back(block);
            }

        }
    }

    std::println("Y : {}", mcus[0].size());
    std::println("Cb : {}", mcus[1].size());
    std::println("Cr : {}", mcus[2].size());

}

void JPEGParser::dequantize(){
    for (int c{1}; c <= nbColorComponents; ++c){
        int quantizationTableId {colorComponentSpecs[c].quantizationTableIndex};
        const QuantizationTable& qtTable {getQuantizationTable(quantizationTableId)};
        
        for (int k {}; k < mcus[c-1].size(); ++k){
            ByteArray dequantizedBlock {};
            for (int i{}; i < 64; ++i){
                dequantizedBlock.append(qtTable.values.getByte(i) * mcus[c-1][k].getByte(i));
            }
            dequantizedMcus[c-1].push_back(dequantizedBlock);
        }

    }
}