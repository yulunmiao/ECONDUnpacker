#include "HGCalUnpacker.h"
#include <fstream>
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
    econdDataCollection.clear();
	while (i < inputSize) {	
        //Loop through ECON-D
        //ECON-D header
        //The second word of ECON-D header contains no information for unpacking, use only the first one
        //Sanity check
        if (((inputArray[i] >> kHeaderShift) & kHeaderMask) == config_.econdHeaderMarker) {
            econdHeader=inputArray[i];
            std::cout<<std::endl<<"packet number="<<std::dec<<econd<<std::endl<<"First word of ECOND header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<econdHeader<<std::endl; //Length of ECON-D header
        }
        else if(((inputArray[i] >> kidleHeaderShift) & kidleHeaderMask) == config_.idleHeaderMarker){
            i++;
            continue;
        }
        else {
            std::cout<<"throw at "<<std::dec<<i<<std::endl;       
            throw 0;
        }

        ECONDData econdData;
        econdData.erxDataCollection.clear();
        econdData.index=econd;

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

        econdData.payloadLength=(econdHeader >> kPayloadLengthShift) & kPayloadLengthMask;
        econdData.P=std::bitset<1>((econdHeader >> kPassThroughShift) & kPassThroughMask);
        econdData.E=std::bitset<1>((econdHeader >> kExpectedShift) & kExpectedMask);
        econdData.HT=std::bitset<2>((econdHeader >> kHTShift) & kHTMask);
        econdData.EBO=std::bitset<2>((econdHeader >> kEBOShift) & kEBOMask);
        econdData.M=std::bitset<1>((econdHeader >> kMatchShift) & kMatchMask);
        econdData.T=std::bitset<1>((econdHeader >> kMatchShift) & kMatchMask);
        econdData.Hamming=std::bitset<6>((econdHeader >> kHammingShift) & kHammingMask);

        econdHeader=inputArray[i+1];
        std::cout<<"Second word of ECOND header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<econdHeader<<std::endl;
        std::cout<<std::dec
        <<"BX number = "<<((econdHeader >> kBXnumberShift) & kBXnumberMask)
        <<", L1A number = "<<((econdHeader >> kL1AnumberShift) & kL1AnumberMask)
        <<", Orbit number ="<<((econdHeader >> kL1AnumberShift) & kL1AnumberMask)
        <<", S = 0b"<<std::bitset<1>((econdHeader >> kStatShift) & kStatMask)
        <<", RR = 0b"<<std::bitset<2>((econdHeader >> kRRShift) & kRRMask)
        <<", CRC = 0b"<<std::bitset<8>((econdHeader >> kCRCShift) & kCRCMask)
        <<std::endl;

        econdData.BXnumber=((econdHeader >> kBXnumberShift) & kBXnumberMask);
        econdData.L1Anumber=((econdHeader >> kL1AnumberShift) & kL1AnumberMask);
        econdData.Orbitnumber=((econdHeader >> kL1AnumberShift) & kL1AnumberMask);
        econdData.S=std::bitset<1>((econdHeader >> kStatShift) & kStatMask);
        econdData.RR=std::bitset<2>((econdHeader >> kRRShift) & kRRMask);
        econdData.CRC=std::bitset<8>((econdHeader >> kCRCShift) & kCRCMask);

        econdHeader=inputArray[i];
        i += 2;
        econdBodyStart=i;//For ECON-D length check
        
        //ECON-D body
        if (((econdHeader >> kPassThroughShift)& kPassThroughMask) == 0) {
            //standard ECOND
            std::cout<<"Standard ECOND"<<std::endl;
            enabledERX = enabledERXMapping(econd);
            for(erx = 0; erx < config_.econdERXMax; erx++) {
                //loop through eRx 
                //pick active eRx
                if((enabledERX >> erx & 1) == 0) continue;
                ERXData erxData;
                erxData.channelDataCollection.clear();
                erxData.index=erx;
                //eRX subpacket header
                //common mode
                std::cout<<"packet:erx="<<econd<<":"<<erx<<std::endl
                <<"First word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i]<<std::endl;
                std::cout
                <<"Stat = 0b"<<std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask)
                <<", Hamming = 0b"<<std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask)
                <<", Format = 0b"<<std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask)
                <<std::dec
                <<", Extract common mode 0="<<((inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask)
                <<", Extract common mode 1="<<((inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask);
                erxData.Stat=std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask);
                erxData.Hamming=std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask);
                erxData.F=std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask);
                erxData.commonMode0=(inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask;
                erxData.commonMode1=(inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask;
                //empty check
                if (((inputArray[i]>> kFormatShift)& kFormatShift) == 1){
                    std::cout<<", E = 0b"<<std::bitset<1>((inputArray[i] >> kEShift) & kEMask)<<std::endl;
                    erxData.E=std::bitset<1>((inputArray[i] >> kEShift) & kEMask);
                    i += 1;//Length of empty eRx header
                    continue;//Go to next eRx
                }
                else{
                    std::cout<<std::endl
                    <<"Second word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i + 1]<<std::endl
                    <<"Channel Map = 0b"<<std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]))<<std::endl;
                    erxData.channelmap=std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]));
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
                    ChannelData channelData;
                    channelData.index=channel;
                    std::cout<<std::dec<<"  packet:erx:channel = "<<econd<<":"<<erx<<":"<<channel;
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
                    channelData.TcTp=std::bitset<2>(readout>>30);
                    channelData.ADCm=((readout >> 20) & 0b1111111111);
                    channelData.ADCToT=((readout >> 10) & 0b1111111111);
                    channelData.ToA=((readout >> 0) & 0b1111111111);
                    erxData.channelDataCollection.push_back(channelData);
                }
                //Pad to whole word
                i += bitCounter / 32;
                if (bitCounter % 32 != 0)
                {
                    i += 1; 
                }
                //eRx subpacket has no trailer
                econdData.erxDataCollection.push_back(erxData);
            }
        }
        else {
        //Pass through ECOND
            std::cout<<"Pass through ECOND"<<std::endl;
            enabledERX = enabledERXMapping(econd);
            for(erx = 0; erx < config_.econdERXMax; erx++) {										
                //loop through eRx 
                //pick active eRx
                if((enabledERX >> erx & 1) == 0) continue;
                ERXData erxData;
                erxData.channelDataCollection.clear();
                erxData.index=erx;
                //eRX subpacket header
                //common mode
                std::cout<<"packet:erx="<<econd<<":"<<erx<<std::endl
                <<"First word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i]<<std::endl;
                std::cout
                <<"Stat = 0b"<<std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask)
                <<", Hamming = 0b"<<std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask)
                <<", Format = 0b"<<std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask)
                <<std::dec
                <<", Extract common mode 0="<<((inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask)
                <<", Extract common mode 1="<<((inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask);
                erxData.Stat=std::bitset<3>((inputArray[i] >> keRxStatShift) & keRxStatMask);
                erxData.Hamming=std::bitset<3>((inputArray[i] >> keRxHammingShift) & keRxHammingMask);
                erxData.F=std::bitset<1>((inputArray[i] >> kFormatShift) & kFormatMask);
                erxData.commonMode0=(inputArray[i] >> kCommonmode0Shift)& kCommonmode0Mask;
                erxData.commonMode1=(inputArray[i] >> kCommonmode1Shift)& kCommonmode1Mask;
                //empty check
                if (((inputArray[i]>> kFormatShift)& kFormatShift) == 1){
                    std::cout<<", E = 0b"<<std::bitset<1>((inputArray[i] >> kEShift) & kEMask)<<std::endl;
                    erxData.E=std::bitset<1>((inputArray[i] >> kEShift) & kEMask);
                    i += 1;//Length of empty eRx header
                    continue;//Go to next eRx
                }
                else{
                    std::cout<<std::endl
                    <<"Second word of the erx header = 0x"<<std::hex<<std::setfill('0') << std::setw(8)<<inputArray[i + 1]<<std::endl
                    <<"Channel Map = 0b"<<std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]))<<std::endl;
                    erxData.channelmap=std::bitset<37>((((uint64_t)inputArray[i] & 0b11111) << 32) | ((uint64_t)inputArray[i + 1]));
                }
                for (channel = 0; channel < config_.erxChannelMax; channel++)
                {
                    //loop through channels in eRx
                    ChannelData channelData;
                    channelData.index=channel;
                    std::cout<<std::dec<<"  packet:erx:channel = "<<econd<<":"<<erx<<":"<<channel;
                    uint32_t readout = inputArray[i];
                    std::cout<<", TcTp = 0b{:b}"<<std::bitset<2>(readout>>30)
                    <<std::dec
                    <<", ADC(-1) = "<<((readout >> 20) & 0b1111111111)
                    <<", ADC/ToT = "<<((readout >> 10) & 0b1111111111)
                    <<", ToA = "<<((readout >> 0) & 0b1111111111)
                    <<std::endl;
                    i++;
                    channelData.TcTp=std::bitset<2>(readout>>30);
                    channelData.ADCm=((readout >> 20) & 0b1111111111);
                    channelData.ADCToT=((readout >> 10) & 0b1111111111);
                    channelData.ToA=((readout >> 0) & 0b1111111111);
                    erxData.channelDataCollection.push_back(channelData);
                }
                econdData.erxDataCollection.push_back(erxData);
            }
        }
        //ECON-D trailer
        //No information needed from ECON-D trailer in unpacker, skip it
        std::cout<<std::hex
        <<"ECON-D trailer CRC = 0x"<<inputArray[i]<<std::endl;
        econdData.trailerCRC=inputArray[i];
        econdDataCollection.push_back(econdData);
        i += 1;//Length of ECOND trailer
        //Check consisitency between length unpacked and payload length
        if ((i-econdBodyStart)!=payloadLength){
            std::cout<<std::dec<<i-econdBodyStart<<std::endl;
        }
        econd++;

	}
    return;
}

void HGCalUnpacker::printInfo(std::string path){
    std::ofstream fout;
    fout.open(path);
	for(unsigned int econd = 0; econd < econdDataCollection.size() ; econd++){
        ECONDData econdData = econdDataCollection.at(econd);
        fout
        <<"packet number = "<<std::dec<<econdData.index<<std::endl
        <<"payload length = "<<std::dec<<econdData.payloadLength
        <<", P = 0b"<<econdData.P
        <<", E = 0b"<<econdData.E
        <<", HT= 0b"<<econdData.HT
        <<", EBO = 0b"<<econdData.EBO
        <<", M = 0b"<<econdData.M
        <<", T = 0b"<<econdData.T
        <<", Hamming = 0b"<<econdData.Hamming
        <<std::endl
        <<"BX number = "<<std::dec<<(unsigned) econdData.BXnumber
        <<", L1A number = "<<std::dec<<(unsigned) econdData.L1Anumber
        <<", Orbit number ="<<std::dec<<(unsigned) econdData.Orbitnumber
        <<", S = 0b"<<econdData.S
        <<", RR = 0b"<<econdData.RR
        <<", CRC = 0b"<<econdData.CRC
        <<std::endl;
        for(unsigned int erx = 0; erx < econdData.erxDataCollection.size(); erx++) {
            ERXData erxData = econdData.erxDataCollection.at(erx);
            fout<<"\tpacket:erx="<<econdData.index<<":"<<erxData.index<<std::endl
            <<"\tStat = 0b"<<erxData.Stat
            <<", Hamming = 0b"<<erxData.Hamming
            <<", Format = 0b"<<erxData.F
            <<", Extract common mode 0="<<std::dec<<erxData.commonMode0
            <<", Extract common mode 1="<<std::dec<<erxData.commonMode1;
            //empty check
            if (erxData.F == 1){
                fout<<", E = 0b"<<erxData.E<<std::endl;
                continue;//Go to next eRx
            }
            else{
                fout<<std::endl
                <<"\tChannel Map = 0b"<<erxData.channelmap<<std::endl;
            }
            for (unsigned int channel = 0; channel < erxData.channelDataCollection.size(); channel++) {
                    ChannelData channelData=erxData.channelDataCollection.at(channel);
                    fout
                    <<std::dec<<"\t\tpacket:erx:channel = "<<econdData.index<<":"<<erxData.index<<":"<<channelData.index
                    <<", TcTp = 0b"<<channelData.TcTp
                    <<std::dec
                    <<", ADC(-1) = "<<channelData.ADCm
                    <<", ADC/ToT = "<<channelData.ADCToT
                    <<", ToA = "<<channelData.ToA
                    <<std::endl;
            }
        }
        fout<<std::hex<<"ECON-D trailer CRC = 0x"<<econdData.trailerCRC<<std::endl;
	}
    fout.close();
}

void HGCalUnpacker::printJSON(std::string path){
    std::ofstream fout;
    fout.open(path);
    fout<<"[";
	for(unsigned int econd = 0; econd < econdDataCollection.size() ; econd++){
        ECONDData econdData = econdDataCollection.at(econd);
        fout<<"{\n"
        <<"\t\"packet\": "<<std::dec<<econdData.index
        <<",\n\t\"payload_length\": "<<std::dec<<econdData.payloadLength
        <<",\n\t\"P\": "<<econdData.P.to_ulong()
        <<",\n\t\"E\": "<<econdData.E.to_ulong()
        <<",\n\t\"HT\": "<<econdData.HT.to_ulong()
        <<",\n\t\"EBO\": "<<econdData.EBO.to_ulong()
        <<",\n\t\"M\": "<<econdData.M.to_ulong()
        <<",\n\t\"T\": "<<econdData.T.to_ulong()
        <<",\n\t\"Hamming\": "<<econdData.Hamming.to_ulong()
        <<",\n\t\"BX_number\": "<<std::dec<<(unsigned) econdData.BXnumber
        <<",\n\t\"L1A_number\": "<<std::dec<<(unsigned) econdData.L1Anumber
        <<",\n\t\"Orbit_number\": "<<std::dec<<(unsigned) econdData.Orbitnumber
        <<",\n\t\"S\": "<<econdData.S.to_ulong()
        <<",\n\t\"RR\": "<<econdData.RR.to_ulong()
        <<",\n\t\"CRC\": "<<econdData.CRC.to_ulong()
        <<",\n\t\"eRx_collection\": [\n";
        for(unsigned int erx = 0; erx < econdData.erxDataCollection.size(); erx++) {
            ERXData erxData = econdData.erxDataCollection.at(erx);
            fout<<"\t\t{\n"
            <<"\t\t\t\"erx\": "<<std::dec<<erxData.index
            <<",\n\t\t\t\"Stat\": "<<erxData.Stat.to_ulong()
            <<",\n\t\t\t\"Hamming\": "<<erxData.Hamming.to_ulong()
            <<",\n\t\t\t\"Format\": "<<erxData.F.to_ulong()
            <<",\n\t\t\t\"common_mode0\": "<<std::dec<<erxData.commonMode0
            <<",\n\t\t\t\"common_mode1\": "<<std::dec<<erxData.commonMode1;
            //empty check
            if (erxData.F == 1){
                fout
                <<",\n\t\t\t\"E\": "<<erxData.E.to_ulong()
                <<"\n\t\t},\n";
                continue;//Go to next eRx
            }
            else{
                fout
                <<",\n\t\t\t\"channel_map\": "<<erxData.channelmap.to_ulong()
                <<",\n\t\t\t\"channel_collection\": [\n";
            }
            for (unsigned int channel = 0; channel < erxData.channelDataCollection.size(); channel++) {
                    ChannelData channelData=erxData.channelDataCollection.at(channel);
                    fout<<"\t\t\t\t{\n"
                    <<"\t\t\t\t\t\"channel\": "<<std::dec<<channelData.index
                    <<",\n\t\t\t\t\t\"TcTp\": "<<channelData.TcTp.to_ulong()
                    <<",\n\t\t\t\t\t\"ADCm\": "<<std::dec<<channelData.ADCm
                    <<",\n\t\t\t\t\t\"ADCToT\": "<<std::dec<<channelData.ADCToT
                    <<",\n\t\t\t\t\t\"ToA\": "<<std::dec<<channelData.ToA;
                    if(channel != erxData.channelDataCollection.size() - 1) fout<<"\n\t\t\t\t},\n";
                    else fout<<"\n\t\t\t\t}\n";
            }
            if(erx != econdData.erxDataCollection.size() - 1) fout<<"\t\t\t]\n\t\t},\n";
            else fout<<"\t\t\t]\n\t\t}\n";
        }
        fout<<"\t]"
        <<",\n\t\"trailer_CRC\": "<<std::dec<<econdData.trailerCRC;
        if(econd != econdDataCollection.size() - 1) fout <<"\n},\n";
        else fout<<"\n}]\n";
	}
    fout.close();
}