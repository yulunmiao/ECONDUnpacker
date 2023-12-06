#include "HGCalUnpacker.h"
#include <fstream>
#include <sstream>

uint16_t enabledERXMapping (uint32_t econd){
    return 0b111111111111;
}


int main(int argc, char** argv){
    std::string ifname;
    std::string ofname;
    std::string jsonfname;
    bool out=false,jsonout=false;
    for(int i=1;i<argc;i++){
		std::string arg(argv[i]);
		if(arg.find("--input")!=std::string::npos)  { ifname=argv[i+1]; i++; }
        if(arg.find("--output")!=std::string::npos) { ofname=argv[i+1]; out=true; i++; }
        if(arg.find("--json")!=std::string::npos)   { jsonfname=argv[i+1]; jsonout=true; i++;}
	}
    std::fstream fin; 
    fin.open(ifname, std::ios::in); 
    char comma;
    uint32_t temp;
    std::string line;
    std::vector<unsigned int> inputArray;

    while (getline(fin,line)){
        std::stringstream s(line);
        while(s>>std::hex>>temp){
            inputArray.push_back(temp);
            s>>comma;
        }
    }
    for(unsigned int i=0;i<inputArray.size();i+=6){
        temp=inputArray[i];
        inputArray[i]=inputArray[i+5];
        inputArray[i+5]=temp;
        temp=inputArray[i+1];
        inputArray[i+1]=inputArray[i+4];
        inputArray[i+4]=temp;
        temp=inputArray[i+2];
        inputArray[i+2]=inputArray[i+3];
        inputArray[i+3]=temp;
    }
    fin.close();
    /*
    for(unsigned int i=0;i<inputArray.size();i+=6){
        std::cout<<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i]<<","
        <<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i+1]<<","
        <<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i+2]<<","
        <<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i+3]<<","
        <<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i+4]<<","
        <<std::hex<<std::setfill('0') << std::setw(8)
        <<inputArray[i+5]<<std::endl;
    }
    */
    /*
    uint32_t testInput[216]={
        0xf3359008,0x0108549e,0xe087a09b,0x67c76e8d,0x15f40889,0x7e124020
        ,0xd19413e8,0x137816a0,0x04094e14,0x0815bc3a,0x745f043c,0xb602ddc8
        ,0x0361c402,0xe90020bc,0x1a06fcbd,0x13e011c8,0x146c518a,0x98931468
        ,0x535abc96,0xe01204fd,0xeb5fb777,0x11881474,0x20dc3509,0x79c311f8
        ,0x13c8219d,0x2913f015,0x000228d5,0x02bcc613,0x7413b803,0xf080c351
        ,0xf05d1500,0x11a038b4,0x82054912,0x0381330c,0x7d05036d,0xe304ed3d
        ,0x4aefc082,0x2324b602,0x552715f8,0x13f00000,0xe038913b,0x0fa6c1b9
        ,0x0459ca22,0x80280258,0xb70e30e7,0x04858617,0x9024216f,0x2cf56212
        ,0x98134414,0x280228a0,0x175413a4,0x0808ec02,0x18a213f0,0x035db364
        ,0x7b00c304,0x01240000,0xe0360cac,0x07da37cf,0x3b447e12,0xa0138020
        ,0x904b120c,0x3a807f02,0xc5a037f8,0x9903c87e,0x125820c6,0xaa3a6499
        ,0x126c13dc,0x14b0625d,0xdc78169c,0x04d0c015,0x5412bc38,0x487c0000
        ,0xe01e072f,0xfb767f9a,0x12943b70,0x9013a414,0x1014a806,0x5cde2014
        ,0x4e0ac930,0x123812f4,0x12501348,0x37f4b204,0xa0e920fc,0x1a164802
        ,0x5d32c460,0xd0743d78,0xa303a9ad,0x12201700,0x13d812c8,0x131411d4
        ,0x13c80000,0xe00d03ef,0xd6f03d91,0x15cc135c,0x02e4b721,0xfc0514d4
        ,0x04388712,0xf008a922,0x11cc3b34,0x88131c02,0xb4f1cc26,0x7caa1784
        ,0x12180564,0x88039515,0x12b012ec,0x0680b20c,0xb10d0000,0xe00801f7
        ,0x3f8fd9d2,0x168c53ac,0x3cb30491,0x2c03990e,0x02e19603,0x54ba053d
        ,0xa90634c0,0x3c608413,0x24154011,0xb437e094,0x14705b4d,0x2ca839a0
        ,0x750338a6,0x1184047c,0xdb757994,0x9d13c806,0xc963544b,0xdc836cb8
        ,0x14720000,0xe02389ed,0xe57f7574,0x04512404,0xa9e80320,0xbd120c02
        ,0xc9960331,0x6f178817,0x18121009,0xe4ec14f8,0x16ec12cc,0x03acac04
        ,0x74861454,0x16ac0301,0x760388d1,0x02715102,0x2cbe3864,0xa313a408
        ,0xb9190000,0xe025868f,0xb5ce9e07,0x0224ac23,0xf906067c,0xbe4b1840
        ,0x9e3f54a0,0x13201278,0x14a02441,0x9815b003,0x094513c4,0x21800b65
        ,0x1854a254,0x07dc8d13,0xf416e006,0x21400394,0xc4201eff,0x15680819
        ,0x6c000000,0xe02f0a9e,0x3ba3d6e6,0x0490a103,0x20991284,0x11d41370
        ,0x16fc11d4,0x12c014ac,0x387cbb02,0x71781530,0x20d4043b,0x749c0f4d
        ,0x240270f6,0x38d48e13,0x8c04e1ab,0x4af89c96,0x023d2913,0xd473a820
        ,0x9a000000,0xe0288c1a,0xfc9b59ce,0x17581350,0x03c8b006,0xfca90210
        ,0xd3142c14,0x6014a83d,0x008b030c,0x7903dc88,0x131c14b0,0x05297412
        ,0xac0320e2,0x54d7fc97,0x13743800,0x8b08095d,0x020ce002,0x44893f80
        ,0xa1000000,0xe01c05fd,0xf21be433,0x13b0128c,0x289c542b,0xf4470b25
        ,0x5d11d405,0xc57b125c,0x0510ec05,0xe1f315c0,0x1340037c,0xdd097119
        ,0x14240250,0xf6032943,0x02e52d23,0x919e02ac,0x8421688b,0x8eba509a
    };
    */
    HGCalUnpackerConfig config;
    config.econdHeaderMarker=0x1e6;
    HGCalUnpacker unpacker(config);
    unpacker.parseECOND(&inputArray[0],inputArray.size(),enabledERXMapping);
    //unpacker.parseECOND(testInput,216,enabledERXMapping);
    if(out){
        unpacker.printInfo(ofname);
    }
    if(jsonout){
        unpacker.printJSON(jsonfname);
    }
    return 0;
}
