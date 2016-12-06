default all:
	make -C protocol
	make -C libkeye
	make -C libvic
	make -C libimmortal
	make -C libvic
	make -C Lobby
	make -C Node
	make -C Charge

clean:
	make -C protocol clean
	make -C libkeye clean
	make -C libvic clean
	make -C libimmortal clean
	make -C libvic clean
	make -C Lobby clean
	make -C Node clean
	make -C Charge clean

echo:
	make -C libkeye echo

.PHONY: default all
