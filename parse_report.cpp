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

typedef struct fund_entry_s {
   struct fund_entry_s *next ;
   uint month ;
   uint year ;
   double end_value ;
} fund_entry_t, *fund_entry_p ;

#define  LEN_CUSIP   5
#define  LEN_DESC    60

typedef struct fund_info_s {
   struct fund_info_s *next ;
   char cusip[LEN_CUSIP+1] ;
   char desc[LEN_DESC+1] ;
   fund_entry_p fe_top ;
   fund_entry_p fe_tail ;
} fund_info_t, *fund_info_p ;

//Trust: Under Agreement
#define  MAX_FUND_LEN      30
#define  MAX_ACCT_NUM_LEN  9

typedef struct acct_id_s {
   struct acct_id_s *next ;
   char fund_name[MAX_FUND_LEN+1];
   char acct_num[MAX_ACCT_NUM_LEN+1];
   uint month ;
   uint year ;
   fund_info_p fi_top ;
   fund_info_p fi_tail ;
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
      acct_id_temp->year  = (uint) atoi(&dtstr[4]);
      break ;
      
   default:
      acct_id_temp->month = (uint) 0;
      acct_id_temp->year  = (uint) 0;
      return ;
   }
}

//**********************************************************************************
static acct_id_p add_account(char const * const fund_name, char const * const account_num, char const * const fname)
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
         // printf("found %s [%s]\n", account_num, fund_name);
         return acct_id_temp ;
      }
   }
   
   acct_id_temp = new acct_id_t ;
   ZeroMemory((char *) acct_id_temp, sizeof(acct_id_t));
   strcpy(acct_id_temp->fund_name, fund_name);
   strcpy(acct_id_temp->acct_num, account_num);
   
   parse_month_year(acct_id_temp, fname);
   
   // printf("\nAccount %s [%s]\n", acct_id_temp->acct_num, acct_id_temp->fund_name);
   
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
static void add_fund_entry(fund_info_p fi_temp, acct_id_p acct_id_temp, char const * const end_value)
{
   fund_entry_p fe_temp ;
   fe_temp = new fund_entry_t ;
   ZeroMemory((char *) fe_temp, sizeof(fund_entry_t));
   fe_temp->month = acct_id_temp->month ;
   fe_temp->year  = acct_id_temp->year ;
   fe_temp->end_value = strtod(end_value, NULL);   //lint !e119
   
   // printf("%s: %02u %04u %9.2f\n", fi_temp->cusip, 
   //    fe_temp->month, fe_temp->year, fe_temp->end_value);

   //  add new fund entry to list   
   if (fi_temp->fe_top == NULL) {
      fi_temp->fe_top = fe_temp ;
   }
   else {
      fi_temp->fe_tail->next = fe_temp ;
   }
   fi_temp->fe_tail = fe_temp ;
}  //lint !e818

//**********************************************************************************
static void add_fund_info(acct_id_p acct_id_temp, 
   char const * const cusip, char const * const desc, char const * const end_value)
{
//06 2023: cusip: [FSCRX], desc: [FIDELITY SMALL CAP DISCOVERY FUND ], end_value: [63709.60]
   // printf("%02u %04u: ", acct_id_temp->month, acct_id_temp->year);
   // printf("cusip: [%s], ", cusip);
   // printf("desc: [%s], ", desc);
   // printf("end_value: [%s]\n", end_value);
   fund_info_p fi_temp ;
   for ( fi_temp = acct_id_temp->fi_top;
         fi_temp != NULL;
         fi_temp = fi_temp->next) {
        
      //  see if we already have an entry for this cusip
      if (strncmp(cusip, fi_temp->cusip, LEN_CUSIP) == 0) {
         // printf("%s: found another entry: ", cusip);
         break ;
      }
   }
   
   //  if fund_info entry does not already exist, create and add it
   if (fi_temp == NULL) {
      // printf("%s: create first entry:  ", cusip);
      fi_temp = new fund_info_t ;
      ZeroMemory((char *) fi_temp, sizeof(fund_info_t));
      strcpy(fi_temp->cusip, cusip);
      strcpy(fi_temp->desc, desc);
      
      //  add fund_info entry
      if (acct_id_temp->fi_top == NULL) {
         acct_id_temp->fi_top = fi_temp ;
      }
      else {
         acct_id_temp->fi_tail->next = fi_temp ;
      }
      acct_id_temp->fi_tail = fi_temp ;
   }
   
   //  add fund_entry struct to fund_info list
   add_fund_entry(fi_temp, acct_id_temp, end_value);
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
   char desc[LEN_DESC+1] = "" ;
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
         src = strccpy(src, desc, LEN_DESC);
         break ;
      case 2:  //  quantity (skip)
         src = strccpy(src, dest, 20);
         break ;
      case 3:  //  price (skip)
         src = strccpy(src, dest, 20);
         break ;
      case 4:  //  beginning value (skip)
         src = strccpy(src, dest, 20);
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
      add_fund_info(acct_id_temp, cusip, desc, end_value);
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
   // printf("\n%s\n", fpath);
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
            // printf("%s\n", inpstr);
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

//**********************************************************************************
void dump_account_lists(void)
{
   acct_id_p acct_id_temp ;
   fund_info_p fi_temp ;
   fund_entry_p fe_temp ;
   fund_entry_p fe_prev ;
   printf("month/             percent  percent\n");
   printf("year     value     of prev  change \n");
   printf("======= =========  =======  =======\n");
   
   for (acct_id_temp = acct_id_top; acct_id_temp != NULL; acct_id_temp = acct_id_temp->next) {
      printf("\nAccount %s [%s]\n", acct_id_temp->acct_num, acct_id_temp->fund_name);
      for (fi_temp = acct_id_temp->fi_top;
           fi_temp != NULL;
           fi_temp = fi_temp->next) {
         printf("   fund %s [%s]\n", fi_temp->cusip, fi_temp->desc) ;
         
         fe_prev = NULL ;
         for (fe_temp = fi_temp->fe_top; fe_temp != NULL; fe_temp = fe_temp->next) {
            if (fe_prev == NULL) {
               printf("%02u/%04u %9.2f\n", fe_temp->month, fe_temp->year, fe_temp->end_value);
            }
            else {
               double value_chg_pct = (fe_temp->end_value / fe_prev->end_value) * 100.0 ;
               double value_diff_pct = ((fe_temp->end_value - fe_prev->end_value) / fe_temp->end_value) * 100.0 ;
               printf("%02u/%04u %9.2f  %.2f    %.2f\n", 
                  fe_temp->month, fe_temp->year, fe_temp->end_value, value_chg_pct, value_diff_pct);
            }
            fe_prev = fe_temp ;
         }
      }
      
   }
   
}
