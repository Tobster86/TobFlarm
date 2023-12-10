
#include "Flarm.h"

#define NMEA_START      '$'
#define NMEA_CS_DELIMIT '*'
#define NMEA_END_CR     '\r'
#define NMEA_END_LF     '\n'

static void Flarm_Interpret(uint8_t* pcData, uint32_t lLength);

void Flarm_Init(struct sdfFlarm* psdcFlarm, uint32_t lID)
{
    memset(psdcFlarm, 0x00, sizeof(struct sdfFlarm));
    psdcFlarm->lID = lID;
}

void Flarm_RXProcess(struct sdfFlarm* psdcFlarm, uint8_t* pcData, uint32_t lLength)
{
    for(int i = 0; i < lLength; i++)
    {
        if(psdcFlarm->bGrabChecksum)
        {
            psdcFlarm->bGrabChecksum = false;
            
            if(pcData[i] == psdcFlarm->cChecksum)
            {
                Flarm_Interpret(psdcFlarm->buffer, psdcFlarm->lBufIdx);
            }
            
            psdcFlarm->bBuffering = false;
            psdcFlarm->lBufIdx = 0;
            psdcFlarm->cChecksum = 0;
        }
        else
        {
            switch(pcData[i])
            {
                case NMEA_START:
                {
                    /* Start buffering a new message. */
                    psdcFlarm->bBuffering = true;
                    psdcFlarm->lBufIdx = 0;
                    psdcFlarm->cChecksum = 0;
                }
                break;
                
                case NMEA_CS_DELIMIT:
                {
                    if(psdcFlarm->bBuffering)
                    {
                        /* End of message. Grab the checksum next. */
                        psdcFlarm->bGrabChecksum = true;
                    }
                }
                break;
                
                default:
                {
                    if(psdcFlarm->bBuffering)
                    {
                        if(psdcFlarm->lBufIdx >= NMEA_MAX_LENGTH)
                        {
                            /* Length limit exceeded. Bin this message. */
                            psdcFlarm->bBuffering = false;
                            psdcFlarm->lBufIdx = 0;
                            psdcFlarm->cChecksum = 0;
                        }
                        else
                        {
                            /* Continue buffering the message. */
                            psdcFlarm->buffer[psdcFlarm->lBufIdx++] = pcData[i];
                            psdcFlarm->cChecksum ^= pcData[i];
                        }
                    }
                }
            }
        }    
    }
}

static void Flarm_Interpret(uint8_t* pcData, uint32_t lLength)
{
    
}
