default all:
	make -C libkeye
	make -C server

clean:
	make -C libkeye clean
	make -C server clean

echo:
	make -C libkeye echo
	make -C server echo

.PHONY: default all
