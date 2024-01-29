#ifndef FLARM_H
#define FLARM_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NMEA_MAX_LENGTH 80
#define NMEA_CS_CHARS   2
#define NMEA_CS_BUFFER  3 /* 2 for checksum + space for null terminator */

#define FLARM_ERROR_SEVERITY_NONE           0u
#define FLARM_ERROR_SEVERITY_INFORMATION    1u
#define FLARM_ERROR_SEVERITY_MINOR          2u
#define FLARM_ERROR_SEVERITY_FATAL          3u

#define FLARM_ERROR_TYPE_FIRMWARE_EXP       0x0011
#define FLARM_ERROR_TYPE_FIRMWARE_UPDATE    0x0012
#define FLARM_ERROR_TYPE_POWER              0x0021
#define FLARM_ERROR_TYPE_UI                 0x0022
#define FLARM_ERROR_TYPE_AUDIO              0x0023
#define FLARM_ERROR_TYPE_ADC                0x0024
#define FLARM_ERROR_TYPE_SD_CARD            0x0025
#define FLARM_ERROR_TYPE_USB                0x0026
#define FLARM_ERROR_TYPE_LED                0x0027
#define FLARM_ERROR_TYPE_EEPROM             0x0028
#define FLARM_ERROR_TYPE_GENERAL_HARDWARE   0x0029
#define FLARM_ERROR_TYPE_TRANSPONDER_US     0x002A
#define FLARM_ERROR_TYPE_EEPROM_AGAIN       0x002B
#define FLARM_ERROR_TYPE_GPIO               0x002C
#define FLARM_ERROR_TYPE_GPS_COMMS          0x0031
#define FLARM_ERROR_TYPE_GPS_CONFIG         0x0032
#define FLARM_ERROR_TYPE_GPS_ANTENNA        0x0033
#define FLARM_ERROR_TYPE_RF_COMMS           0x0041
#define FLARM_ERROR_TYPE_FLARM_DUPID_DET    0x0042
#define FLARM_ERROR_TYPE_ID                 0x0043
#define FLARM_ERROR_TYPE_COMMS              0x0051
#define FLARM_ERROR_TYPE_FLASH_MEMORY       0x0061
#define FLARM_ERROR_TYPE_PRESSURE_SENSOR    0x0071
#define FLARM_ERROR_TYPE_OBSTACLE_DATABASE  0x0081
#define FLARM_ERROR_TYPE_OBSTACLE_DB_EXP    0x0082
#define FLARM_ERROR_TYPE_FLIGHT_REC         0x0091
#define FLARM_ERROR_TYPE_ENG_NOISE_SENSOR   0x0093
#define FLARM_ERROR_TYPE_RANGE_ANALYZER     0x0094
#define FLARM_ERROR_TYPE_CONFIG             0x00A1
#define FLARM_ERROR_TYPE_OBSTACLE_LICENSE   0x00B1
#define FLARM_ERROR_TYPE_IGC_LICENSE        0x00B2
#define FLARM_ERROR_TYPE_AUD_LICENSE        0x00B3
#define FLARM_ERROR_TYPE_ENL_LICENSE        0x00B4
#define FLARM_ERROR_TYPE_RFB_LICENSE        0x00B5
#define FLARM_ERROR_TYPE_TIS_LICENSE        0x00B6
#define FLARM_ERROR_TYPE_GENERIC            0x0100
#define FLARM_ERROR_TYPE_FLASH_FILESYSTEM   0x0101
#define FLARM_ERROR_TYPE_FW_UPDATE_EXT_DISP 0x0110
#define FLARM_ERROR_TYPE_REGION             0x0120
#define FLARM_ERROR_TYPE_OTHER              0x00F1


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

extern void _Flarm_PFLAE(uint32_t lID, uint8_t cSeverity, uint16_t nErrorCode, uint8_t* pcMessage);
extern void _Flarm_PFLAV(uint32_t lID, float fltHwVersion, float fltSwVersion, uint8_t* pcObstVersion);

#ifndef FLARM_GPRMC_DISABLED
extern void _Flarm_GPRMC(uint32_t lID,
                         float fltTime,
                         bool bActive,
                         float fltLatitude,
                         char cLatitudeHemisphere,
                         float fltLongitude,
                         char cLongitudeHemisphere,
                         float fltSpeed,
                         float fltTrack,
                         uint32_t lDate,
                         float fltMagVar,
                         char cMagVarDirection);
#endif

extern void _Flarm_GPGGA(uint32_t lID);
extern void _Flarm_GPGSA(uint32_t lID);
extern void _Flarm_PGRMZ(uint32_t lID);
extern void _Flarm_PFLAQ(uint32_t lID);
extern void _Flarm_PFLAO(uint32_t lID);
extern void _Flarm_PFLAB(uint32_t lID);

struct sdfFlarm
{
    uint32_t lID;
    uint8_t cState;
    uint8_t buffer[NMEA_MAX_LENGTH];
    uint32_t lBufIdx;
    uint8_t cChecksumCalced;
    uint8_t cChecksumStored[NMEA_CS_BUFFER];
    uint32_t lCSIdx;
};

void Flarm_Init(struct sdfFlarm* psdcFlarm, uint32_t lID);

void Flarm_RXProcess(struct sdfFlarm* psdcFlarm, uint8_t* pcData, uint32_t lLength);

#endif
