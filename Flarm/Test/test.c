
#include <stdio.h>
#include "Flarm.h"

int GetTokens(char* pcString, uint32_t lLength, const char cDelimiter, char* Token[], bool HasContent[], uint32_t lMaxTokens)
{
    uint32_t lCharIndex = 0;
    uint32_t lTokenCounter = 0;
    uint32_t lTokenLength = 0;
    char* cTokenStart = pcString;
    
    while(lCharIndex < lLength)
    {
        if(cDelimiter == pcString[lCharIndex])
        {
            if(lTokenLength > 0)
            {
                Token[lTokenCounter] = cTokenStart;
                HasContent[lTokenCounter] = true;
            }
            else
            {
                Token[lTokenCounter] = NULL;
                HasContent[lTokenCounter] = false;
            }
            
            lTokenCounter++;
            cTokenStart = &pcString[lCharIndex + 1];
            
            if(lMaxTokens == lTokenCounter)
                break;
                
            pcString[lCharIndex] = '\0';
            lTokenLength = 0;
        }
        else
        {
            lTokenLength++;
        }
    
        lCharIndex++;
    }
    
    return lTokenCounter;
}

int main()
{
    struct sdfFlarm sdcFlarm;
    
    Flarm_Init(&sdcFlarm, 0xAA55AA55u);
    
    FILE *file = fopen("flarmdata.log", "r");

    int ch;
    
    while ((ch = fgetc(file)) != EOF)
    {
        Flarm_RXProcess(&sdcFlarm, (uint8_t*)&ch, sizeof(char));
    }
    
    fclose(file);
    
    /*char cTestString[] = "PFLAU,6,1,2,1,0,,0,,,";
    
    int lMaxTokens = 10;
    char* Token[lMaxTokens];
    bool HasContent[lMaxTokens];
    
    GetTokens(cTestString, sizeof(cTestString), ',', Token, HasContent, lMaxTokens);
    
    for(int i = 0; i < lMaxTokens; i++)
    {
        if(HasContent[i])
            printf("%s\n", Token[i]);
        else
            printf("(No content)\n");
    }*/

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
    printf("PFLAU\n");
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
    printf("PFLAA\n");
}

void _Flarm_PFLAE(uint32_t lID, uint8_t cSeverity, uint16_t nErrorCode, uint8_t* pcMessage)
{
    printf("PFLAE\n");
}

void _Flarm_PFLAV(uint32_t lID, float fltHwVersion, float fltSwVersion, uint8_t* pcObstVersion)
{
    printf("PFLAV\n");
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
    printf("GPRMC\n");
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
    printf("GPGGA\n");
}

void _Flarm_GPGSA(uint32_t lID)
{
    printf("GPGSA\n");
}

void _Flarm_PGRMZ(uint32_t lID)
{
    printf("PGRMZ\n");
}

void _Flarm_PFLAQ(uint32_t lID)
{
    printf("PFLAQ\n");
}

void _Flarm_PFLAO(uint32_t lID)
{
    printf("PFLAO\n");
}

void _Flarm_PFLAB(uint32_t lID)
{
    printf("PFLAB\n");
}

