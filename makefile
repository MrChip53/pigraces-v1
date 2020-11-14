TARGET = PigRaces
OBJS = multimp3player.o framebuffer.o graphics.o main.o RDLib.o
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

LIBS = -lpspgu -lpng -lz -lm -lmad -lpspaudiolib -lpspaudio -lpsppower -lcurl
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Pig Races
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 