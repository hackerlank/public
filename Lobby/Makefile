# make echo first to ensure compile environment is fine

TARGET = server
TARGET := $(addprefix ./,$(TARGET))

CXX= g++ -g -O0 -Wall -Wl,--no-as-needed -Wno-unknown-pragmas -Wno-reorder -Wno-unused-value -I./ -I../libkeye/ -I../libvic/ -I$(DEPENDENCIES)/include -L../libkeye/ -L../libvic/ -L$(DEPENDENCIES)/libs -Wl,-rpath-link=../libkeye/:../libvic/:$(DEPENDENCIES)/libs -Wl,-rpath=sdks:../libkeye/:../libvic/:$(DEPENDENCIES)/libs -lkeye -lvic -lprotobuf-lite -lhiredis -lstdc++ -lm
# -lcurses

FILES= $(wildcard *.cpp)
PROTO_FILES= $(wildcard protocol/*.cc)
OBJS=$(patsubst %.cpp,%.o,$(FILES)) $(patsubst %.cc,%.o,$(PROTO_FILES))

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
	@echo FILES=$(FILES) $(PROTO_FILES)
	@echo OBJS=$(OBJS)

.PHONY: default all clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CFLAGS) $(LFLAGS)
