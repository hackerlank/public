# make echo first to ensure compile environment is fine

TARGET = server
TARGET := $(addprefix ./,$(TARGET))

CXX= g++ -std=c++11 -g -O0 -Wall -Wno-unknown-pragmas -Wno-reorder -Wno-unused-value -I./ -I../libkeye/ -L$(DEPENDENCIES)/libs -L../libkeye/ -Wl,-rpath-link=$(DEPENDENCIES)/libs -Wl,-rpath=$(DEPENDENCIES)/libs -lkeye -lcurses

FILES= $(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(FILES))

default all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS)
	find ./ -name *.o -exec rm -f {} \;

echo:
	$(E)
	@echo TARGET=$(TARGET)
	@echo CXX=$(CXX)
	@echo CFLAGS=$(CFLAGS)
	@echo LFLAGS=$(LFLAGS)
	$(E)
	@echo FILES=$(FILES)
	@echo OBJS=$(OBJS)

.PHONY: default all clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CFLAGS) $(LFLAGS)
