# ECOND Raw data unpacking

This ECOND raw data unpacking is based on stand alone HGCal Unpacker at
https://gitlab.cern.ch/hgcal-integration/hgcal-raw-data-unpacking

To install clone the repository locally

```
git clone git@github.com:yulunmiao/ECONDUnpacker.git
```

and compile with

```
make
```

The code is expected to be compatible with c++11.
The structure of the directories is similar to the one used in CMSSW.
The test directory is reserved for test units and executables.

## Executables
To use the tool
```
./bin/testHGCalUnpacker --input [ECOND output csv file]
```
Test data can be found at ```test_capture_ASIC.csv```, with the test output ```log.txt```
