all: gen_sparse cp_sparse mytee

gen_sparse: gen_sparse.c
	gcc -O2 -Wall -o gen_sparse gen_sparse.c

cp_sparse: cp_sparse.c
	gcc -O2 -Wall -o cp_sparse cp_sparse.c

mytee: mytee.c
	gcc -O2 -Wall -o mytee mytee.c

run: gen_sparse cp_sparse mytee
	./gen_sparse
	@echo "\n=== ls -lh ==="
	ls -lh *.bin
	@echo "\n=== du -h ==="
	du -h *.bin
	@echo "\n=== 測試 cp_sparse ==="
	./cp_sparse no_hole.bin copy_no_hole.bin
	./cp_sparse hole_head.bin copy_hole_head.bin
	./cp_sparse hole_tail.bin copy_hole_tail.bin
	./cp_sparse multi_hole.bin copy_multi_hole.bin
	ls -lh copy_*.bin
	du -h copy_*.bin
	@echo "\n=== 測試 mytee ==="
	echo "Hello, tee!" | ./mytee tee_test.txt
	cat tee_test.txt
	echo "Append line" | ./mytee -a tee_test.txt
	cat tee_test.txt

clean:
	rm -f gen_sparse cp_sparse mytee *.bin *.txt
