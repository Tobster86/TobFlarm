
#include "Flarm.h"

#define NMEA_START      '$'
#define NMEA_CS_DELIMIT '*'
#define NMEA_END_CR     '\r'
#define NMEA_END_LF     '\n'

#define FLARM_STATE_IDLE 0x01
#define FLARM_STATE_BUFF 0x02
#define FLARM_STATE_CHKS 0x03

static void Flarm_Interpret(uint32_t lID, uint8_t* pcData, uint32_t lLength);

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
    _Flarm_Sentence(lID, pcData, lLength);
}

