# Makefile for compiling an ev3 project with the following folder structure:
#
#	this_folder/
# 		Makefile (this file)
#		ev3dev-c/
#		libraries/
#			libbluetooth.a
#			libev3dev-c.a
#		include/
# 			bt_client.h
#			messages.h
#			movement.h
#			sensors.h
#	 	source/
#			bt_client.c
#			messages.c
#			movement.c
#			sensors.c
#			main.c
#
# The main executable will be located in the same directory as you ran 
# "Make" from. To add new .c files, simply add them to the OBJS variable.

CC 			= arm-linux-gnueabi-gcc
CFLAGS 		= -O2 -g -std=gnu99 -W -Wall -Wno-comment
INCLUDES 	= -I./ev3dev-c/source/ev3
LDFLAGS 	= -L./libraries -lrt -lm -lpthread -lev3dev-c
BUILD_DIR 	= ./build
SOURCE_DIR 	= ./source

OBJS = \
	$(BUILD_DIR)/touch.o \
	$(BUILD_DIR)/gyro.o \
	$(BUILD_DIR)/color.o \
	$(BUILD_DIR)/sonar.o \
	$(BUILD_DIR)/motors.o \
	$(BUILD_DIR)/main.o

all: main

main: ${OBJS}
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o main

$(OBJS): $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c $(SOURCE_DIR)/$*.c -o $(BUILD_DIR)/$*.o

clean:
	rm -f $(BUILD_DIR)/*.o
	rm ./main