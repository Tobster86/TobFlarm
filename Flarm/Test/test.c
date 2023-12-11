
#include <stdio.h>
#include "Flarm.h"

void _Flarm_Sentence(uint32_t lFlarmID, uint8_t* message, uint32_t lLength)
{
    printf("%.*s\n", lLength, message);
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

    return 0;
}

