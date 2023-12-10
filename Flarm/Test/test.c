
#include <stdio.h>
#include "Flarm.h"

int main()
{
    struct sdfFlarm sdcFlarm;
    
    Flarm_Init(&sdcFlarm, 0xAA55AA55u);

    return 0;
}

