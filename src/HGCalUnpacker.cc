#include "HGCalUnpacker.h"

HGCalUnpacker::HGCalUnpacker(HGCalUnpackerConfig config):config_(config){}

void HGCalUnpacker::parseECOND(uint32_t* inputArray, uint32_t inputSize, uint16_t (*enabledERXMapping)(uint32_t econd)){
    uint32_t temp;
	uint32_t payloadLength;

	uint32_t econd = 0;
	uint32_t erx = 0;
	uint32_t channel = 0;
    uint16_t enabledERX;

	uint32_t econdHeader;
	uint64_t erxHeader;
	uint32_t econdBodyStart;
	
    uint32_t bitCounter;
	uint32_t tempIndex;
	uint8_t tempBit;
	uint8_t code;

    uint32_t i = 0;
	while (i < inputSize) {	
        //Loop through ECON-D
        //ECON-D header
        //The second word of ECON-D header contains no information for unpacking, use only the first one
        //Sanity check
        if (((inputArray[i] >> kHeaderShift) & kHeaderMask) == config_.econdHeaderMarker) {
            econdHeader=inputArray[i];
            std::cout<<std::endl<<"ECOND="<<std::dec<<econd<<std::endl<<"First word of ECOND header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<econdHeader<<std::endl; //Length of ECON-D header
        }
        else if(((inputArray[i] >> kidleHeaderShift) & kidleHeaderMask) == config_.idleHeaderMarker){
            i++;
            continue;
        }
        else {
            std::cout<<std::hex<<((inputArray[i] >> kidleHeaderShift) & kidleHeaderMask)<<std::endl;         
            std::cout<<"throw at "<<std::dec<<i<<std::endl;       
            throw 0;
        }
        payloadLength=(econdHeader >> kPayloadLengthShift) & kPayloadLengthMask;
        std::cout<<std::dec
        <<"payload length = "<<((econdHeader >> kPayloadLengthShift) & kPayloadLengthMask)
        <<", P = 0b"<<std::bitset<1>((econdHeader >> kPassThroughShift) & kPassThroughMask)
        <<", E = 0b"<<std::bitset<1>((econdHeader >> kExpectedShift) & kExpectedMask)
        <<", HT= 0b"<<std::bitset<2>((econdHeader >> kHTShift) & kHTMask)
        <<", EBO = 0b"<<std::bitset<2>((econdHeader >> kEBOShift) & kEBOMask)
        <<", M = 0b"<<std::bitset<1>((econdHeader >> kMatchShift) & kMatchMask)
        <<", T = 0b"<<std::bitset<1>((econdHeader >> kTruncatedShift) & kTruncatedMask)
        <<", Hamming = 0b"<<std::bitset<6>((econdHeader >> kHammingShift) & kHammingMask)
        <<std::endl;

        econdHeader=inputArray[i+1];
        std::cout<<"Second word of ECOND header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<econdHeader<<std::endl;
        std::cout<<std::dec
        <<"BX number = "<<((econdHeader >> kBXnumberShift) & kBXnumberMask)
        <<", L1A number = "<<((econdHeader >> kL1AnumberShift) & kL1AnumberMask)
        <<", Orbit number ="<<((econdHeader >> kOrbitnumberShift) & kOrbitnumberMask)
        <<", S = 0b"<<std::bitset<1>((econdHeader >> kStatShift) & kStatMask)
        <<", RR = 0b"<<std::bitset<2>((econdHeader >> kRRShift) & kRRMask)
        <<", CRC = 0b"<<std::bitset<8>((econdHeader >> kCRCShift) & kCRCMask)
        <<std::endl;

        econdHeader=inputArray[i];
        i += 2;
        econdBodyStart=i;//For ECON-D length check
        
        //ECON-D body
        if (((econdHeader >> kPassThroughShift)& kPassThroughMask) == 0) {
            //standard ECOND
            std::cout<<"Standard ECOND";
            enabledERX = enabledERXMapping(econd);
            for(erx = 0; erx < config_.econdERXMax; erx++) {										
                //loop through eRx 
                //pick active eRx
                if((enabledERX >> erx & 1) == 0) continue;
                //eRX subpacket header
                //common mode
                std::cout<<"ECOND:erx="<<econd<<":"<<erx<<std::endl
                <<"First word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i]<<std::endl;
                std::cout
                <<"Stat = 0b"<<std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask)
                <<", Hamming = 0b"<<std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask)
                <<", Format = 0b"<<std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask)
                <<std::dec
                <<", Extract common mode 0="<<((inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask)
                <<", Extract common mode 1="<<((inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask);
                //empty check
                if (((inputArray[i]>> kFormatShift)& kFormatShift) == 1){
                    std::cout<<", E = 0b"<<std::bitset<1>((inputArray[i] >> kEShift) & kEMask)<<std::endl;
                    i += 1;//Length of empty eRx header
                    continue;//Go to next eRx
                }
                else{
                    std::cout<<std::endl
                    <<"Second word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i + 1]<<std::endl
                    <<"Channel Map = 0b"<<std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]))<<std::endl;
                }
                //regular
                erxHeader = ((uint64_t)inputArray[i] << 32) | ((uint64_t)inputArray[i + 1]);
                i += 2;//Length of standard eRx header
                bitCounter = 0;
                //eRx subpacket body
                for (channel = 0; channel < config_.erxChannelMax; channel++) {
                    //Loop through channels in eRx
                    //Pick active channels
                    if (((erxHeader >> channel) & 1)==0) continue;
                    std::cout<<std::dec<<"  ECOND:erx:channel = "<<econd<<":"<<erx<<":"<<channel;
                    tempIndex = bitCounter / 32 + i;
                    tempBit = bitCounter % 32;
                    if (tempBit == 0)
                    {
                        temp = inputArray[tempIndex];
                    }
                    else
                    {
                        temp = (inputArray[tempIndex] << tempBit) | (inputArray[tempIndex + 1] >> (32 - tempBit));
                    }
                    code = temp >> 28;
                    //use if and else here
                    uint32_t readout=((temp << erxBodyLeftShift_[code])>>erxBodyRightShift_[code]) & erxBodyMask_[code];
                    bitCounter += erxBodyBits_[code];
                    if (code == 0b0010)
                    {
                        readout |= 0b1 << 30;
                    }
                    std::cout<<", TcTp = 0b"<<std::bitset<2>(readout>>30)
                    <<std::dec
                    <<", ADC(-1) = "<<((readout >> 20) & 0b1111111111)
                    <<", ADC/ToT = "<<((readout >> 10) & 0b1111111111)
                    <<", ToA = "<<((readout >> 0) & 0b1111111111)
                    <<std::endl;
                }
                //Pad to whole word
                i += bitCounter / 32;
                if (bitCounter % 32 != 0)
                {
                    i += 1; 
                }
                //eRx subpacket has no trailer
            }
        }
        else {
        //Pass through ECOND
            std::cout<<"Pass through ECOND";
            enabledERX = enabledERXMapping(econd);
            for(erx = 0; erx < config_.econdERXMax; erx++) {										
                //loop through eRx 
                //pick active eRx
                if((enabledERX >> erx & 1) == 0) continue;
                //eRX subpacket header
                //common mode
                
                std::cout<<"ECOND:erx="<<econd<<":"<<erx<<std::endl
                <<"First word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i]<<std::endl;
                std::cout
                <<"Stat = 0b"<<std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask)
                <<", Hamming = 0b"<<std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask)
                <<", Format = 0b"<<std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask)
                <<std::dec
                <<", Extract common mode 0="<<((inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask)
                <<", Extract common mode 1="<<((inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask);
                //empty check
                if (((inputArray[i]>> kFormatShift)& kFormatShift) == 1){
                    std::cout<<", E = 0b"<<std::bitset<1>((inputArray[i] >> kEShift) & kEMask)<<std::endl;
                    i += 1;//Length of empty eRx header
                    continue;//Go to next eRx
                }
                else{
                    std::cout<<std::endl
                    <<"Second word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i + 1]<<std::endl
                    <<"Channel Map = 0b"<<std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]))<<std::endl;
                }


                for (channel = 0; channel < config_.erxChannelMax; channel++)
                {
                    //loop through channels in eRx
                    std::cout<<std::dec<<"  ECOND:erx:channel = "<<econd<<":"<<erx<<":"<<channel;
                    uint32_t readout = inputArray[i];
                    std::cout<<", TcTp = 0b{:b}"<<std::bitset<2>(readout>>30)
                    <<std::dec
                    <<", ADC(-1) = "<<((readout >> 20) & 0b1111111111)
                    <<", ADC/ToT = "<<((readout >> 10) & 0b1111111111)
                    <<", ToA = "<<((readout >> 0) & 0b1111111111)
                    <<std::endl;
                    i++;
                }
            }
        }
        //ECON-D trailer
        //No information needed from ECON-D trailer in unpacker, skip it
        std::cout<<std::hex
        <<"ECON-D trailer CRC = 0x"<<inputArray[i]<<std::endl;
        i += 1;//Length of ECOND trailer
        //Check consisitency between length unpacked and payload length
        if ((i-econdBodyStart)!=payloadLength){
            std::cout<<std::dec<<i-econdBodyStart<<std::endl;
        }
        econd++;

	}
    return;
}