APP := gamelet
CPP := g++
CFLAGS := -Wall -O2 -std=c++0x
LDFLAGS := -O2 -levent -lpthread

all: $(APP)

OBJS = main.o \
	   player.o \
	   server.o \
	   alog.o \
	   cfgdata.o \
	   packet.o \
	   cmd.o 

$(APP): $(OBJS)
	$(CPP) -o $@ $^ $(LDFLAGS)

%.o:%.cpp
	$(CPP) -c $(CFLAGS) $< -o $@
    
clean:
	@rm -f $(OBJS) $(APP)

