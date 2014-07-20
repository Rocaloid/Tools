#include <stdio.h>
#include <unistd.h>

#include <RUtil2.h>
#include <CVEDSP2.h>
#include <CVESVP.h>
#include <RUCE.h>

/*
NAME
    GenRUDB - Analyze voice waveform and generate RUCE voice bank.
    
SYNOPSIS
    genrudb [Other Options] [-n unitname] rotofile
    
OPTIONS
    
    Behaviour
    -n <unitname>
        Only generate the datafile for the specified vocal unit.
        Default: Disabled(batch process all units in roto)
    
    -a <sound intensity>
        Normalize the amplitude of utterance(in decibel).
        Default: No normalization
    
    Fundamental Frequency Estimation    
    -u <frequency>
        Upper bound for fundamental frequency estimation.
        Default: 700
    
    -l <frequency>
        Lower bound for fundamental frequency estimation.
        Default: 80
    
    -m <method>
        Fundamental frequency estimation method.
        Choices: YIN, SPECSTEP
        Default: YIN    
    
    HNM Analysis
    -s <frequency>
        The upper bound of sinusoidal component.
        Default: 10000
    
    -h <hopsize>
        Hop size for HNM analysis.
        Default: 256    
    
    -z <size>
        Size of analysis window in integer power of 2.
        Default: 2048
    
    -w <window>
        Analysis window.
        Choices: Hanning, Hamming, Blackman.
        Default: Hanning
    
    -o <offset>
        The offset(in relation to VOT) of left bound for HNM analysis.
        Default: 500
    
    Other
    -v
        Verbose.
        Default: disabled
*/

int main(int ArgN, char** Arg)
{
    
    return 0;
}

