
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
static void Flarm_PFLAR(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_GPRMC(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_GPGGA(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_GPGSA(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_GPTXT(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PGRMZ(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAS(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAQ(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAO(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAI(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAC(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAJ(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAB(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAF(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAL(uint32_t lID, uint8_t* pcData, uint32_t lLength);

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
    //printf("%.*s\n", lLength, pcData);
    
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
                                            
#ifndef FLARM_PFLAA_DISABLED
                                    case 'A': Flarm_PFLAA(lID, pcData, lLength); break;
#endif
                                    
                                    case 'E': Flarm_PFLAE(lID, pcData, lLength); break;
                                    case 'V': Flarm_PFLAV(lID, pcData, lLength); break;
                                    case 'R': Flarm_PFLAR(lID, pcData, lLength); break;
                                    case 'S': Flarm_PFLAS(lID, pcData, lLength); break;
                                    case 'Q': Flarm_PFLAQ(lID, pcData, lLength); break;
                                    case 'O': Flarm_PFLAO(lID, pcData, lLength); break;
                                    case 'I': Flarm_PFLAI(lID, pcData, lLength); break;
                                    case 'C': Flarm_PFLAC(lID, pcData, lLength); break;
                                    case 'J': Flarm_PFLAJ(lID, pcData, lLength); break;
                                    case 'B': Flarm_PFLAB(lID, pcData, lLength); break;
                                    case 'F': Flarm_PFLAF(lID, pcData, lLength); break;
                                    case 'L': Flarm_PFLAL(lID, pcData, lLength); break;
                                }
                            }
                        }
                    }
                    break;
                    
                    case 'G': //PG
                    {
                        if(0 == strcmp("RMZ", (char*)&pcData[2]))
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
                        case 'R': //GPR
                        {
                            if(0 == strcmp("MC", (char*)&pcData[3]))
                            {
                                Flarm_GPRMC(lID, pcData, lLength);
                            }
                        }
                        break;
                        
                        case 'G': //GPG
                        {
                            switch(pcData[3])
                            {
                                case 'G': //GPGG
                                {
                                    if('A' == pcData[4])
                                    {
                                        Flarm_GPGGA(lID, pcData, lLength);
                                    }
                                }
                                break;
                                
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
                        
                        case 'T': //GPT
                        {
                            if(0 == strcmp("XT", (char*)&pcData[3]))
                            {
                                Flarm_GPTXT(lID, pcData, lLength);
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

#ifndef FLARM_PLFAA_DISABLED
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
#endif

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

static void Flarm_PFLAV(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAR(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_GPRMC(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_GPGGA(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_GPGSA(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_GPTXT(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PGRMZ(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAS(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAQ(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAO(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAI(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAC(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAJ(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAB(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAF(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

static void Flarm_PFLAL(uint32_t lID, uint8_t* pcData, uint32_t lLength)
{
    
}

