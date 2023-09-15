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

//**********************************************************************************
int parse_fidelity_report(ffdata *ftemp)
{
   printf("%s\n", ftemp->path_spec);
   return 0;
}
