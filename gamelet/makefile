APP := gamelet
CPP := g++
CFLAGS := -O2
LDFLAGS := -O2 -levent -lpthread

all: $(APP)

OBJS = main.o \
	   client.o \
	   server.o \
	   alog.o \
	   cfgdata.o

$(APP): $(OBJS)
	$(CPP) $(LDFLAGS) -o $@ $^

%.o:%.cpp
	$(CPP) -c $(CFLAGS) $< -o $@
    
clean:
	@rm -f $(OBJS) $(APP)

