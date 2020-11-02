#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.1                         #
#------------------------------------------------------------------------------#

WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I./CommonLib
CFLAGS = -std=c++11 -Wall `pkg-config libglade-2.0 --cflags`
RESINC = 
LIBDIR = 
LIB = -lPocoFoundation -lglade-2.0
LDFLAGS = `pkg-config gtk+-2.0 --libs` -export-dynamic

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -march=athlon-fx -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR) -L./CommonLib/bin/Debug
LIB_DEBUG = $(LIB) -llibCommonLib
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = TimeKeeper/TimeKeeper

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR) -L./CommonLib/bin/Release
LIB_RELEASE = $(LIB) -llibCommonLib
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = TimeKeeper/TimeKeeper

OBJ_DEBUG = $(OBJDIR_DEBUG)/TimeKeeper.o $(OBJDIR_DEBUG)/TimePeriod.o $(OBJDIR_DEBUG)/TkStuff.o $(OBJDIR_DEBUG)/main.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/TimeKeeper.o $(OBJDIR_RELEASE)/TimePeriod.o $(OBJDIR_RELEASE)/TkStuff.o $(OBJDIR_RELEASE)/main.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d TimeKeeper || mkdir -p TimeKeeper
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/TimeKeeper.o: TimeKeeper.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c TimeKeeper.cpp -o $(OBJDIR_DEBUG)/TimeKeeper.o

$(OBJDIR_DEBUG)/TimePeriod.o: TimePeriod.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c TimePeriod.cpp -o $(OBJDIR_DEBUG)/TimePeriod.o

$(OBJDIR_DEBUG)/TkStuff.o: TkStuff.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c TkStuff.cpp -o $(OBJDIR_DEBUG)/TkStuff.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf TimeKeeper
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d TimeKeeper || mkdir -p TimeKeeper
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/TimeKeeper.o: TimeKeeper.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c TimeKeeper.cpp -o $(OBJDIR_RELEASE)/TimeKeeper.o

$(OBJDIR_RELEASE)/TimePeriod.o: TimePeriod.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c TimePeriod.cpp -o $(OBJDIR_RELEASE)/TimePeriod.o

$(OBJDIR_RELEASE)/TkStuff.o: TkStuff.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c TkStuff.cpp -o $(OBJDIR_RELEASE)/TkStuff.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf TimeKeeper
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

