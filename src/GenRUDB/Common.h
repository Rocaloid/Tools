#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Wave CDSP2_Wave_Float
#define F0Iterlyzer CSVP_F0Iterlyzer_Float
#define PSOLAIterlyzer CSVP_PSOLAIterlyzer_Float
#define Real Float

char* CRotoFile;
char* CUnitName;
int   ReadOnlyFlag;
float UFundFreq;
float LFundFreq;
char* CFundMethod;
float USinuFreq;
int   HopSize;
int   WinSize;
char* CWindow;
int   VOT;
int   VOTFlag;
int   Offset;
int   VerboseFlag;

String FundMethod, UnitName, WindowName;

enum
{
    Hanning = 0,
    Hamming,
    Blackman
} EWindow;

enum
{
    YIN = 0,
    SpecStep = 1
} EF0;

void DirFromFilePath(String* Dest, String* Sorc);
void BaseFromFilePath(String* Dest, String* Sorc);

#endif

