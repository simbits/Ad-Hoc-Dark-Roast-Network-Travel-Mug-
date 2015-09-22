#Set this to @ to keep the makefile quiet
SILENCE = @

AVR_TOOLCHAIN = /opt/local/avr
MCU = m1284p
F_CPU = 8000000
F_CLOCK = $(F_CPU)

#---- Outputs ----#
COMPONENT_NAME = mug
TARGET_LIB = \
	lib/lib$(COMPONENT_NAME).a
	
TEST_TARGET = \
	$(COMPONENT_NAME)_tests

#--- Inputs ----#
PROJECT_HOME_DIR = .
CPPUTEST_HOME = tests/CppUTest
CPP_PLATFORM = Gcc

##
#CPPUTEST_WARNINGFLAGS += -pedantic-errors -Wconversion -Wshadow  

CPPUTEST_WARNINGFLAGS += -Wall -Werror -Wswitch-default -Wswitch-enum  
CPPUTEST_CFLAGS = -std=c89 -Wall -Wextra -pedantic -Wstrict-prototypes

#-pedantic -Wall -Werror 
#-pedantic-errors
#-C89

SRC_DIRS = \
	src/*\

TEST_SRC_DIRS = \
	tests \
	mocks
	
INCLUDE_DIRS =\
  $(AVR_TOOLCHAIN)/include/ \
  $(CPPUTEST_HOME)/include/ \
  include\
  mocks
  
include $(CPPUTEST_HOME)/build/MakefileWorker.mk
