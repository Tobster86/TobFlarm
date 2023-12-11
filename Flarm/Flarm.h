#ifndef FLARM_H
#define FLARM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NMEA_MAX_LENGTH 80
#define NMEA_CS_CHARS   2

extern void _Flarm_Sentence(uint32_t lFlarmID, uint8_t* message, uint32_t lLength);

struct sdfFlarm
{
    uint32_t lID;
    uint8_t cState;
    uint8_t buffer[NMEA_MAX_LENGTH];
    uint32_t lBufIdx;
    uint8_t cChecksumCalced;
    uint8_t cChecksumStored[NMEA_CS_CHARS];
    uint32_t lCSIdx;
};

void Flarm_Init(struct sdfFlarm* psdcFlarm, uint32_t lID);

void Flarm_RXProcess(struct sdfFlarm* psdcFlarm, uint8_t* pcData, uint32_t lLength);

#endif
