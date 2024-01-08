
#include "Flarm.h"

#define NMEA_START      '$'
#define NMEA_CS_DELIMIT '*'
#define NMEA_END_CR     '\r'
#define NMEA_END_LF     '\n'

#define FLARM_STATE_IDLE 0x01
#define FLARM_STATE_BUFF 0x02
#define FLARM_STATE_CHKS 0x03

#define NEXT_TOKEN strtok(NULL, ",");

#define GET_INT(pTX) strtol((pTX), NULL, 10);
#define GET_BOOL(pTX) strtol((pTX), NULL, 10);
#define GET_UINT8(pTX) strtol((pTX), NULL, 10);
#define GET_UINT8_FROM_HEX(pTX) strtol((pTX), NULL, 16);
#define GET_UINT16(pTX) strtol((pTX), NULL, 10);
#define GET_INT16(pTX) strtol((pTX), NULL, 10);
#define GET_UINT32(pTX) strtol((pTX), NULL, 10);
#define GET_INT32(pTX) strtol((pTX), NULL, 10);
#define GET_FLOAT(pTX) strtof((pTX), NULL);

static void Flarm_Interpret(uint32_t lID, uint8_t* pcData, uint32_t lLength);
static void Flarm_PFLAU(uint32_t lID);
static void Flarm_PFLAA(uint32_t lID);
static void Flarm_PFLAE(uint32_t lID);
static void Flarm_PFLAV(uint32_t lID);
static void Flarm_PFLAR(uint32_t lID);
static void Flarm_GPRMC(uint32_t lID);
static void Flarm_GPGGA(uint32_t lID);
static void Flarm_GPGSA(uint32_t lID);
static void Flarm_GPTXT(uint32_t lID);
static void Flarm_PGRMZ(uint32_t lID);
static void Flarm_PFLAS(uint32_t lID);
static void Flarm_PFLAQ(uint32_t lID);
static void Flarm_PFLAO(uint32_t lID);
static void Flarm_PFLAI(uint32_t lID);
static void Flarm_PFLAC(uint32_t lID);
static void Flarm_PFLAJ(uint32_t lID);
static void Flarm_PFLAB(uint32_t lID);
static void Flarm_PFLAF(uint32_t lID);
static void Flarm_PFLAL(uint32_t lID);

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
                        
                        if(NMEA_CS_CHARS >= psdcFlarm->lCSIdx)
                        {
                            char checksum[NMEA_CS_CHARS];
                            snprintf(checksum, NMEA_CS_CHARS, "%02X", psdcFlarm->cChecksumCalced);
                            
                            if(0 == memcmp(psdcFlarm->cChecksumStored, checksum, NMEA_CS_CHARS))
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
    char* token = strtok(pcData, ",");
    
    if(token)
    {
        switch(token[0])
        {
            case 'P': //P
            {
                switch(token[1])
                {
                    case 'F': //PF
                    {
                        if('L' == token[2]) //PFL
                        {
                            if('A' == token[3]) //PFLA
                            {
                                switch(token[4])
                                {
                                    case 'U': Flarm_PFLAU(lID); break;
                                            
#ifndef FLARM_PLFAA_DISABLED
                                    case 'A': Flarm_PFLAA(lID); break;
#endif
                                    
                                    case 'E': Flarm_PFLAE(lID); break;
                                    case 'V': Flarm_PFLAV(lID); break;
                                    case 'R': Flarm_PFLAR(lID); break;
                                    case 'S': Flarm_PFLAS(lID); break;
                                    case 'Q': Flarm_PFLAQ(lID); break;
                                    case 'O': Flarm_PFLAO(lID); break;
                                    case 'I': Flarm_PFLAI(lID); break;
                                    case 'C': Flarm_PFLAC(lID); break;
                                    case 'J': Flarm_PFLAJ(lID); break;
                                    case 'B': Flarm_PFLAB(lID); break;
                                    case 'F': Flarm_PFLAF(lID); break;
                                    case 'L': Flarm_PFLAL(lID); break;
                                }
                            }
                        }
                    }
                    break;
                    
                    case 'G': //PG
                    {
                        if(0 == strcmp("RMZ", &token[2]))
                        {
                            Flarm_PGRMZ(lID);
                        }
                    }
                    break;
                }
            }
            break;
            
            case 'G': //G
            {
                if('P' == token[1]) //GP
                {
                    switch(token[2])
                    {
                        case 'R': //GPR
                        {
                            if(0 == strcmp("MC", &token[3]))
                            {
                                Flarm_GPRMC(lID);
                            }
                        }
                        break;
                        
                        case 'G': //GPG
                        {
                            switch(token[3])
                            {
                                case 'G': //GPGG
                                {
                                    if('A' == token[4])
                                    {
                                        Flarm_GPGGA(lID);
                                    }
                                }
                                break;
                                
                                case 'S': //GPGS
                                {
                                    if('A' == token[4])
                                    {
                                        Flarm_GPGSA(lID);
                                    }
                                }
                                break;
                            }
                        }
                        break;
                        
                        case 'T': //GPT
                        {
                            if(0 == strcmp("XT", &token[3]))
                            {
                                Flarm_GPTXT(lID);
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

static void Flarm_PFLAU(uint32_t lID)
{
    char* pRX = NEXT_TOKEN;
    
    if(pRX)
    {
        char* pTX = NEXT_TOKEN;
        
        if(pTX)
        {
            char* pGPS = NEXT_TOKEN;
            
            if(pGPS)
            {
                char* pPower = NEXT_TOKEN;
                
                if(pPower)
                {
                    char* pAlarmLevel = NEXT_TOKEN;
                    
                    if(pAlarmLevel)
                    {
                        char* pRelativeBearing = NEXT_TOKEN;
                        
                        if(pRelativeBearing)
                        {
                            char* pAlarmType = NEXT_TOKEN;
                            
                            if(pAlarmType)
                            {
                                char* pRelativeVertical = NEXT_TOKEN;
                                
                                if(pRelativeVertical)
                                {
                                    char* pRelativeDistance = NEXT_TOKEN;
                                    
                                    if(pRelativeDistance)
                                    {
                                        uint8_t cContacts = GET_UINT8(pRX);
                                        bool bTXOk = GET_BOOL(pTX);
                                        uint8_t cGPSStatus = GET_UINT8(pGPS);
                                        bool bPowerOk = GET_BOOL(pPower);
                                        uint8_t cAlarmLevel = GET_UINT8(pAlarmLevel);
                                        int16_t snRelativeBearing = GET_INT16(pRelativeBearing);
                                        uint8_t cAlarmType = GET_UINT8(pAlarmType);
                                        int16_t snRelativeVertical = GET_INT16(pRelativeVertical);
                                        uint32_t lRelativeDistance = GET_UINT32(pRelativeDistance);
                                        
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
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#ifndef FLARM_PLFAA_DISABLED
static void Flarm_PFLAA(uint32_t lID)
{
    char* pAlarmLevel = NEXT_TOKEN;
    
    if(pAlarmLevel)
    {
        char* pRelativeNorth = NEXT_TOKEN;
        
        if(pRelativeNorth)
        {
            char* pRelativeEast = NEXT_TOKEN;
            
            if(pRelativeEast)
            {
                char* pRelativeVertical = NEXT_TOKEN;
                
                if(pRelativeVertical)
                {
                    char* pIDType = NEXT_TOKEN;
                    
                    if(pIDType)
                    {
                        char* pHexID = NEXT_TOKEN;
                        
                        if(pHexID)
                        {
                            char* pTrack = NEXT_TOKEN;
                            
                            if(pTrack)
                            {
                                char* pTurnRate = NEXT_TOKEN;
                                
                                if(pTurnRate)
                                {
                                    char* pGroundSpeed = NEXT_TOKEN;
                                    
                                    if(pGroundSpeed)
                                    {
                                        char* pClimbRate = NEXT_TOKEN;
                                        
                                        if(pClimbRate)
                                        {
                                            char* pAircraftType = NEXT_TOKEN;
                                            
                                            if(pAircraftType)
                                            {
                                                /* Data port version 8 & 9 only items, check seperately */
                                                char* pNoTrack = NEXT_TOKEN;
                                                char* pSource = NEXT_TOKEN;
                                                char* pRSSI = NEXT_TOKEN;
                                                
                                                uint8_t cAlarmLevel = GET_UINT8(pAlarmLevel);
                                                int32_t slRelativeNorth = GET_INT32(pRelativeNorth);
                                                int32_t slRelativeEast = GET_INT32(pRelativeEast);
                                                int16_t snRelativeVertical = GET_INT16(pRelativeVertical);
                                                uint8_t cIDType = GET_UINT8(pIDType);
                                                /*(Just pass the raw hex ID)*/
                                                uint16_t nTrack = GET_UINT16(pTrack);
                                                /*(As of 2022/06/06, turn rate is empty)*/
                                                uint16_t nGroundSpeed = GET_UINT16(pGroundSpeed);
                                                float fltClimbRate = GET_FLOAT(pClimbRate); /* TO DO: For performance, bin decimal and use int (x10)? */
                                                uint8_t cAircraftType = GET_UINT8_FROM_HEX(pAircraftType);
                                                
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
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif

static void Flarm_PFLAE(uint32_t lID)
{
    
}

static void Flarm_PFLAV(uint32_t lID)
{
    
}

static void Flarm_PFLAR(uint32_t lID)
{
    
}

static void Flarm_GPRMC(uint32_t lID)
{
    
}

static void Flarm_GPGGA(uint32_t lID)
{
    
}

static void Flarm_GPGSA(uint32_t lID)
{
    
}

static void Flarm_GPTXT(uint32_t lID)
{
    
}

static void Flarm_PGRMZ(uint32_t lID)
{
    
}

static void Flarm_PFLAS(uint32_t lID)
{
    
}

static void Flarm_PFLAQ(uint32_t lID)
{
    
}

static void Flarm_PFLAO(uint32_t lID)
{
    
}

static void Flarm_PFLAI(uint32_t lID)
{
    
}

static void Flarm_PFLAC(uint32_t lID)
{
    
}

static void Flarm_PFLAJ(uint32_t lID)
{
    
}

static void Flarm_PFLAB(uint32_t lID)
{
    
}

static void Flarm_PFLAF(uint32_t lID)
{
    
}

static void Flarm_PFLAL(uint32_t lID)
{
    
}

