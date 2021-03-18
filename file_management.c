#include "data.h"
#include "file_management.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern int g_entries;
extern int g_address;
extern int g_externals;
extern struct Table *tableList;
extern struct Labels *labelsList;

/*Check that there weren't any errors with opening files*/
int checkFileOpen(FILE *input) {
  if (!input) {
    printf("Error opening file\n");
    return 0;
  }
  else
    return 1;
}
/*Removes extra spaces from the line for easier parsing and removes all the starting spaces*/
/*as well as comments and returns 0 if the line whole line is empty or a comment line      */
char* removeUnwanted(char *str){
  int i, x, inQuotes;
  
  if((strchr(str, ';'))){   /*If there is a comment in the middle of the line we just put a \0 where the ; is and that way we just ignore everything after that*/             
        for (inQuotes = i = 0; str[i]; i++){  /*Checking that ; isn't inside a string*/ 
            if(str[i] == '\"'){
                if(inQuotes == 0)
                    inQuotes = 1;   /*Inside string*/
                else
                    inQuotes = 0;   /*Outside string*/
            }
            if(str[i] == ';' && inQuotes == 0){
                str[i] = '\0';       
                break;
            }                            
        }
    }
    for(i = x = 0; str[i]; ++i){   /*Removes extra/starting spaces*/
        if(!isspace(str[i]) || (i > 0 && !isspace(str[i - 1])))
            str[x++] = str[i];
    }
    str[x] = '\0';
    return str;                   /*If it reached here then the line is not empty / comment line*/
}
/*Outputs the hex, extern and entries to files*/
void outputToFile(char *fileName, int DC){
    struct Table *last_table_head = tableList;
    struct Labels *last_label_head = labelsList;
    FILE *outputFile;
    int index;
    char hex[HEX_CHARACTERS], *file = (char*)malloc((strlen(fileName)+FILE_EXTENSION_LENGTH+1) * sizeof(char));

    /*Output of main .ob file*/
    sprintf(file, "%s.ob", fileName);
    outputFile = fopen(file, "w");
    checkFileOpen(outputFile);
    fprintf(outputFile,"  %d %d  \n", g_address-100-DC, DC);
    while(last_table_head){
        sprintf(hex,"%03X", (int)strtol(last_table_head->machineCode,NULL,2));
        fprintf(outputFile,"%04d %s %c\n",last_table_head->address, hex, last_table_head->ARE);
        last_table_head = last_table_head->next;
    }
    fclose(outputFile);
    index = (strrchr(file ,'.') - file);
    file[index] = '\0';
    if(g_externals > 0){ /*check here*/
        last_table_head = tableList;
        sprintf(file, "%s.ext", fileName);
        outputFile = fopen(file, "w");
        checkFileOpen(outputFile);
        while(last_table_head && g_externals > 0){
            if(last_table_head->ARE == 'E'){
                fprintf(outputFile,"%s %04d\n",last_table_head->sourceCode, last_table_head->address);
                g_externals--;
            }
            last_table_head = last_table_head->next;
        }
    fclose(outputFile);
    }
    index = (strrchr(file ,'.') - file);
    file[index] = '\0';
    if(g_entries > 0){
        sprintf(file, "%s.ent", fileName);
        outputFile = fopen(file, "w");
        checkFileOpen(outputFile);
        while(last_label_head && g_entries > 0){
            if(strstr(last_label_head->attribute, "entry")){
                fprintf(outputFile,"%s %04d\n",last_label_head->symbol, last_label_head->address);
                g_entries--;
            }
            last_label_head = last_label_head->next;           
        }
        fclose(outputFile);
    }
    free(file);
}