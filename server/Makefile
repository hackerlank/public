# make echo first to ensure compile environment is fine

TARGET = server
TARGET := $(addprefix ./,$(TARGET))

CXX= g++ -g -O0 -Wall -Wl,--no-as-needed -Wno-unknown-pragmas -Wno-reorder -Wno-unused-value -I./ -I../libkeye/ -L../libkeye/ -Wl,-rpath-link=../libkeye/:$(DEPENDENCIES)/libs -Wl,-rpath=sdks:../libkeye/:$(DEPENDENCIES)/libs -lkeye -lstdc++ -lm
# -lcurses

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
