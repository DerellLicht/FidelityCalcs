//**********************************************************************************
//  Copyright (c) 2023 Daniel D. Miller                       
//  media_list.cpp - list info about various media files
//                                                                 
//  Written by:   Daniel D. Miller
//  
//**********************************************************************************

//************************************************************
struct ffdata {
   uchar          attrib ;
   FILETIME       ft ;
   ULONGLONG      fsize ;
   char           *filename ;
   uchar          dirflag ;
   uint           sub_path_idx ;
   char           *path_spec ;
   struct ffdata  *next ;
} ;

//  parse_report.cpp
int parse_fidelity_report(ffdata *ftemp);
void dump_account_lists(void);
