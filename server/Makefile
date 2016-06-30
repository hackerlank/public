TARGET = server
TARGET := $(addprefix ./,$(TARGET))

CXX= g++ -std=c++11 -g -O0 -Wall -Wno-unknown-pragmas -Wno-reorder  -Wno-unused-value -D_CRT_SECURE_NO_DEPRECATE -I./ -I$(DEPENDENCIES)/include -I../libkeye/ -L$(DEPENDENCIES)/libs/ -L../libkeye/ -L/usr/local/lib/ -lkeye -lpthread -lmysqlclient -lcurses
#CFLAGS= -g -O0 -Wall -D_CRT_SECURE_NO_DEPRECATE -fpermissive -Wno-parentheses -Wno-sign-compare -Wno-unused-variable -Wpointer-sign -Wparentheses -I./ -I$(DEPENDENCIES)include -I../libkeye/
#LFLAGS += -lkeye -lcurses -lboost_system -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono

FILES+= $(wildcard *.cpp)
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
	$(CXX) $(CFLAGS) $(LFLAGS) $(OBJS) -o $@
