//**********************************************************************************
//  parse_report.cpp 
//  Read Fidelity Investments report documents and perform various calculations
//  
//  Written by:  Derell Licht
//**********************************************************************************
// D:\SourceCode\Git\fidelity_calcs\reports\Fidelity 2990\Statement3312023.csv
// D:\SourceCode\Git\fidelity_calcs\reports\Fidelity 3185\Statement1312023.csv
// D:\SourceCode\Git\fidelity_calcs\reports\Fidelity 5534\Statement3312023.csv
// D:\SourceCode\Git\fidelity_calcs\reports\Fidelity 5706\Statement3312023.csv

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  PATH_MAX

#include "common.h"
#include "FidelityCalcs.h"

static char inpstr[512];
//**********************************************************************************
int process_text_file(char *fpath)
{
   uint lcount ;
   // printf("%s\n", fpath);
   FILE *fptr = fopen(fpath, "rt");
   if (fptr == NULL) {
      printf("%3u: %s\n", (uint) errno, fpath);
      return errno;
   }
   //  scan through all the lines and collect data
   lcount = 0 ;
   while(fgets(inpstr, sizeof(inpstr), fptr) != 0) {
      strip_newlines(inpstr);
      lcount++ ;
      // int result = wd_parse_data_row(inpstr);
      // if (result != 0) {
      //    printf("parse error [L %u]: %s\n", lcount, filename);
      //    goto exit_point ;
      // }
      // 
      // //  next, check max/min values against static wd_current
      // wd_check_records();
   }
   printf("%3u [%s]\n", lcount, fpath);
   
   fclose(fptr);

   return 0 ;
}
//**********************************************************************************
//lint -esym(818, ftemp)   //  parameter could be declared as pointing to const
int parse_fidelity_report(ffdata *ftemp)
{
   // int result = 
   process_text_file(ftemp->path_spec);   //lint !e534
   return 0;
}
