#ifndef GENRUDB_COMMON_H
#define GENRUDB_COMMON_H

#define Wave CDSP2_Wave_Float
#define Real Float

char* CRotoFile;
char* CUnitName;
int   ReadOnlyFlag;
float Intensity;
int   NormFlag;
float UFundFreq;
float LFundFreq;
char* CFundMethod;
float USinuFreq;
int   HopSize;
int   WinSize;
char* CWindow;
int   Offset;
int   VerboseFlag;

String FundMethod, UnitName, WindowName;

enum
{
    Hanning = 0,
    Hamming,
    Blackman
} EWindow;

void DirFromFilePath(String* Dest, String* Sorc);
void BaseFromFilePath(String* Dest, String* Sorc);

#endif

