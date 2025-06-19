# makefile for media_list app
SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
TOOLS=d:\tdm32\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
CxxFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -s -O3 -c
CxxFLAGS = -Wall -s -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
CFLAGS += -Wno-format-overflow
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
CxxFLAGS += -DUSE_64BIT
endif
CFLAGS += -DLEGACY_QUALIFY

LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
IFLAGS += -Ider_libs

# This is required for *some* versions of makedepend
IFLAGS += -DNOMAKEDEPEND

CPPSRC=FidelityCalcs.cpp parse_report.cpp \
der_libs/common_funcs.cpp \
der_libs/qualify_orig.cpp

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

LIBS=-lshlwapi

#**************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) -c $< -o $@

ifeq ($(USE_64BIT),NO)
BIN = FidelityCalcs.exe
else
BIN = FidelityCalcs64.exe
endif

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f FidelityCalcs.zip
	zip FidelityCalcs.zip $(BIN) Readme.md

wc:
	wc -l *.cpp

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(IFLAGS) $(CSRC) $(CPPSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

FidelityCalcs.o: der_libs/common.h FidelityCalcs.h der_libs/qualify.h
parse_report.o: der_libs/common.h FidelityCalcs.h
der_libs/qualify.o: der_libs/common.h der_libs/conio_min.h der_libs/qualify.h
