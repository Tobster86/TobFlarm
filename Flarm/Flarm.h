#ifndef FLARM_H
#define FLARM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NMEA_MAX_LENGTH 80
#define NMEA_CS_CHARS   2

extern void _Flarm_PFLAU();
extern void _Flarm_PFLAA();
extern void _Flarm_PFLAE();
extern void _Flarm_PFLAV();
extern void _Flarm_PFLAR();
extern void _Flarm_GPRMC();
extern void _Flarm_GPGGA();
extern void _Flarm_GPGSA();
extern void _Flarm_GPTXT();
extern void _Flarm_PGRMZ();
extern void _Flarm_PFLAS();
extern void _Flarm_PFLAQ();
extern void _Flarm_PFLAO();
extern void _Flarm_PFLAI();
extern void _Flarm_PFLAC();
extern void _Flarm_PFLAJ();
extern void _Flarm_PFLAB();
extern void _Flarm_PFLAF();
extern void _Flarm_PFLAL();

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
