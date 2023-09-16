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

//Trust: Under Agreement
#define  MAX_FUND_LEN      30
#define  MAX_ACCT_NUM_LEN  9
typedef struct acct_id_s {
struct acct_id_s *next ;
char fund_name[MAX_FUND_LEN+1];
char acct_num[MAX_ACCT_NUM_LEN+1];
} acct_id_t, *acct_id_p ;

acct_id_p acct_id_top = NULL ;
acct_id_p acct_id_tail = NULL ;

//**********************************************************************************
static acct_id_p add_account(char *fund_name, char *account_num)
{
   // printf("acct: %s, fund: %s\n", account_num, fund_name);
   acct_id_p acct_id_temp ;

   for (acct_id_temp = acct_id_top;
        acct_id_temp != NULL;
        acct_id_temp = acct_id_temp->next) {
      if (strcmp(acct_id_temp->acct_num, account_num) == 0) {
         //  return true if this item is already in list
         printf("found %s [%s]\n", account_num, fund_name);
         return acct_id_temp ;
      }
   }
   
   printf("adding %s [%s]\n", account_num, fund_name);
   acct_id_temp = new acct_id_t ;
   strcpy(acct_id_temp->fund_name, fund_name);
   strcpy(acct_id_temp->acct_num, account_num);
   acct_id_temp->next = NULL ;
   
   if (acct_id_top == NULL) {
      acct_id_top = acct_id_temp ;
   }
   else {
      acct_id_tail->next = acct_id_temp ;
   }
   acct_id_tail = acct_id_temp ;
   return acct_id_temp ;
}

//**********************************************************************************
// Account Type,Account,Beginning mkt Value,Change in Investment,Ending mkt Value,
//    Short Balance,Ending Net Value,Dividends This Period,Dividends Year to Date,
//    Interest This Year,Interest Year to Date,Total This Period,Total Year to Date
// Roth IRA - BDA,230465706,243233.75,-2179.05,241054.70,,,,,,,0.58,0.58
//  
// 
// Symbol/CUSIP,Description,Quantity,Price,Beginning Value,Ending Value,Cost Basis
// , 
// 
// 230465706 
// Mutual Funds 
// FNCMX,FIDELITY NASDAQ COMPOSITE INDEX ,78.74000,154.38000,10385.02,12155.88,7781.96
// FSPHX,FIDELITY SELECT HEALTH CARE ,747.33200,27.78000,20641.31,20760.88,8550.02
// FSMEX,FIDELITY SELECT MED TECHNOLOGY & DEVICES ,179.40300,64.67000,11047.64,11601.99,7643.07
// FSHCX,FIDELITY SELECT HEALTH CARE SRVC ,222.80900,122.57000,29410.79,27309.70,3125.70
// FBIOX,FIDELITY SELECT BIOTECHNOLOGY ,6787.77500,15.63000,110504.98,106092.92,14871.88
// FSDAX,FIDELITY SELECT DEFENSE & AEROSPACE ,3934.87800,16.03000,61187.35,63076.09,9554.07
// Subtotal of Mutual Funds,,,,,240997.46,51526.70,,, 
// 
// 230465706 
// Core Account 
// SPAXX,FIDELITY GOVERNMENT MONEY MARKET ,57.24000,1.00000,56.66,57.24,not applicable
// Subtotal of Core Account,,,,,57.24,,,,,
//**********************************************************************************
static int process_text_file(char *fpath)
{
   uint lcount ;
   acct_id_p acct_id_temp = NULL ;
   // printf("%s\n", fpath);
   FILE *fptr = fopen(fpath, "rt");
   if (fptr == NULL) {
      printf("%3u: %s\n", (uint) errno, fpath);
      return errno;
   }
   //  scan through all the lines and collect data
   char *ctemp ;
   lcount = 0 ;
   uint pstate = 0 ;
   bool fvalid = true ;
   while(fgets(inpstr, sizeof(inpstr), fptr) != 0) {
      strip_newlines(inpstr);
      lcount++ ;
      switch (pstate) {
      case 0:  //Account Type,Account,Beginning mkt Value,
         if (strncmp(inpstr, "Account Type", 12) != 0) {
            fvalid = false ;
            break ;
         }
         pstate = 1 ;
         break ;
         
      //  fund name, account number
      case 1:  //IRA - BDA,172162990,58276.29,3097.84,61374.13,,,,,,,,
         {
         char *fund_name = inpstr ;
         char *account_num = strchr(inpstr,',');
         if (account_num == NULL) {
            fvalid = false ;
            break ;
         }
         *account_num = 0 ;
         account_num++ ;
         ctemp = strchr(account_num, ',') ;
         if (ctemp == NULL) {
            fvalid = false ;
            break ;
         }
         *ctemp = 0 ;
         //  okay, parse is sufficient...
         acct_id_temp = add_account(fund_name, account_num);
         if (acct_id_temp == NULL) {
            fvalid = false ;
         }
         pstate = 2 ;
         }
         break ;
         
         //  searching for line matching account number
         case 2:
//lint -esym(613, acct_id_temp)  Possible use of null pointer in left argument to operator '->'
            if (strncmp(acct_id_temp->acct_num, inpstr, MAX_ACCT_NUM_LEN) == 0) {
               // printf("found entry for %s\n", acct_id_temp->acct_num);
               pstate = 3 ;
            }
            break ;

         // Mutual Funds 
         // Core Account 
         case 3:
            if (strncmp(inpstr, "Mutual Funds", 12) == 0) {
               // printf("found entry for %s Mutual Funds\n", acct_id_temp->acct_num);
               pstate = 4 ;
            }
            break ;

// FNCMX,FIDELITY NASDAQ COMPOSITE INDEX ,78.74000,154.38000,10385.02,12155.88,7781.96
// FSPHX,FIDELITY SELECT HEALTH CARE ,747.33200,27.78000,20641.31,20760.88,8550.02
// FSMEX,FIDELITY SELECT MED TECHNOLOGY & DEVICES ,179.40300,64.67000,11047.64,11601.99,7643.07
// FSHCX,FIDELITY SELECT HEALTH CARE SRVC ,222.80900,122.57000,29410.79,27309.70,3125.70
// FBIOX,FIDELITY SELECT BIOTECHNOLOGY ,6787.77500,15.63000,110504.98,106092.92,14871.88
// FSDAX,FIDELITY SELECT DEFENSE & AEROSPACE ,3934.87800,16.03000,61187.35,63076.09,9554.07
// Subtotal of Mutual Funds,,,,,240997.46,51526.70,,, 
         case 4:
            if (strncmp(inpstr, "Subtotal", 8) == 0) {
               // printf("found end of %s Mutual Funds\n", acct_id_temp->acct_num);
               pstate = 5 ;
               break ;
            }
            printf("%s\n", inpstr);
            break ;

         case 5:  //  read to end of file
         default:
            break ;      
      }  //  switch pstate
      
      if (!fvalid) {
         break ;
      }
   }  //  while reading lines from file
   printf("%3u [%u] [%s]\n", lcount, (uint) fvalid, fpath);
   
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
