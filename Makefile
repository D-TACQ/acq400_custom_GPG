all: bin/stl2gpg

bin/stl2gpg: stl2gpg.c
	${CC} stl2gpg.c -o bin/stl2gpg

stl2gpg.x86: stl2gpg.c
	cc stl2gpg.c -o stl2gpg.x86


	
	
