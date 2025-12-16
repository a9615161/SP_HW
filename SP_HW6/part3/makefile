all:convert lookup
	./convert dict mydict
	./lookup mydict 
convert :convert.c
	gcc convert.c -o convert

lookup : lookup1.c main.c
	gcc lookup1.c main.c -o lookup

clean :
	rm -f convert lookup mydict