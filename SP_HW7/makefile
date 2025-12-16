all : bidir.out mypopen.out
	./bidir.out <lower.txt>upper.txt
	./mypopen.out
bidir.out: bidir.c
	gcc -o bidir.out bidir.c
mypopen.out :mypopen.c
	gcc -o mypopen.out mypopen.c
clean:
	rm -f *.out
	rm -f upper.txt