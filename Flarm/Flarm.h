#ifndef FLARM_H
#define FLARM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define NMEA_MAX_LENGTH 80

struct sdfFlarm
{
    uint32_t lID;
    bool bBuffering;
    uint8_t buffer[NMEA_MAX_LENGTH];
    uint32_t lBufIdx;
    uint8_t cChecksum;
    bool bGrabChecksum;
};

void Flarm_Init(struct sdfFlarm* psdcFlarm, uint32_t lID);

void Flarm_RXProcess(struct sdfFlarm* psdcFlarm, uint8_t* pcData, uint32_t lLength);

#endif
