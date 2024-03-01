
SHELL = /bin/sh
.PHONY: build
build: 
	mkdir build 
	cd build && cmake ..
	cd build && make
	ln -s ./build/Client/Tcp_Client ./Tcp_Client
	ln -s ./build/Server/Tcp_Server ./Tcp_Server

.PHONY: rebuild
rebuild:
	cd build && make
	ln -s ./build/Client/Tcp_Client ./Tcp_Client
	ln -s ./build/Server/Tcp_Server ./Tcp_Server

.PHONY: remove
remove:
	rm -rf ./build
	rm Tcp_Client
	rm Tcp_Server