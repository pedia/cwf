##
##                  INTEL CORPORATION PROPRIETARY INFORMATION
##     This software is supplied under the terms of a license agreement or
##     nondisclosure agreement with Intel Corporation and may not be copied
##     or disclosed except in accordance with the terms of that agreement.
##          Copyright(c) 2007-2010 Intel Corporation. All Rights Reserved.
##

##############################################################################
#  Makefile
#    used to build IJG sample
#

####### Compiler, tools and options

LibName = jpeg.ipp

.SUFFIXES:
.SUFFIXES: .c .o

LIBPREFIX := lib
SOPOSTFIX := so
ARPOSTFIX := a
LibSrcDir := ./_src
SmpSrcDir := ./samples/utils
TstSrcDir := ./samples/ijgtiming
ObjDir    := ./_obj/$(ARCH)_$(COMP)
BinDir    := ./bin/$(ARCH)_$(COMP)

INCDIR1   := ./include
INCDIR2   := ./_include
INCDIR3   := $(IPPROOT)/include
CFLAGS    := -O2 -fpic
CINCLUDE  := -I $(INCDIR1) -I $(INCDIR2) -I $(INCDIR3)
CDEFINES  := -Dlinux -Dlinux64

LDFLAGS := -L $(BinDir) -L $(IPPROOT)/lib/$(ARCHL)

ifeq ($(LINKAGE),static)
IPPLIBS := -lippj_l -lippi_l -lipps_l -lippcore_l
else
IPPLIBS := -lippj -lippi -lipps -lippcore
endif

LibObj = \
  $(ObjDir)/jcapimin.o $(ObjDir)/jcmarker.o $(ObjDir)/jdapimin.o \
  $(ObjDir)/jdinput.o  $(ObjDir)/jdtrans.o  $(ObjDir)/jidctred.o \
  $(ObjDir)/jcapistd.o $(ObjDir)/jcmaster.o $(ObjDir)/jdapistd.o \
  $(ObjDir)/jdmainct.o $(ObjDir)/jerror.o   $(ObjDir)/jmemmgr.o  \
  $(ObjDir)/jccoefct.o $(ObjDir)/jcomapi.o  $(ObjDir)/jdatadst.o \
  $(ObjDir)/jdmarker.o $(ObjDir)/jfdctflt.o $(ObjDir)/jmemnobs.o \
  $(ObjDir)/jccolor.o  $(ObjDir)/jcparam.o  $(ObjDir)/jdatasrc.o \
  $(ObjDir)/jdmaster.o $(ObjDir)/jfdctfst.o $(ObjDir)/jquant1.o  \
  $(ObjDir)/jcdctmgr.o $(ObjDir)/jcphuff.o  $(ObjDir)/jdcoefct.o \
  $(ObjDir)/jdmerge.o  $(ObjDir)/jfdctint.o $(ObjDir)/jquant2.o  \
  $(ObjDir)/jchuff.o   $(ObjDir)/jcprepct.o $(ObjDir)/jdcolor.o  \
  $(ObjDir)/jdphuff.o  $(ObjDir)/jidctflt.o $(ObjDir)/jutils.o   \
  $(ObjDir)/jcinit.o   $(ObjDir)/jcsample.o $(ObjDir)/jddctmgr.o \
  $(ObjDir)/jdpostct.o $(ObjDir)/jidctfst.o $(ObjDir)/jcmainct.o \
  $(ObjDir)/jctrans.o  $(ObjDir)/jdhuff.o   $(ObjDir)/jdsample.o \
  $(ObjDir)/jidctint.o


CjpegObj = \
  $(ObjDir)/cdjpeg.o   $(ObjDir)/cjpeg.o    $(ObjDir)/rdbmp.o   \
  $(ObjDir)/rdcolmap.o $(ObjDir)/rdgif.o    $(ObjDir)/rdppm.o   \
  $(ObjDir)/rdrle.o    $(ObjDir)/rdswitch.o $(ObjDir)/rdtarga.o \
  $(ObjDir)/wrbmp.o    $(ObjDir)/wrgif.o    $(ObjDir)/wrppm.o   \
  $(ObjDir)/wrrle.o

DjpegObj = \
  $(ObjDir)/cdjpeg.o   $(ObjDir)/djpeg.o    $(ObjDir)/rdbmp.o    \
  $(ObjDir)/rdcolmap.o $(ObjDir)/rdgif.o    $(ObjDir)/rdppm.o    \
  $(ObjDir)/rdrle.o    $(ObjDir)/rdswitch.o $(ObjDir)/rdtarga.o  \
  $(ObjDir)/wrbmp.o    $(ObjDir)/wrgif.o    $(ObjDir)/wrppm.o    \
  $(ObjDir)/wrrle.o    $(ObjDir)/wrtarga.o

RdjpgcomObj = \
  $(ObjDir)/rdjpgcom.o

WrjpgcomObj = \
  $(ObjDir)/wrjpgcom.o

JpegtranObj = \
  $(ObjDir)/cdjpeg.o $(ObjDir)/jpegtran.o $(ObjDir)/rdswitch.o \
  $(ObjDir)/transupp.o


IjgtimingObj = \
  $(ObjDir)/ijg_timing.o


VPATH := $(LibSrcDir) $(SmpSrcDir) $(TstSrcDir)

all : clean prepare build

clean :
	rm -rf $(ObjDir)
	rm -rf $(BinDir)

prepare :
	mkdir -p $(ObjDir)
	mkdir -p $(BinDir)

build :  $(BinDir)/$(LIBPREFIX)$(LibName).$(ARPOSTFIX) \
         $(BinDir)/$(LIBPREFIX)$(LibName).$(SOPOSTFIX) \
         $(BinDir)/cjpeg \
         $(BinDir)/djpeg \
         $(BinDir)/rdjpgcom \
         $(BinDir)/wrjpgcom \
         $(BinDir)/jpegtran \
         $(BinDir)/ijg_timing


$(ObjDir)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CINCLUDE) $(CDEFINES) $^ -o $@


# build libjpeg static library
$(BinDir)/$(LIBPREFIX)$(LibName).$(ARPOSTFIX) : $(LibObj)
	$(AR) cr $@ $(LibObj)

$(BinDir)/$(LIBPREFIX)$(LibName).$(SOPOSTFIX) : $(LibObj)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(IPPLIBS)


#############################################################################################################
# build jpeg utils

ADD_LIB=$(IPPLIBS)

ifneq ($(LINKAGE),static)
ADD_LIB += -ldl $(LIBPTHREAD)
endif

$(BinDir)/cjpeg : $(CjpegObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

$(BinDir)/djpeg : $(DjpegObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

$(BinDir)/rdjpgcom : $(RdjpgcomObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

$(BinDir)/wrjpgcom : $(WrjpgcomObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

$(BinDir)/jpegtran : $(JpegtranObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

$(BinDir)/ijg_timing : $(IjgtimingObj)
	$(CXX) -o $@ $^ $(LDFLAGS) -l$(LibName) $(ADD_LIB)

