default all:
	make -C libkeye
	make -C server

clean:
	make -C libkeye clean
	make -C server clean

.PHONY: default all
