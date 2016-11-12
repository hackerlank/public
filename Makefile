default all:
	make -C libkeye
	make -C libvic
	make -C Node
	make -C protocol

clean:
	make -C libkeye clean
	make -C libvic clean
	make -C Node clean
	make -C protocol clean

echo:
	make -C libkeye echo
	make -C libvic echo
	make -C Node echo
	make -C protocol echo

.PHONY: default all
