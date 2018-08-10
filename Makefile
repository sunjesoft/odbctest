
CC = gcc
CFLAGS = -g -Wall -Wno-switch-enum -D__PRINT_CONN__ -D__PRINT_FETCH__ -D__PRINT_BIND__ -D__PRINT_COMMIT__

DB=goldilocks

JSON=$(PWD)/json-c

COMMON_LIB= -lpthread -ldl -lm -lrt $(JSON)/lib/libjson-c.a

ifeq ($(DB), goldilocks)
CFLAGS += -D__GOLDILOCKS__
DB_HOME = $(GOLDILOCKS_HOME)
STATIC_DA_LIB = -lgoldilocksa
STATIC_CS_LIB = -lgoldilocksc
SHARED_DA_LIB = -lgoldilocksas
SHARED_CS_LIB = -lgoldilockscs-ul64
else
CFLAGS += -D__SUNDB__
DB_HOME=$(SUNDB_HOME)
STATIC_DA_LIB = -lsundba
STATIC_CS_LIB = -lsundbc
SHARED_DA_LIB = -lsundbas
SHARED_CS_LIB = -lsundbcs-ul64
endif

INC=-I. -I$(DB_HOME)/include -I$(JSON)/include
LFLAGS = -L$(DB_HOME)/lib

OBJECTS = main.o log.o property.o odbc.o latency.o
TARGET = testd testcs testds testcss


.SUFFIXES: .c.o

.c.o :
	$(CC) $(CFLAGS) $(INC) -c $<



all: $(TARGET)
testd:	$(OBJECTS)
		$(CC) -o testd $(OBJECTS) $(LFLAGS) $(STATIC_DA_LIB) $(COMMON_LIB)
testcs: $(OBJECTS)
		$(CC) -o testcs $(OBJECTS) $(LFLAGS) $(STATIC_CS_LIB) $(COMMON_LIB)
testds: $(OBJECTS)
		$(CC) -o testds $(OBJECTS) $(LFLAGS) $(SHARED_DA_LIB) $(COMMON_LIB)
testcss: $(OBJECTS)
		$(CC) -o testcss $(OBJECTS) $(LFLAGS) $(SHARED_CS_LIB) $(COMMON_LIB)
clean:
		rm -rf $(OBJECTS) testd testcs testds testcss

