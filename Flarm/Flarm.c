
#include "Flarm.h"

#define NMEA_START      '$'
#define NMEA_CS_DELIMIT '*'
#define NMEA_END_CR     '\r'
#define NMEA_END_LF     '\n'

#define FLARM_STATE_IDLE 0x01
#define FLARM_STATE_BUFF 0x02
#define FLARM_STATE_CHKS 0x03

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
    char token = strtok(pcData, ",");
    
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
                                    case 'A': Flarm_PFLAA(lID); break;
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
    
}

static void Flarm_PFLAA(uint32_t lID)
{
    
}

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

