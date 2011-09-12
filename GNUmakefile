### ------------------------------------------------------
### GNUmakefile for Amrvis
### ------------------------------------------------------
PRECISION = FLOAT
PRECISION = DOUBLE
PROFILE   = TRUE
PROFILE   = FALSE

COMP      = g++
FCOMP     = gfortran

DEBUG     = TRUE
DEBUG     = FALSE

DIM       = 2
DIM       = 3

USE_ARRAYVIEW = TRUE
USE_ARRAYVIEW = FALSE

USE_MPI=TRUE
USE_MPI=FALSE

USE_VOLRENDER = FALSE
USE_VOLRENDER = TRUE

USE_PARALLELVOLRENDER = TRUE
USE_PARALLELVOLRENDER = FALSE

BOXLIB_HOME = ../BoxLib

include $(BOXLIB_HOME)/Tools/C_mk/Make.defs

EBASE = amrvis
HERE = .

INCLUDE_LOCATIONS += $(HERE)
INCLUDE_LOCATIONS += $(BOXLIB_HOME)/Src/C_BaseLib

DEFINES += -DBL_PARALLEL_IO

############################################### x includes and libraries
ifeq ($(MACHINE), OSF1)
  LIBRARIES += -lXm -lXt -lX11
endif

ifeq ($(WHICHLINUX), ATLAS)
  LIBRARY_LOCATIONS += /usr/X11R6/lib64
endif 

ifeq ($(MACHINE), Linux)
  ifeq ($(WHICHLINUX), INTREPID)
  # NOTE: on intrepid, use g++ and gfortran
  LIBRARY_LOCATIONS += /usr/X11/lib
  INCLUDE_LOCATIONS += /usr/X11/include
  LIBRARY_LOCATIONS += /usr/lib
  else
  LIBRARY_LOCATIONS += /usr/lib64
  INCLUDE_LOCATIONS += /usr/include/Xm
  INCLUDE_LOCATIONS += /usr/include/
  endif

  LIBRARIES += -lXm -lXp -lXt -lXext -lSM -lICE -lXpm -lX11
  # LIBRARIES += -LlibXm.so.2.1
endif

ifeq ($(MACHINE), AIX)
  INCLUDE_LOCATIONS += /usr/include/X11
  INCLUDE_LOCATIONS += /usr/include/Xm
  #INCLUDE_LOCATIONS += /usr/include/X11/Xaw
  LIBRARIES += -lXm -lXt -lX11
  DEFINES += -D_ALL_SOURCE
endif

ifeq ($(MACHINE), CYGWIN_NT)
  #INCLUDE_LOCATIONS += /cygdrive/c/usr/X11R6/include
  #LIBRARY_LOCATIONS += /cygdrive/c/usr/X11R6/lib
  INCLUDE_LOCATIONS += /usr/X11R6/include
  LIBRARY_LOCATIONS += /usr/X11R6/lib
  #CXXFLAGS += -fpermissive
  #CXXFLAGS += -x c++
#  LDFLAGS += -noinhibit-exec
  LIBRARIES += -lXm -lXt -lSM -lICE -lXpm -lX11
endif

# JFG: this line is needed on hive
# LIBRARY_LOCATIONS += /usr/X11R6/lib64

############################################### arrayview
ifeq (USE_ARRAYVIEW, TRUE)
  DEFINES += -DBL_USE_ARRAYVIEW
  ARRAYVIEWDIR = .
  INCLUDE_LOCATIONS += $(ARRAYVIEWDIR)
  #LIBRARY_LOCATIONS += $(ARRAYVIEWDIR)
  #LIBRARIES += -larrayview$(DIM)d.$(machineSuffix)
endif


############################################### volume rendering
ifeq ($(DIM),3)
  ifeq ($(MACHINE), T3E)
    USE_VOLRENDER = FALSE
  endif
  ifeq ($(MACHINE), AIX)
#   USE_VOLRENDER = FALSE
  endif
  ifeq ($(USE_VOLRENDER), TRUE)
    DEFINES += -DBL_VOLUMERENDER
    VOLPACKDIR = ../volpack
    INCLUDE_LOCATIONS += $(VOLPACKDIR)
    LIBRARY_LOCATIONS += $(VOLPACKDIR)
    LIBRARIES += -lvolpack
    #DEFINES += -DVOLUMEBOXES
  endif
endif

############################################### parallel volume rendering
ifeq ($(DIM),3)
  ifeq ($(USE_PARALLELVOLRENDER), TRUE)
    DEFINES += -DBL_PARALLELVOLUMERENDER
  endif
endif

############################################### other defines
#DEFINES += -DSCROLLBARERROR
#DEFINES += -DFIXDENORMALS

############################################### float fix
# if we are using float override FOPTF which sets -real_size 64
ifeq ($(PRECISION), FLOAT)
  ifeq ($(MACHINE), OSF1)
    FDEBF += -C 
    FDEBF += -fpe2
    #FDEBF += -fpe0
    FOPTF  = -fast -O5 -tune ev5
  endif
endif

include $(HERE)/Make.package
include $(BOXLIB_HOME)/Src/C_BaseLib/Make.package

vpath %.cpp $(HERE) $(BOXLIB_HOME)/Src/C_BaseLib
vpath %.H   $(HERE) $(BOXLIB_HOME)/Src/C_BaseLib
vpath %.F   $(HERE) $(BOXLIB_HOME)/Src/C_BaseLib
vpath %.f   $(HERE) $(BOXLIB_HOME)/Src/C_BaseLib
vpath %.a   $(LIBRARY_LOCATIONS)

all: $(executable)

include $(BOXLIB_HOME)/Tools/C_mk/Make.rules
### ------------------------------------------------------
### ------------------------------------------------------
