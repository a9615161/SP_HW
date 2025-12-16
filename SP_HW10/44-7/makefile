CC = gcc
CFLAGS = -Wall -O2

# 執行檔名稱
TARGET = fifo_test

# 定義 FIFO 的暫存路徑
FIFO_NAME = /tmp/test_fifo_demo

# 虛擬目標
.PHONY: all clean test-all test-open-nb test-io-nb test-switch

all: $(TARGET)

$(TARGET): fifo_test.c
	$(CC) $(CFLAGS) -o $(TARGET) fifo_test.c

clean:
	rm -f $(TARGET) $(FIFO_NAME) *.pid output.tmp

# -----------------------------------------------------------
# 自動化測試腳本
# -----------------------------------------------------------

# 輔助：重置環境
reset:
	@pkill -x $(TARGET) || true
	@rm -f $(FIFO_NAME) *.pid
	@sleep 0.5

# 執行所有測試
test-all: test-open-nb test-io-nb test-switch

# -----------------------------------------------------------
# 測試 1: 驗證 Non-blocking Open (Table 44-1)
# -----------------------------------------------------------
test-open-nb: all reset
	@echo "\n====== [Test 1] Verifying Non-blocking Open ======"
	
	@echo ">> Case A: Writer opens with O_NONBLOCK (No Reader)"
	@echo "   Expectation: Should FAIL with ENXIO immediately."
	@if ./$(TARGET) -f $(FIFO_NAME) -w -n > /dev/null 2>&1; then \
		echo "   [FAIL] Open succeeded unexpectedly!"; \
	else \
		echo "   [PASS] Open failed as expected (ENXIO)."; \
	fi

	@echo "\n>> Case B: Reader opens with O_NONBLOCK (No Writer)"
	@echo "   Expectation: Should SUCCEED immediately."
	@if ./$(TARGET) -f $(FIFO_NAME) -r -n > /dev/null 2>&1; then \
		echo "   [PASS] Open succeeded."; \
	else \
		echo "   [FAIL] Open failed!"; \
	fi

# -----------------------------------------------------------
# 測試 2: 驗證 Non-blocking I/O (Table 44-2 & 44-3)
# -----------------------------------------------------------
test-io-nb: all reset
	@echo "\n====== [Test 2] Verifying Non-blocking I/O ======"
	
	@echo ">> Case C: Non-blocking READ (Waiting for data)"
	@echo "   Action: Reader (O_NONBLOCK) polls while Writer sleeps 2s."
	@echo "   Expectation: Reader prints EAGAIN multiple times, then reads data."
	@# 1. 啟動慢速 Writer (背景)
	@./$(TARGET) -f $(FIFO_NAME) -w -s 2 > /dev/null & echo $$! > writer.pid
	@# 2. 啟動 Reader (限時 4 秒)
	@timeout 4s ./$(TARGET) -f $(FIFO_NAME) -r -n || true
	@kill `cat writer.pid` 2>/dev/null || true
	@rm writer.pid

	@echo "\n>> Case D: Non-blocking WRITE (Pipe Full)"
	@echo "   Action: Writer (O_NONBLOCK) writes fast, Reader reads slow."
	@echo "   Expectation: Writer eventually prints EAGAIN (Pipe Full)."
	@rm -f $(FIFO_NAME)
	@# 1. 啟動慢速 Reader (背景)
	@./$(TARGET) -f $(FIFO_NAME) -r -s 1 > /dev/null & echo $$! > reader.pid
	@# 2. 啟動 Writer (限時 3 秒)
	@timeout 3s ./$(TARGET) -f $(FIFO_NAME) -w -n || true
	@kill `cat reader.pid` 2>/dev/null || true
	@rm reader.pid

# -----------------------------------------------------------
# 測試 3: 驗證模式切換 (O_NONBLOCK -> Blocking)
# -----------------------------------------------------------
test-switch: all reset
	@echo "\n====== [Test 3] Verifying Mode Switch (-B flag) ======"
	@echo "   Action: Open with O_NONBLOCK, switch to Blocking, then Read."
	@echo "   Note: We start a background Writer to keep the pipe open (preventing EOF)."
	@echo "   Expectation: Reader should BLOCK (hang) waiting for data."
	
	@# 1. 啟動一個睡眠中的 Writer，確保 Pipe 的寫入端是開啟的
	@./$(TARGET) -f $(FIFO_NAME) -w -s 10 > /dev/null & echo $$! > writer.pid
	@sleep 0.5
	
	@# 2. 啟動 Reader (限時 2 秒)
	@#    因為 Writer 不送資料，Blocking Reader 應該會卡住直到 timeout
	@#    如果它沒卡住 (回傳 EAGAIN 或 EOF)，輸出行數會很多
	@timeout 2s ./$(TARGET) -f $(FIFO_NAME) -r -n -B > output.tmp || true
	
	@# 3. 檢查輸出
	@if [ `wc -l < output.tmp` -lt 5 ]; then \
		echo "   [PASS] Low output activity detected (Reader blocked properly)."; \
	else \
		echo "   [FAIL] High output activity detected (Reader did NOT block)."; \
		head -n 5 output.tmp; \
	fi
	
	@kill `cat writer.pid` 2>/dev/null || true
	@rm -f output.tmp writer.pid
	@echo "\n====== All Tests Completed ======"