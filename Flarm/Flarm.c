
#include "Flarm.h"

#define NMEA_START      '$'
#define NMEA_CS_DELIMIT '*'
#define NMEA_END_CR     '\r'
#define NMEA_END_LF     '\n'

#define FLARM_STATE_IDLE 0x01
#define FLARM_STATE_BUFF 0x02
#define FLARM_STATE_CHKS 0x03

static void Flarm_Interpret(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAU(uint32_t lID, uint8_t* pcData, uint32_t lLength);

#ifndef FLARM_PLFAA_DISABLED
static void Flarm_PFLAA(uint32_t lID, uint8_t* pcData, uint32_t lLength);
#endif

static void Flarm_PFLAE(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAV(uint32_t lID, uint8_t* pcData, uint32_t lLength);

#ifndef FLARM_GPRMC_DISABLED
static void Flarm_GPRMC(uint32_t lID, uint8_t* pcData, uint32_t lLength);
#endif

#ifndef FLARM_GPGGA_DISABLED
static void Flarm_GPGGA(uint32_t lID, uint8_t* pcData, uint32_t lLength);
#endif

static void Flarm_GPGSA(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PGRMZ(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAQ(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAO(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAB(uint32_t lID, uint8_t* pcData, uint32_t lLength);

static long Flarm_GetInt(uint8_t* pcData)
{
    if(pcData)
        return strtol((char*)pcData, NULL, 10);
        
    return 0;
}

static long Flarm_GetIntFromHex(uint8_t* pcData)
{
    if(pcData)
        return strtoul((char*)pcData, NULL, 16);
        
    return 0;
}

static float Flarm_GetDecimal(uint8_t* pcData)
{
    if(pcData)
        return strtof((char*)pcData, NULL);
        
    return 0.0f;
}

static char Flarm_GetChar(uint8_t* pcData)
{
    if(pcData)
        return *pcData;
        
    return '\0';
}

static uint32_t Flarm_GetTokens(uint8_t* pcString,
                                uint32_t lLength,
                                uint8_t* Token[],
                                bool HasContent[],
                                uint32_t lMaxTokens)
{
    static const char cDelimiter = ',';
    uint32_t lCharIndex = 0;
    uint32_t lTokenCounter = 0;
    uint32_t lTokenLength = 0;
    uint8_t* pcTokenStart = pcString;
    
    while(lCharIndex < lLength)
    {
        if(cDelimiter == pcString[lCharIndex])
        {
            if(lTokenLength > 0)
            {
                Token[lTokenCounter] = pcTokenStart;
                HasContent[lTokenCounter] = true;
            }
            else
            {
                Token[lTokenCounter] = NULL;
                HasContent[lTokenCounter] = false;
            }
            
            pcString[lCharIndex] = '\0';
            
            //printf("Token: %s\n", pcTokenStart);
            
            lTokenCounter++;
            pcTokenStart = &pcString[lCharIndex + 1];
            
            if(lMaxTokens == lTokenCounter)
            {
                break;
            }

            lTokenLength = 0;
        }
        else
        {
            lTokenLength++;
        }
    
        lCharIndex++;
    }
    
    if(lTokenCounter < lMaxTokens)
    {
        //Assign any remaining token slots as empty.
        for(int i = lTokenCounter; i < lMaxTokens; i++)
        {
            //printf("Unoccupied: %d\n", i);
            Token[i] = NULL;
            HasContent[i] = false;
        }
    }
    
    return lTokenCounter;
}

void Flarm_Init(struct sdfFlarm* psdcFlarm, uint32_t lID)
{
    memset(psdcFlarm, 0x00, sizeof(struct sdfFlarm));
    psdcFlarm->lID = lID;
}

void Flarm_RXProcess(struct sdfFlarm* psdcFlarm, uint8_t* pcData, uint32_t lLength)
{
    for(int i = 0; i < lLength; i++)
    {
        switch(pcData[i])
        {
            case NMEA_START:
            {
                /* Start buffering a new message. */
                psdcFlarm->cState = FLARM_STATE_BUFF;
                psdcFlarm->lBufIdx = 0;
                psdcFlarm->cChecksumCalced = 0;
            }
            break;
            
            case NMEA_CS_DELIMIT:
            {
                if(FLARM_STATE_BUFF == psdcFlarm->cState)
                {
                    /* End of message. Grab the checksum next. */
                    psdcFlarm->cState = FLARM_STATE_CHKS;
                    psdcFlarm->lCSIdx = 0;
                }
            }
            break;

            default:
            {
                switch(psdcFlarm->cState)
                {
                    case FLARM_STATE_BUFF:
                    {
                        if(psdcFlarm->lBufIdx >= NMEA_MAX_LENGTH)
                        {
                            /* Length limit exceeded. Bin this message. */
                            psdcFlarm->cState = FLARM_STATE_IDLE;
                        }
                        else
                        {
                            /* Continue buffering the message. */
                            psdcFlarm->buffer[psdcFlarm->lBufIdx++] = pcData[i];
                            psdcFlarm->cChecksumCalced ^= pcData[i];
                        }
                    }
                    break;
                    
                    case FLARM_STATE_CHKS:
                    {
                        psdcFlarm->cChecksumStored[psdcFlarm->lCSIdx++] = pcData[i];
                        
                        if(NMEA_CS_CHARS <= psdcFlarm->lCSIdx)
                        {
                            psdcFlarm->cChecksumStored[NMEA_CS_CHARS] = '\0';
                            uint8_t cChecksum = (uint8_t)Flarm_GetIntFromHex(psdcFlarm->cChecksumStored);
                            
                            if(cChecksum == psdcFlarm->cChecksumCalced)
                            {
                                Flarm_Interpret(psdcFlarm->lID, psdcFlarm->buffer, psdcFlarm->lBufIdx);
                            }
                            
                            psdcFlarm->cState = FLARM_STATE_IDLE;
                        }
                    }
                    break;
                    
                    default:
                    {
                        /* Do nothing. */
                    }
                }
            }
        }
    }
}

static void Flarm_Interpret(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    if(lLength > 0)
    {
        switch(pcData[0])
        {
            case 'P': //P
            {
                switch(pcData[1])
                {
                    case 'F': //PF
                    {
                        if('L' == pcData[2]) //PFL
                        {
                            if('A' == pcData[3]) //PFLA
                            {
                                switch(pcData[4])
                                {
                                    case 'U': Flarm_PFLAU(lID, pcData, lLength); break;
                                    case 'A': Flarm_PFLAA(lID, pcData, lLength); break;
                                    case 'E': Flarm_PFLAE(lID, pcData, lLength); break;
                                    case 'V': Flarm_PFLAV(lID, pcData, lLength); break;
                                    case 'Q': Flarm_PFLAQ(lID, pcData, lLength); break;
                                    case 'O': Flarm_PFLAO(lID, pcData, lLength); break;
                                    case 'B': Flarm_PFLAB(lID, pcData, lLength); break;
                                }
                            }
                        }
                    }
                    break;
                    
                    case 'G': //PG
                    {
                        if(0 == strncmp("RMZ", (char*)&pcData[2], 3))
                        {
                            Flarm_PGRMZ(lID, pcData, lLength);
                        }
                    }
                    break;
                }
            }
            break;
            
            case 'G': //G
            {
                if('P' == pcData[1]) //GP
                {
                    switch(pcData[2])
                    {
#ifndef FLARM_GPRMC_DISABLED
                        case 'R': //GPR
                        {
                            if(0 == strncmp("MC", (char*)&pcData[3], 2))
                            {
                                Flarm_GPRMC(lID, pcData, lLength);
                            }
                        }
                        break;
#endif
                        
                        case 'G': //GPG
                        {
                            switch(pcData[3])
                            {
#ifndef FLARM_GPGGA_DISABLED
                                case 'G': //GPGG
                                {
                                    if('A' == pcData[4])
                                    {
                                        Flarm_GPGGA(lID, pcData, lLength);
                                    }
                                }
                                break;
#endif
                                
                                case 'S': //GPGS
                                {
                                    if('A' == pcData[4])
                                    {
                                        Flarm_GPGSA(lID, pcData, lLength);
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
}

enum
{
    PFLAU_RX = 0,
    PFLAU_TX,
    PFLAU_GPS,
    PFLAU_POWER,
    PFLAU_ALARM_LEVEL,
    PFLAU_RELATIVE_BEARING,
    PFLAU_ALARM_TYPE,
    PFLAU_RELATIVE_VERTICAL,
    PFLAU_RELATIVE_DISTANCE,
    PFLAU_LENGTH
};

static void Flarm_PFLAU(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[PFLAU_LENGTH];
    bool Contents[PFLAU_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    PFLAU_LENGTH);

    uint8_t cContacts = (uint8_t)Flarm_GetInt(Tokens[PFLAU_RX]);
    bool bTXOk = (bool)Flarm_GetInt(Tokens[PFLAU_TX]);
    uint8_t cGPSStatus = (uint8_t)Flarm_GetInt(Tokens[PFLAU_GPS]);
    bool bPowerOk = (bool)Flarm_GetInt(Tokens[PFLAU_POWER]);
    uint8_t cAlarmLevel = (uint8_t)Flarm_GetInt(Tokens[PFLAU_ALARM_LEVEL]);
    int16_t snRelativeBearing = (int16_t)Flarm_GetInt(Tokens[PFLAU_RELATIVE_BEARING]);
    uint8_t cAlarmType = (uint8_t)Flarm_GetInt(Tokens[PFLAU_ALARM_TYPE]);
    int16_t snRelativeVertical = (int16_t)Flarm_GetInt(Tokens[PFLAU_RELATIVE_VERTICAL]);
    uint32_t lRelativeDistance = (uint32_t)Flarm_GetInt(Tokens[PFLAU_RELATIVE_DISTANCE]);

    _Flarm_PFLAU(lID,
                 cContacts,
                 bTXOk,
                 cGPSStatus,
                 bPowerOk,
                 cAlarmLevel,
                 snRelativeBearing,
                 cAlarmType,
                 snRelativeVertical,
                 lRelativeDistance);
}

enum
{
    PFLAA_ALARM_LEVEL = 0,
    PFLAA_RELATIVE_NORTH,
    PFLAA_RELATIVE_EAST,
    PFLAA_RELATIVE_VERTICAL,
    PFLAA_ID_TYPE,
    PFLAA_HEX_ID,
    PFLAA_TRACK,
    PFLAA_TURN_RATE,
    PFLAA_GROUND_SPEED,
    PFLAA_CLIMB_RATE,
    PFLAA_AIRCRAFT_TYPE,
    /* Data port version 8 & 9 only items */
    PFLAA_NO_TRACK,
    PFLAA_SOURCE,
    PFLAA_RSSI,
    PFLAA_LENGTH
};

static void Flarm_PFLAA(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[PFLAA_LENGTH];
    bool Contents[PFLAA_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    PFLAA_LENGTH);
                    
    uint8_t cAlarmLevel = (uint8_t)Flarm_GetInt(Tokens[PFLAA_ALARM_LEVEL]);
    int32_t slRelativeNorth = (int32_t)Flarm_GetInt(Tokens[PFLAA_RELATIVE_NORTH]);
    int32_t slRelativeEast = (int32_t)Flarm_GetInt(Tokens[PFLAA_RELATIVE_EAST]);
    int16_t snRelativeVertical = (int16_t)Flarm_GetInt(Tokens[PFLAA_RELATIVE_VERTICAL]);
    uint8_t cIDType = (uint8_t)Flarm_GetInt(Tokens[PFLAA_ID_TYPE]);
    /*(Just pass the raw hex ID)*/
    uint16_t nTrack = (uint16_t)Flarm_GetInt(Tokens[PFLAA_TRACK]);
    /*(As of 2022/06/06, turn rate is empty)*/
    uint16_t nGroundSpeed = (uint16_t)Flarm_GetInt(Tokens[PFLAA_GROUND_SPEED]);
    float fltClimbRate = Flarm_GetDecimal(Tokens[PFLAA_CLIMB_RATE]); /* TO DO: For performance, bin decimal and use int (x10)? */
    uint8_t cAircraftType = (uint8_t)Flarm_GetIntFromHex(Tokens[PFLAA_AIRCRAFT_TYPE]);
    
    _Flarm_PFLAA(lID,
                 cAlarmLevel,
                 slRelativeNorth,
                 slRelativeEast,
                 snRelativeVertical,
                 cIDType,
                 nTrack,
                 nGroundSpeed,
                 fltClimbRate,
                 cAircraftType);
}

enum
{
    PFLAE_QUERY_TYPE = 0,
    PFLAE_SEVERITY,
    PFLAE_ERROR_CODE,
    PFLAE_MESSAGE,
    PFLAE_LENGTH
};

static void Flarm_PFLAE(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[PFLAE_LENGTH];
    bool Contents[PFLAE_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    PFLAE_LENGTH);
                    
    /* Ignore query type */
    uint8_t cSeverity = (uint8_t)Flarm_GetInt(Tokens[PFLAE_SEVERITY]);
    uint16_t nErrorCode = (uint16_t)Flarm_GetIntFromHex(Tokens[PFLAE_ERROR_CODE]);
    uint8_t* pcMessage = Tokens[PFLAE_MESSAGE];
    
    _Flarm_PFLAE(lID,
                 cSeverity,
                 nErrorCode,
                 pcMessage);
}

enum
{
    PFLAV_QUERY_TYPE = 0,
    PFLAV_HW_VERSION,
    PFLAV_SW_VERSION,
    PFLAV_OBST_VERSION,
    PFLAV_LENGTH
};

static void Flarm_PFLAV(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[PFLAV_LENGTH];
    bool Contents[PFLAV_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    PFLAV_LENGTH);
                    
    /* Ignore query type */
    float fltHwVersion = Flarm_GetDecimal(Tokens[PFLAV_HW_VERSION]);
    float fltSwVersion = Flarm_GetDecimal(Tokens[PFLAV_SW_VERSION]);
    uint8_t* pcObstVersion = Tokens[PFLAV_OBST_VERSION];
    
    _Flarm_PFLAV(lID,
                 fltHwVersion,
                 fltSwVersion,
                 pcObstVersion);
}

#ifndef FLARM_GPRMC_DISABLED
enum
{
    GPRMC_TIME = 0,
    GPRMC_ACTIVE,
    GPRMC_LATITUDE,
    GPRMC_LAT_H_IND,
    GPRMC_LONGITUDE,
    GPRMC_LONG_H_IND,
    GPRMC_SPEED,
    GPRMC_TRACK,
    GPRMC_DATE,
    GPRMC_MAG_VAR,
    GPRMC_MAG_VAR_DIR,
    GPRMC_LENGTH,
};

#define GPRMC_ACTIVE 'A'

static void Flarm_GPRMC(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[GPRMC_LENGTH];
    bool Contents[GPRMC_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    GPRMC_LENGTH);

    float fltTime = Flarm_GetDecimal(Tokens[GPRMC_TIME]);
    bool bActive = GPRMC_ACTIVE == Flarm_GetChar(Tokens[GPRMC_ACTIVE]);
    float fltLatitude = Flarm_GetDecimal(Tokens[GPRMC_LATITUDE]);
    char cLatitudeHemisphere = Flarm_GetChar(Tokens[GPRMC_LAT_H_IND]);
    float fltLongitude = Flarm_GetDecimal(Tokens[GPRMC_LONGITUDE]);
    char cLongitudeHemisphere = Flarm_GetChar(Tokens[GPRMC_LONG_H_IND]);
    float fltSpeed = Flarm_GetDecimal(Tokens[GPRMC_SPEED]);
    float fltTrack = Flarm_GetDecimal(Tokens[GPRMC_TRACK]);
    uint32_t lDate = Flarm_GetInt(Tokens[GPRMC_DATE]);
    float fltMagVar = Flarm_GetDecimal(Tokens[GPRMC_MAG_VAR]);
    char cMagVarDirection = Flarm_GetChar(Tokens[GPRMC_MAG_VAR_DIR]);
    
    _Flarm_GPRMC(lID,
                 fltTime,
                 bActive,
                 fltLatitude,
                 cLatitudeHemisphere,
                 fltLongitude,
                 cLongitudeHemisphere,
                 fltSpeed,
                 fltTrack,
                 lDate,
                 fltMagVar,
                 cMagVarDirection);
}
#endif

#ifndef FLARM_GPGGA_DISABLED
enum
{
    GPGGA_TIME = 0,
    GPGGA_LATITUDE,
    GPGGA_LAT_H_IND,
    GPGGA_LONGITUDE,
    GPGGA_LONG_H_IND,
    GPGGA_QUALITY,
    GPGGA_SATELLITES,
    GPGGA_HDOP,
    GPGGA_ALTITUDE,
    GPGGA_ALT_UNIT,
    GPGGA_UNDULATION,
    GPGGA_UND_UNIT,
    GPGGA_LENGTH,
};

static void Flarm_GPGGA(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[GPGGA_LENGTH];
    bool Contents[GPGGA_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    GPGGA_LENGTH);

    float fltTime = Flarm_GetDecimal(Tokens[GPRMC_TIME]);
    float fltLatitude = Flarm_GetDecimal(Tokens[GPGGA_LATITUDE]);
    char cLatitudeHemisphere = Flarm_GetChar(Tokens[GPGGA_LAT_H_IND]);
    float fltLongitude = Flarm_GetDecimal(Tokens[GPGGA_LONGITUDE]);
    char cLongitudeHemisphere = Flarm_GetChar(Tokens[GPGGA_LONG_H_IND]);
    uint8_t cQuality = (uint8_t)Flarm_GetInt(Tokens[GPGGA_QUALITY]);
    uint8_t cSatellites = (uint8_t)Flarm_GetInt(Tokens[GPGGA_SATELLITES]);
    float fltHDOP = Flarm_GetDecimal(Tokens[GPGGA_HDOP]);
    float fltAltitude = Flarm_GetDecimal(Tokens[GPGGA_ALTITUDE]);
    char cAltitudeUnit = Flarm_GetChar(Tokens[GPGGA_ALT_UNIT]);
    float fltUndulation = Flarm_GetDecimal(Tokens[GPGGA_UNDULATION]);
    char cUndulationUnit = Flarm_GetChar(Tokens[GPGGA_UND_UNIT]);
    
    _Flarm_GPGGA(lID,
                 fltTime,
                 fltLatitude,
                 cLatitudeHemisphere,
                 fltLongitude,
                 cLongitudeHemisphere,
                 cQuality,
                 cSatellites,
                 fltHDOP,
                 fltAltitude,
                 cAltitudeUnit,
                 fltUndulation,
                 cUndulationUnit);
}
#endif

enum
{
    GPGSA_MODEAUTO,
    GPGSA_MODE,
    GPGSA_PRN1,
    GPGSA_PRN2,
    GPGSA_PRN3,
    GPGSA_PRN4,
    GPGSA_PRN5,
    GPGSA_PRN6,
    GPGSA_PRN7,
    GPGSA_PRN8,
    GPGSA_PRN9,
    GPGSA_PRN10,
    GPGSA_PRN11,
    GPGSA_PRN12,
    GPGSA_PDOP,
    GPGSA_HDOP,
    GPGSA_VDOP,
    /* Nobody cares about system ID. */
    GPGSA_LENGTH
};

static void Flarm_GPGSA(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    uint8_t* Tokens[GPGSA_LENGTH];
    bool Contents[GPGSA_LENGTH];
    
    Flarm_GetTokens(pcData,
                    lLength,
                    Tokens,
                    Contents,
                    GPGSA_LENGTH);

    char cModeAuto = Flarm_GetChar(Tokens[GPGSA_MODEAUTO]);
    char cMode = Flarm_GetChar(Tokens[GPGSA_MODE]);
    /* Nobody cares about PRN IDs. */
    float fltPDOP = Flarm_GetDecimal(Tokens[GPGSA_PDOP]);
    float fltHDOP = Flarm_GetDecimal(Tokens[GPGSA_HDOP]);
    float fltVDOP = Flarm_GetDecimal(Tokens[GPGSA_VDOP]);
    
    _Flarm_GPGSA(lID,
                 cModeAuto,
                 cMode,
                 fltPDOP,
                 fltHDOP,
                 fltVDOP);
}

static void Flarm_PGRMZ(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAQ(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAO(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAB(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

