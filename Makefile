ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

TARGET = DinguxCommander

CC=$(CROSS_COMPILE)g++
DPLATFORM=PLATFORM_$(shell echo $(PLATFORM) | tr 'a-z' 'A-Z')

RESDIR:=res

SRCS=$(wildcard src/*.cpp)
OBJS=$(patsubst %cpp,%o,$(SRCS))

INCLUDE = -I$(PREFIX)/include/SDL2
LIB = -lSDL2 -lSDL2_image -lSDL2_ttf 

all:$(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -D$(DPLATFORM) $(LIB)

%.o:%.cpp
	$(CC) -DRESDIR="\"$(RESDIR)\"" -D$(DPLATFORM)  -c $< -o $@  $(INCLUDE) 

clean:
	rm $(OBJS) $(TARGET) -f

