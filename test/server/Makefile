TARGET = server
TARGET := $(addprefix bin/,$(TARGET))

CXX = g++ -std=c++11
CFLAGS = -g -O0 -Wall -fpermissive -Wno-parentheses -Wno-sign-compare -Wno-unused-variable -Wpointer-sign -Wparentheses
CFLAGS += -I./ -I../ -I/usr/include/mysql/
LFLAGS = -L/usr/lib/mysql -L/usr/local/lib/ -lpthread -lm -lrt -lmysqlclient -lstdc++
LFLAGS += -lkeye -lcurses -lboost_system -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono

FILES+= $(wildcard *.cpp) \
	$(wildcard game/*.cpp)
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
