BACKEND = gtk
# BACKEND := sdl

all:
clean:

.PHONY: all clean

## server

COMMON_CFLAGS = -g -Wuninitialized -O2

sdl_CFLAGS = `pkg-config sdl --cflags`
sdl_LDFLAGS = `pkg-config sdl --libs`
gtk_CFLAGS = `pkg-config gtk+-3.0 --cflags`
gtk_LDFLAGS = `pkg-config gtk+-3.0 --libs`

CFLAGS = $(COMMON_CFLAGS) `pkg-config cairo --cflags` $($(BACKEND)_CFLAGS) -Icommon
LDFLAGS = -g `pkg-config cairo --libs` $($(BACKEND)_LDFLAGS) -lm

MAIN_OBJ = server/main.o server/toolkit_$(BACKEND).o server/field.o common/net_utils.o server/net_commands.o server/net_core.o server/robotserver.o

robotserver: $(MAIN_OBJ)
	$(CC) -o robotserver $(MAIN_OBJ) $(LDFLAGS)
server/field.o: server/field.c server/toolkit.h server/field.h
server/testlogic.o: server/testlogic.c server/robotserver.h
server/robotserver.o: server/robotserver.c server/robotserver.h
server/toolkit_$(BACKEND).o: server/toolkit_$(BACKEND).c server/toolkit.h
server/net_core.o: server/net_core.c server/robotserver.h common/net_utils.h server/net_defines.h server/field.h
server/net_commands.o: server/net_commands.c server/net_defines.h server/robotserver.h
common/net_utils.o: common/net_utils.c common/net_utils.h

## robots

ROBOTS = counter rabbit rook sniper spot
LIBROBOTS_OBJS = clients/robots.o common/net_utils.o

all-robots: $(ROBOTS)

clients/robots.o: clients/robots.c
robots.a: $(LIBROBOTS_OBJS)
	ar cru $@ $(LIBROBOTS_OBJS)

counter: clients/counter.c clients/robots.h robots.a
	$(CC) $(COMMON_CFLAGS) -o $@ $< -lm robots.a

rabbit: clients/rabbit.c clients/robots.h robots.a
	$(CC) $(COMMON_CFLAGS) -o $@ $< -lm robots.a

rook: clients/rook.c clients/robots.h robots.a
	$(CC) $(COMMON_CFLAGS) -o $@ $< -lm robots.a

sniper: clients/sniper.c clients/robots.h robots.a
	$(CC) $(COMMON_CFLAGS) -o $@ $< -lm robots.a

spot: clients/spot.c clients/robots.h robots.a
	$(CC) $(COMMON_CFLAGS) -o $@ $< -lm robots.a


## overall rules

all: robotserver $(ROBOTS)

clean:
	rm -f $(ROBOTS) $(LIBROBOTS_OBJS) robots.a robotserver $(MAIN_OBJ)
