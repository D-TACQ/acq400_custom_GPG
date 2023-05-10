all: 
	echo Hello

clean:
	rm bin/stl2gpg stl2gpg.x86

CFLAGS += -I../../include

bin/stl2gpg: stl2gpg.c
	${CC} ${CFLAGS} stl2gpg.c -o bin/stl2gpg -L../../lib/linux-arm -lpopt


stl2gpg.x86: stl2gpg.c
	cc stl2gpg.c -o stl2gpg.x86


	
	
