CC=g++

DFLAG=-g -Wall
#DFLAG=-O3

EXEC=./bin/align

OBJS=./obj/main.o \
     ./obj/bmp.o

OPENMP=-fopenmp
#OPENMP= 

LIBS= -lrt
TIME=time

INPUT_DIR=./input

all: $(OBJS)
	$(CC) $(DFLAG) \
	    -o $(EXEC) \
	    $(OBJS) \
	    $(OPENMP) $(LIBS)

./obj/main.o: ./src/main.cpp
	$(CC) $(DFLAG) \
	    -o ./obj/main.o \
	    -c ./src/main.cpp \
	    $(OPENMP)

./obj/color.o: ./src/color.cpp
	$(CC) $(DFLAG) \
	    -o ./obj/color.o \
	    -c ./src/color.cpp

./obj/bmp.o: ./src/bmp.cpp
	$(CC) $(DFLAG) \
	    -o ./obj/bmp.o \
	    -c ./src/bmp.cpp

exec:
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01b.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01b_dark.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01c.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01c_light.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01d.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test01a.bmp $(INPUT_DIR)/test01d_dark.bmp
	$(TIME) $(EXEC) $(INPUT_DIR)/test01a_half.bmp $(INPUT_DIR)/test01c_half.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02b.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02b_dark.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02c.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02c_light.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02d.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a.bmp $(INPUT_DIR)/test02d_dark.bmp
	@#$(TIME) $(EXEC) $(INPUT_DIR)/test02a_half.bmp $(INPUT_DIR)/test02c_half.bmp

clean: 
	rm -f ./obj/*.o
	rm -f $(EXEC)

