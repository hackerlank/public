default all:
	make -C libkeye
	make -C libvic
	make -C server
	make -C client

clean:
	make -C libkeye clean
	make -C libvic clean
	make -C server clean
	make -C client clean

echo:
	make -C libkeye echo
	make -C libvic echo
	make -C server echo
	make -C client echo

.PHONY: default all
