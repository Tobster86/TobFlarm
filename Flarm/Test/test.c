
#include <stdio.h>
#include "Flarm.h"
#include "test.h"

#define TEST_FLARM_ID 0xAA55AA55u

char pcTestPFLAU[] = "$PFLAU,99,1,2,0,3,-63,4,-5793,2147483646,*XX";

uint32_t lCallsPFLAU = 0;
uint32_t lCallsPFLAA = 0;
uint32_t lCallsPFLAE = 0;
uint32_t lCallsPFLAV = 0;
uint32_t lCallsGPRMC = 0;
uint32_t lCallsGPGGA = 0;
uint32_t lCallsGPGSA = 0;
uint32_t lCallsPGRMZ = 0;
uint32_t lCallsPFLAQ = 0;

uint8_t* nmea_xorify(char* pcInput)
{
    uint32_t lLength = strlen(pcInput);
    uint8_t cXOR = 0x00;

    //Perform NMEA-spec XOR on input message.
    for(int i = 1; i < lLength - 3; i++)
    {
        cXOR ^= pcInput[i];
    }
    
    sprintf(&pcInput[lLength - 2], "%02X", cXOR);
    
    printf("\nNMEA xorified: %s\n", pcInput);
    
    return (uint8_t*)pcInput;
}

int main()
{
    struct sdfFlarm sdcFlarm;
    
    Flarm_Init(&sdcFlarm, TEST_FLARM_ID);
    
    Flarm_RXProcess(&sdcFlarm, nmea_xorify(pcTestPFLAU), sizeof(pcTestPFLAU));
    
/*    FILE *file = fopen("flarmdata.log", "r");

    int ch;
    
    while ((ch = fgetc(file)) != EOF)
    {
        Flarm_RXProcess(&sdcFlarm, (uint8_t*)&ch, sizeof(char));
    }
    
    fclose(file);*/
    
    printf("\n---=== Function counts ===---\n");
    ASSERT_EQUAL(1, lCallsPFLAU, "PFLAU count correct");
    ASSERT_EQUAL(0, lCallsPFLAA, "PFLAA count correct");
    ASSERT_EQUAL(0, lCallsPFLAE, "PFLAE count correct");
    ASSERT_EQUAL(0, lCallsPFLAV, "PFLAV count correct");
    ASSERT_EQUAL(0, lCallsGPRMC, "GPRMC count correct");
    ASSERT_EQUAL(0, lCallsGPGGA, "GPGGA count correct");
    ASSERT_EQUAL(0, lCallsGPGSA, "GPGSA count correct");
    ASSERT_EQUAL(0, lCallsPGRMZ, "PGRMZ count correct");
    ASSERT_EQUAL(0, lCallsPFLAQ, "PFLAQ count correct");

    PRINT_TEST_RESULTS;

    return 0;
}

void _Flarm_PFLAU(uint32_t lID,
                  uint8_t cContacts,
                  bool bTXOk,
                  uint8_t cGPSStatus,
                  bool bPowerOk,
                  uint8_t cAlarmLevel,
                  int16_t snRelativeBearing,
                  uint8_t cAlarmType,
                  int16_t snRelativeVertical,
                  uint32_t lRelativeDistance)
{
    printf("\nPFLAU\n");
    lCallsPFLAU++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
    ASSERT_EQUAL(99, cContacts, "Contacts OK");
    ASSERT_EQUAL(true, bTXOk, "TXOK OK");
    ASSERT_EQUAL(2, cGPSStatus, "GPSStatus OK");
    ASSERT_EQUAL(false, bPowerOk, "PowerOK OK");
    ASSERT_EQUAL(3, cAlarmLevel, "AlarmLevel OK");
    ASSERT_EQUAL(-63, snRelativeBearing, "RelativeBearing OK");
    ASSERT_EQUAL(4, cAlarmType, "AlarmType OK");
    ASSERT_EQUAL(-5793, snRelativeVertical, "RelativeVertical OK");
    ASSERT_EQUAL(2147483646, lRelativeDistance, "RelativeDistance OK");
}

void _Flarm_PFLAA(uint32_t lID,
                  uint8_t cAlarmLevel,
                  int32_t slRelativeNorth,
                  int32_t slRelativeEast,
                  int16_t snRelativeVertical,
                  uint8_t cIDType,
                  uint16_t nTrack,
                  uint16_t nGroundSpeed,
                  float fltClimbRate,
                  uint8_t cAircraftType)
{
    printf("\nPFLAA\n");
    lCallsPFLAA++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_PFLAE(uint32_t lID, uint8_t cSeverity, uint16_t nErrorCode, uint8_t* pcMessage)
{
    printf("\nPFLAE\n");
    lCallsPFLAE++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_PFLAV(uint32_t lID, float fltHwVersion, float fltSwVersion, uint8_t* pcObstVersion)
{
    printf("\nPFLAV\n");
    lCallsPFLAV++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_GPRMC(uint32_t lID,
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
                  char cMagVarDirection)
{
    printf("\nGPRMC\n");
    lCallsGPRMC++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_GPGGA(uint32_t lID,
                  float fltTime,
                  float fltLatitude,
                  char cLatitudeHemisphere,
                  float fltLongitude,
                  char cLongitudeHemisphere,
                  uint8_t cQuality,
                  uint8_t cSatellites,
                  float fltHDOP,
                  float fltAltitude,
                  char cAltitudeUnit,
                  float fltUndulation,
                  char cUndulationUnit)
{
    printf("\nGPGGA\n");
    lCallsGPGGA++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_GPGSA(uint32_t lID,
                  char cModeAuto,
                  char cMode,
                  float fltPDOP,
                  float fltHDOP,
                  float fltVDOP)
{
    printf("\nGPGSA\n");
    lCallsGPGSA++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_PGRMZ(uint32_t lID)
{
    printf("\nPGRMZ\n");
    lCallsPGRMZ++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

void _Flarm_PFLAQ(uint32_t lID, char* pcOp, char* pcInfo, uint8_t cProgress)
{
    printf("\nPFLAQ\n");
    lCallsPFLAQ++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID);
}

