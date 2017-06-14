TARGET = sample
OBJS = sample.o \
       TM1637.o \
       LiquidCrystal_I2C.o \
       DS3231.o

CC = gcc
CFLAGS=-I.
COMPILE = $(CC) -c

LIBS = -lwiringPi -lwiringPiDev -lpthread -lncurses

$(TARGET):$(OBJS)
	$(CC) -lm -o $@ $(OBJS) $(CFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)
	echo "clean!!"
