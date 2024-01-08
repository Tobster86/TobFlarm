
#include <stdio.h>
#include "Flarm.h"

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

void _Flarm_PFLAE(uint32_t lID)
{
    printf("PFLAE\n");
}

void _Flarm_PFLAV(uint32_t lID)
{
    printf("PFLAV\n");
}

void _Flarm_PFLAR(uint32_t lID)
{
    printf("PFLAR\n");
}

void _Flarm_GPRMC(uint32_t lID)
{
    printf("GPRMC\n");
}

void _Flarm_GPGGA(uint32_t lID)
{
    printf("GPGGA\n");
}

void _Flarm_GPGSA(uint32_t lID)
{
    printf("GPGSA\n");
}

void _Flarm_GPTXT(uint32_t lID)
{
    printf("GPTXT\n");
}

void _Flarm_PGRMZ(uint32_t lID)
{
    printf("PGRMZ\n");
}

void _Flarm_PFLAS(uint32_t lID)
{
    printf("PFLAS\n");
}

void _Flarm_PFLAQ(uint32_t lID)
{
    printf("PFLAQ\n");
}

void _Flarm_PFLAO(uint32_t lID)
{
    printf("PFLAO\n");
}

void _Flarm_PFLAI(uint32_t lID)
{
    printf("PFLAI\n");
}

void _Flarm_PFLAC(uint32_t lID)
{
    printf("PFLAC\n");
}

void _Flarm_PFLAJ(uint32_t lID)
{
    printf("PFLAJ\n");
}

void _Flarm_PFLAB(uint32_t lID)
{
    printf("PFLAB\n");
}

void _Flarm_PFLAF(uint32_t lID)
{
    printf("PFLAF\n");
}

void _Flarm_PFLAL(uint32_t lID)
{
    printf("PFLAL\n");
}

