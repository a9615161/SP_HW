CC = gcc
CFLAGS = -Wall -O2

TARGET = MyTailF
GEN = gen_test
TESTFILE = testdata.txt

all: $(TARGET) $(GEN)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

$(GEN): gen_test.c
	$(CC) $(CFLAGS) -o $(GEN) gen_test.c

# 產生 100000 行測試資料
gen: $(GEN)
	./$(GEN) $(TESTFILE) 100000

# 測試 tail
test: $(TARGET) gen
	@echo "=== Last 10 lines (default) ==="
	./$(TARGET) $(TESTFILE) | tail -n 10
	@echo
	@echo "=== Last 10 lines (-n 10) ==="
	./$(TARGET) -n 10 $(TESTFILE) | tail -n 10

clean:
	rm -f $(TARGET) $(GEN) $(TESTFILE)
