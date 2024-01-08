#ifndef FLARM_H
#define FLARM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NMEA_MAX_LENGTH 80
#define NMEA_CS_CHARS   2

extern void _Flarm_PFLAU(uint32_t lID,
                         uint8_t cContacts,
                         bool bTXOk,
                         uint8_t cGPSStatus,
                         bool bPowerOk,
                         uint8_t cAlarmLevel,
                         int16_t snRelativeBearing,
                         uint8_t cAlarmType,
                         int16_t snRelativeVertical,
                         uint32_t lRelativeDistance);

#ifndef FLARM_PLFAA_DISABLED
extern void _Flarm_PFLAA(uint32_t lID,
                         uint8_t cAlarmLevel,
                         int32_t slRelativeNorth,
                         int32_t slRelativeEast,
                         int16_t snRelativeVertical,
                         uint8_t cIDType,
                         uint16_t nTrack,
                         uint16_t nGroundSpeed,
                         float fltClimbRate,
                         uint8_t cAircraftType);
#endif

extern void _Flarm_PFLAE(uint32_t lID);
extern void _Flarm_PFLAV(uint32_t lID);
extern void _Flarm_PFLAR(uint32_t lID);
extern void _Flarm_GPRMC(uint32_t lID);
extern void _Flarm_GPGGA(uint32_t lID);
extern void _Flarm_GPGSA(uint32_t lID);
extern void _Flarm_GPTXT(uint32_t lID);
extern void _Flarm_PGRMZ(uint32_t lID);
extern void _Flarm_PFLAS(uint32_t lID);
extern void _Flarm_PFLAQ(uint32_t lID);
extern void _Flarm_PFLAO(uint32_t lID);
extern void _Flarm_PFLAI(uint32_t lID);
extern void _Flarm_PFLAC(uint32_t lID);
extern void _Flarm_PFLAJ(uint32_t lID);
extern void _Flarm_PFLAB(uint32_t lID);
extern void _Flarm_PFLAF(uint32_t lID);
extern void _Flarm_PFLAL(uint32_t lID);

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
