/*
Authors: 
Yulun Miao, Northwestern University
Huilin Qu, CERN

Last Modified: 

Description: 

*/

#ifndef EventFilter_HGCalRawToDigi_HGCalUnpacker_h
#define EventFilter_HGCalRawToDigi_HGCalUnpacker_h

#include <cstdint>
#include <vector>
#include <string>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <fstream>

struct HGCalUnpackerConfig {
	uint32_t sLinkBOE=0x00;                    //SLink BOE pattern
	uint32_t captureBlockReserved=0x3F;        //capture block reserved pattern
	uint32_t econdHeaderMarker=0x154;          //ECOND header Marker patter
	uint32_t idleHeaderMarker=0x555555;
	uint32_t sLinkCaptureBlockMax=10;          //maximum number of capture blocks in one S-Link, default to be 10
	uint32_t captureBlockECONDMax=12;          //maximum number of ECOND's in one capture block, default to be 12
	uint32_t econdERXMax=12;                   //maximum number of erx's in one ECOND, default to be 12 
	uint32_t erxChannelMax=37;                 //maximum number of channels in one erx, default to be 37
	uint32_t payloadLengthMax=469;             //maximum length of payload length
};

struct ChannelData{
	uint32_t index;
	std::bitset<2> TcTp;
	uint16_t ADCm;
	uint16_t ADCToT;
	uint16_t ToA;
};

struct ERXData{
	uint32_t index;
	std::bitset<3> Stat;
	std::bitset<3> Hamming;
	std::bitset<1> F;
	uint16_t commonMode0;
	uint16_t commonMode1;
	std::bitset<1> E;
	std::bitset<37> channelmap;
	std::vector<ChannelData> channelDataCollection;
};

struct ECONDData{
	uint32_t index;
	uint16_t payloadLength;
	std::bitset<1> P;
	std::bitset<1> E;
	std::bitset<2> HT;
	std::bitset<2> EBO;
	std::bitset<1> M;
	std::bitset<1> T;
	std::bitset<6> Hamming;
	uint16_t BXnumber;
	uint8_t L1Anumber;
	uint8_t Orbitnumber;
	std::bitset<1> S;
	std::bitset<2> RR;
	std::bitset<8> CRC;
	std::vector<ERXData> erxDataCollection;
	uint32_t trailerCRC;
};

class HGCalUnpacker {
public:
    enum SLinkHeaderShift{
		kSLinkBOEShift = 24,
	};
	enum SLinkHeaderMask{
		kSLinkBOEMask=0b11111111,
	};
	enum CaptureBlockHeaderShift {
		kCaptureBlockReservedShift = 26,
	};
	enum CaptureBlockMask{
		kCaptureBlockReservedMask = 0b111111,
		kCaptureBlockECONDStatusMask = 0b111,
	};
	enum ECONDHeaderShift {
		kidleHeaderShift = 8,
		kHeaderShift = 23,
		kPayloadLengthShift = 14,
		kPassThroughShift = 13,
        kExpectedShift = 12,
		kHTShift = 10,
		kEBOShift = 8,
		kMatchShift = 7,
		kTruncatedShift = 6,
        kHammingShift = 0,
        kBXnumberShift = 20,
        kL1AnumberShift = 14,
        kOrbitnumberShift = 11,
        kStatShift = 10,
        kRRShift = 8,
        kCRCShift = 0
	};
	enum ECONDHeaderMask {
		kidleHeaderMask = 0b111111111111111111111111,
		kHeaderMask = 0b111111111,
		kPayloadLengthMask = 0b111111111,
		kPassThroughMask = 0b1,
        kExpectedMask = 0b1,
		kHTMask = 0b11,
		kEBOMask = 0b11,
		kMatchMask = 0b1,
		kTruncatedMask = 0b1,
        kHammingMask = 0b111111,
        kBXnumberMask = 0b111111111111,
        kL1AnumberMask = 0b111111,
        kOrbitnumberMask = 0b111,
        kStatMask = 0b1,
        kRRMask = 0b11,
        kCRCMask = 0b11111111
	};
	enum ERXHeaderShift {
        keRxStatShift = 29,
        keRxHammingShift = 26,
		kFormatShift = 25,
		kCommonmode0Shift = 15,
		kCommonmode1Shift = 5,
        kEShift = 4
	};
	enum ERXHeaderMask {
        keRxStatMask = 0b111,
        keRxHammingMask = 0b111,
		kFormatMask = 0b1,
		kCommonmode0Mask = 0b1111111111,
		kCommonmode1Mask = 0b1111111111,
        kEMask = 0b1 
	};

	HGCalUnpacker(HGCalUnpackerConfig config);
    HGCalUnpackerConfig config_;

	void parseECOND(uint32_t* inputArray, uint32_t inputSize, uint16_t (*enabledERXMapping)(uint32_t econd));
	void printInfo(std::string path);
	void printJSON(std::string path);


private:
	const uint32_t erxBodyLeftShift_[16]={2,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0};
	const uint32_t erxBodyRightShift_[16]={0,8,0,8,0,0,0,0,0,0,0,0,0,0,0,0};
	const uint32_t erxBodyMask_[16]={
		0b00111111111111111111110000000000,
		0b00000000000011111111110000000000,
		0b00111111111111111111110000000000,
		0b00000000000011111111111111111111,
		0b00111111111111111111111111111111,
		0b00111111111111111111111111111111,
		0b00111111111111111111111111111111,
		0b00111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b11111111111111111111111111111111
	};
	const uint32_t erxBodyBits_[16]={24,16,24,24,32,32,32,32,32,32,32,32,32,32,32,32};
	std::vector<ECONDData> econdDataCollection;
};


#endif