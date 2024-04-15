
#include <stdio.h>
#include "Flarm.h"
#include "test.h"

#define TEST_FLARM_ID 0xAA55AA55u

char pcTestPFLAU[] = "$PFLAU,99,1,2,0,3,-63,4,-5793,2147483646,*XX";
char pcTestPFLAA[] = "$PFLAA,3,-19999999,19999999,-32767,1,ABCD6969,359,xxxxx,32500,-32.7,C,*XX";
char pcTestPFLAE[] = "$PFLAE,A,3,120,LOLOLOL69,*XX";
char pcTestPFLAV[] = "$PFLAV,A,1.23,4.56,*XX";
char pcTestGPRMC[] = "$GPRMC,170102.69,A,1.069,E,57.4659,N,69.5,269.42,080424,,,,,*XX";

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
    Flarm_RXProcess(&sdcFlarm, nmea_xorify(pcTestPFLAA), sizeof(pcTestPFLAA));
    Flarm_RXProcess(&sdcFlarm, nmea_xorify(pcTestPFLAE), sizeof(pcTestPFLAE));
    Flarm_RXProcess(&sdcFlarm, nmea_xorify(pcTestPFLAV), sizeof(pcTestPFLAV));
    Flarm_RXProcess(&sdcFlarm, nmea_xorify(pcTestGPRMC), sizeof(pcTestGPRMC));
    
/*    FILE *file = fopen("flarmdata.log", "r");

    int ch;
    
    while ((ch = fgetc(file)) != EOF)
    {
        Flarm_RXProcess(&sdcFlarm, (uint8_t*)&ch, sizeof(char));
    }
    
    fclose(file);*/
    
    printf("\n---=== Function counts ===---\n");
    ASSERT_EQUAL(1, lCallsPFLAU, "PFLAU count correct");
    ASSERT_EQUAL(1, lCallsPFLAA, "PFLAA count correct");
    ASSERT_EQUAL(1, lCallsPFLAE, "PFLAE count correct");
    ASSERT_EQUAL(1, lCallsPFLAV, "PFLAV count correct");
    ASSERT_EQUAL(1, lCallsGPRMC, "GPRMC count correct");
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
                  char* pcID,
                  uint16_t nTrack,
                  uint16_t nGroundSpeed,
                  float fltClimbRate,
                  uint8_t cAircraftType)
{
    printf("\nPFLAA\n");
    lCallsPFLAA++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
    ASSERT_EQUAL(3, cAlarmLevel, "Alarm Level OK");
    ASSERT_EQUAL(-19999999, slRelativeNorth, "Relative North OK");
    ASSERT_EQUAL(19999999, slRelativeEast, "Relative East OK");
    ASSERT_EQUAL(-32767, snRelativeVertical, "Relative Vertical OK");
    ASSERT_EQUAL(1, cIDType, "ID Type OK");
    ASSERT_EQUAL(0, strcmp(pcID, "ABCD6969"), "FLARM ID OK");
    ASSERT_EQUAL(359, nTrack, "Track OK");
    ASSERT_EQUAL(32500, nGroundSpeed, "Ground Speed OK");
    ASSERT_EQUAL(FLOAT_EQUAL(-32.7f, fltClimbRate), true, "Climb Rate OK");
    ASSERT_EQUAL(0x0C, cAircraftType, "Aircraft Type OK");
}

void _Flarm_PFLAE(uint32_t lID, uint8_t cSeverity, uint16_t nErrorCode, char* pcMessage)
{
    printf("\nPFLAE\n");
    lCallsPFLAE++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
    ASSERT_EQUAL(3, cSeverity, "Severity OK");
    ASSERT_EQUAL(0x120, nErrorCode, "Error Code OK");
    ASSERT_EQUAL(0, strcmp(pcMessage, "LOLOLOL69"), "Error Message OK");
}

void _Flarm_PFLAV(uint32_t lID, float fltHwVersion, float fltSwVersion, uint8_t* pcObstVersion)
{
    printf("\nPFLAV\n");
    lCallsPFLAV++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
    ASSERT_EQUAL(FLOAT_EQUAL(1.23, fltHwVersion), true, "HW Version OK");
    ASSERT_EQUAL(FLOAT_EQUAL(4.56, fltSwVersion), true, "SW Version OK");
    ASSERT_EQUAL(NULL,pcObstVersion, "Obst Version OK");
}

void _Flarm_GPRMC(uint32_t lID,
                  float fltTime,
                  bool bValid,
                  float fltLatitude,
                  char cLatitudeHemisphere,
                  float fltLongitude,
                  char cLongitudeHemisphere,
                  float fltSpeed,
                  float fltTrack)
{
    printf("\nGPRMC\n");
    lCallsGPRMC++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
    ASSERT_EQUAL(FLOAT_EQUAL(170102.69, fltTime), true, "Time OK");
    ASSERT_EQUAL(bValid, true, "Valid OK");
    ASSERT_EQUAL(FLOAT_EQUAL(1.069, fltLatitude), true, "Latitude OK");
    ASSERT_EQUAL(cLatitudeHemisphere, 'E', "Latitude Hemisphere OK");
    ASSERT_EQUAL(FLOAT_EQUAL(57.4659, fltLongitude), true, "Longitude OK");
    ASSERT_EQUAL(cLongitudeHemisphere, 'N', "Longitude Hemisphere OK");
    ASSERT_EQUAL(FLOAT_EQUAL(69.5, fltSpeed), true, "Speed OK");
    ASSERT_EQUAL(FLOAT_EQUAL(269.42, fltTrack), true, "Track OK");
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
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
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
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
}

void _Flarm_PGRMZ(uint32_t lID)
{
    printf("\nPGRMZ\n");
    lCallsPGRMZ++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
}

void _Flarm_PFLAQ(uint32_t lID, char* pcOp, char* pcInfo, uint8_t cProgress)
{
    printf("\nPFLAQ\n");
    lCallsPFLAQ++;
    ASSERT_EQUAL(TEST_FLARM_ID, lID, "ID OK");
}

