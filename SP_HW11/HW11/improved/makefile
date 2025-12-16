# --- 變數定義 ---
CC = gcc
# 加入 POSIX 宏定義到 CFLAGS，確保函式和結構體正確宣告 (如 sigaction, usleep)
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=199309L
LDFLAGS = -lrt -lpthread
TARGETS = producer consumer
SHM_NAME = /producer_consumer_shm
CONSUMER_COUNT = 3  # 測試時啟動的 Consumer 數量 (需與 shm_structure.h 一致)
MESSAGE_INTERVAL = 10  # Producer 發送訊息的間隔 (毫秒 R)

# --- 預設目標：編譯所有程式 ---
all: $(TARGETS)

# --- 編譯規則 ---
producer: producer.c shm_structure.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

consumer: consumer.c shm_structure.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# --- 測試目標：執行 run_test.sh 腳本 ---
# 測試邏輯已移至 run_test.sh，以隔離 Group Signal 導致 make 終止的問題。
.PHONY: test
test: $(TARGETS) run_test.sh
	@echo "--- 啟動測試 (Group Signal 版本) ---"
	@chmod +x run_test.sh  # 確保腳本有執行權限
	@echo "Consumer 數量 (C): $(CONSUMER_COUNT)"
	@echo "發送間隔 (R): $(MESSAGE_INTERVAL)ms"
	# 執行測試腳本
	./run_test.sh $(CONSUMER_COUNT) $(MESSAGE_INTERVAL)

# --- 輔助目標：確保 run_test.sh 存在並包含正確的邏輯 ---
# 為了 make 測試的完整性，將 run_test.sh 的內容嵌入這裡 (注意：您仍需創建實際檔案)
run_test.sh:
	@echo "請確保您已創建 run_test.sh 檔案，並包含以下邏輯："
	@echo "#!/bin/bash" > $@
	@echo "trap '' SIGUSR1" >> $@
	@echo "CONSUMER_COUNT=$${1:-$(CONSUMER_COUNT)}" >> $@
	@echo "MESSAGE_INTERVAL=$${2:-$(MESSAGE_INTERVAL)}" >> $@
	@echo "PRODUCER_EXECUTABLE=./producer" >> $@
	@echo "CONSUMER_EXECUTABLE=./consumer" >> $@
	@echo "..." >> $@

# --- 清理目標 ---
.PHONY: clean
clean:
	@echo "--- 清理編譯檔案與共享記憶體 ---"
	$(RM) $(TARGETS)
	# 嘗試刪除共享記憶體檔案 (POSIX shm)
	@if test -f /dev/shm/$(SHM_NAME); then \
		echo "Removing Shared Memory: $(SHM_NAME)"; \
		rm /dev/shm/$(SHM_NAME); \
	else \
		echo "Shared Memory: $(SHM_NAME) not found, skipping."; \
	fi
	# 清理測試腳本 (如果需要)
	# $(RM) run_test.sh
	@echo "清理完成。"