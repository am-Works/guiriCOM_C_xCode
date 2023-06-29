//
//  main.c
//  guiriCOM_C_xCode
//
//  Created by Jose Andrade on 15/06/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "gCOM.h"
#include "gParser.h"

int main(int argc, const char * argv[])
{
    
    FILE *pFile;
    
    if ((pFile = fopen ("../guiriCOM_C_xCode_MingoPrograms/B1.txt","r")) == NULL)
    {
        printf("Program file can't be opened / Problemas al abrir el archivo con el programa\n\n");
        printf("Value of error number: %d\n", errno);
        perror("Error message by perror");
        
        exit(EXIT_FAILURE);
    }
    else
    {
        char strMingoProgram[1000];
        int index = 0;
        
        do
        {
            
            strMingoProgram[index] = fgetc(pFile);
            
        } while ( strMingoProgram[index++] != EOF);
        
        fclose (pFile);
        
        strMingoProgram[index-1] = 0;
        
        printf("---------- Program / Programa ----------\n\n");
        printf("%s\n\n", strMingoProgram);
        printf("--------- Execution / Ejecucion --------\n\n");
        
        interpreter(strMingoProgram);
    }
    
    printf("\n\n");

    return 0;   //exit(EXIT_SUCCESS);
}
