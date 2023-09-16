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
uint month ;
uint year ;
} acct_id_t, *acct_id_p ;

acct_id_p acct_id_top = NULL ;
acct_id_p acct_id_tail = NULL ;

//**********************************************************************************
static void parse_month_year(acct_id_p acct_id_temp, char const * const fname)
{
   char dtstr[20];
   char monstr[3];
   
   //  parse month/date from filename
   char *stemp = strstr(fname, "Statement"); //lint !e158  asmt to stemp increases capability
   if (stemp == NULL) {
      acct_id_temp->month = (uint) 0;
      acct_id_temp->year  = (uint) 0;
      return ;
   }
   stemp += 9 ;   //  point to date string
   strcpy(dtstr, stemp);
   
   //  truncate file extension, leaving just m[m]ddyyyy
   stemp = strchr(dtstr, '.');
   if (stemp == NULL) {
      acct_id_temp->month = (uint) 0;
      acct_id_temp->year  = (uint) 0;
      return ;
   }
   *stemp = 0 ;
   
   uint slen = strlen(dtstr);
   switch (slen) {
   case 7:
      monstr[0] = dtstr[0];
      monstr[1] = 0 ;
      acct_id_temp->month = (uint) atoi(monstr);
      acct_id_temp->year  = (uint) atoi(&dtstr[3]);
      break ;
      
   case 8:
      monstr[0] = dtstr[0];
      monstr[1] = dtstr[1];
      monstr[2] = 0 ;
      acct_id_temp->month = (uint) atoi(monstr);
      acct_id_temp->year  = (uint) atoi(&dtstr[3]);
      break ;
      
   default:
      acct_id_temp->month = (uint) 0;
      acct_id_temp->year  = (uint) 0;
      return ;
   }
}

//**********************************************************************************
static acct_id_p add_account(char *fund_name, char *account_num, char const * const fname)
{
   //  D:\SourceCode\Git\fidelity_calcs\reports\Fidelity 5706\Statement3312023.csv
   // printf("acct: %s, fund: %s\n", account_num, fund_name);
   acct_id_p acct_id_temp ;

   for (acct_id_temp = acct_id_top;
        acct_id_temp != NULL;
        acct_id_temp = acct_id_temp->next) {
      if (strcmp(acct_id_temp->acct_num, account_num) == 0) {
         //  return true if this item is already in list
         parse_month_year(acct_id_temp, fname);
         printf("found %s [%s]\n", account_num, fund_name);
         return acct_id_temp ;
      }
   }
   
   acct_id_temp = new acct_id_t ;
   ZeroMemory((char *) acct_id_temp, sizeof(acct_id_t));
   strcpy(acct_id_temp->fund_name, fund_name);
   strcpy(acct_id_temp->acct_num, account_num);
   
   parse_month_year(acct_id_temp, fname);
   
   printf("adding %s m%u y%u [%s]\n", 
      acct_id_temp->acct_num,
      acct_id_temp->month,
      acct_id_temp->year,
      acct_id_temp->fund_name);
   
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
//  copy chars from src to dest until comma or 0 are encountered
//**********************************************************************************
char *strccpy(char *src, char *dest, unsigned max_len)
{
   if (src == NULL  ||  dest == NULL) {
      return NULL ;
   }
   unsigned slen = 0 ;
   while (LOOP_FOREVER) {
      if (*src == ','  ||  *src == 0  ||  slen >= max_len) {
         *dest = 0 ;
         if (*src == ',') {
            src++ ;  //  skip terminating character
         }
         
         return (slen >= max_len) ? NULL : src ;
      }
      *dest++ = *src++ ;
   }
}

//**********************************************************************************
// Symbol/CUSIP,Description,Quantity,Price,Beginning Value,Ending Value,Cost Basis
// FNCMX,FIDELITY NASDAQ COMPOSITE INDEX ,78.74000,154.38000,10385.02,12155.88,7781.96
// 0: [FNCMX]
// 1: [FIDELITY NASDAQ COMPOSITE INDEX ]
// 2: [78.74000]
// 3: [154.38000]
// 4: [10385.02]
// 5: [12155.88]
// 6: [7781.96]
//FIDELITY SELECT MED TECHNOLOGY & DEVICES 
//**********************************************************************************
static void parse_fund_data(char *ldata, acct_id_p acct_id_temp)
{
   char *src = ldata ;
   char dest[20+1];
   char cusip[5+1] = "" ;
   char desc[60+1] = "" ;
   char end_value[20+1] = "" ;
   
   if (acct_id_temp == NULL) {
      return ;
   }
   int lcount = 0 ;
   bool dvalid = false ;
   while (LOOP_FOREVER) {
      switch (lcount) {
      case 0:  //  cusip
         src = strccpy(src, cusip, 6);
         break ;
      case 1:  //  description of fund
         src = strccpy(src, desc, 60);
         break ;
      case 2:  //  quantity (skip)
         src = strccpy(src, dest, 60);
         break ;
      case 3:  //  price (skip)
         src = strccpy(src, dest, 60);
         break ;
      case 4:  //  beginning value (skip)
         src = strccpy(src, dest, 60);
         break ;
      case 5:  //  ending value
         src = strccpy(src, end_value, 20);
         break ;
      default:
         break ;
      }
      if (src == NULL) {
         dvalid = false ;
         break ;
      }
      if (*src == 0) {
         dvalid = false ;
         break ;
      }
      lcount++ ;
      if (lcount >= 6) {
         dvalid = true ;
         break ;
      }
   }
   
   //  now, process results
   if (dvalid) {
//06 2023: cusip: [FSCRX], desc: [FIDELITY SMALL CAP DISCOVERY FUND ], end_value: [63709.60]
      printf("%02u %04u: ", acct_id_temp->month, acct_id_temp->year);
      printf("cusip: [%s], ", cusip);
      printf("desc: [%s], ", desc);
      printf("end_value: [%s]\n", end_value);
   }
}  //lint !e818

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
static int process_text_file(char const * const fpath)
{
   acct_id_p acct_id_temp = NULL ;
   printf("\n%s\n", fpath);
   FILE *fptr = fopen(fpath, "rt");
   if (fptr == NULL) {
      printf("%3u: %s\n", (uint) errno, fpath);
      return errno;
   }
   //  scan through all the lines and collect data
   char *ctemp ;
   uint pstate = 0 ;
   bool fvalid = true ;
   while(fgets(inpstr, sizeof(inpstr), fptr) != 0) {
      strip_newlines(inpstr);
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
         acct_id_temp = add_account(fund_name, account_num, fpath);
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
            parse_fund_data(inpstr, acct_id_temp);
            break ;

         case 5:  //  read to end of file
         default:
            break ;      
      }  //  switch pstate
      
      if (!fvalid) {
         break ;
      }
   }  //  while reading lines from file
   fclose(fptr);
   // printf("%3u [%u] [%s]\n\n", lcount, (uint) fvalid, fpath);

   return 0 ;
}
//**********************************************************************************
//lint -esym(818, ftemp)   //  parameter could be declared as pointing to const
int parse_fidelity_report(ffdata *ftemp)
{
   process_text_file(ftemp->path_spec);   //lint !e534
   return 0;
}
