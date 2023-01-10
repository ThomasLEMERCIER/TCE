release:
	gcc -O3 tce.c -o tce.exe
debug:
	gcc -O3 -DDEBUG tce.c -o tce_d.exe; ./tce_d.exe
all:
	gcc -O3 tce.c -o tce.exe
	gcc -O3 -DDEBUG tce.c -o tce_d.exe