TARGET = server
TARGET := $(addprefix ./,$(TARGET))

CXX = g++ -std=c++11 -g -O0 -Wall -fpermissive -Wno-parentheses -Wno-sign-compare -Wno-unused-variable -Wpointer-sign -Wparentheses -I./ -I$(DEPENDENCIES)/include -I../../libkeye/
LFLAGS = -L$(DEPENDENCIES)/libs/ -L/usr/local/lib/ -lpthread -lm -lrt -lkeye -lmysqlclient -lstdc++ -lcurses -lboost_system -lboost_date_time
#LFLAGS += -lkeye -lcurses -lboost_system -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono

FILES+= $(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(FILES))

default all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS)
	find ./ -name *.o -exec rm -f {} \;

echo:
	@echo CFLAGS=$(CFLAGS)

.PHONY: default all clean depend

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(LFLAGS) $(OBJS) -o $@
