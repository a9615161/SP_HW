# 編譯器與參數設定
CC = gcc
CFLAGS = -Wall
SERVER_BIN = server
CLIENT_BIN = client

# 預設目標：編譯所有程式
all: $(SERVER_BIN) $(CLIENT_BIN)

# 編譯 Server
$(SERVER_BIN): server.c seq.h
	$(CC) $(CFLAGS) -o $(SERVER_BIN) server.c

# 編譯 Client
$(CLIENT_BIN): client.c seq.h
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) client.c

# 自動化測試腳本
test: all
	@echo "========================================"
	@echo "      Starting Automated Test"
	@echo "========================================"
	@echo "[TEST] Starting Server in background..."
	@# 啟動 Server 並將 PID 寫入檔案，以便稍後關閉
	@./$(SERVER_BIN) > server.log 2>&1 & echo $$! > server.pid
	@sleep 1
	@echo "[TEST] Server is running (PID: `cat server.pid`)"
	
	@echo "\n[TEST] 1. Client A requests 5 numbers"
	./$(CLIENT_BIN) 5
	
	@echo "\n[TEST] 2. Client B requests 10 numbers"
	./$(CLIENT_BIN) 10
	
	@echo "\n[TEST] 3. Client C requests 3 numbers"
	./$(CLIENT_BIN) 3
	
	@echo "\n[TEST] Stopping Server..."
	@kill `cat server.pid`
	@rm server.pid
	@echo "[TEST] Check server.log for server-side output."
	@echo "========================================"
	@echo "      Test Completed Successfully"
	@echo "========================================"

# 清除編譯產生的檔案與暫存檔
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) server.pid server.log

# 強制清除殘留的 Message Queue (如果測試失敗導致 Queue 卡住)
# 注意：這裡使用程式碼中定義的 Key: 0x1aaaaaa1 (十進位: 447392417)
clean-ipc:
	@echo "Removing Message Queue with Key 0x1aaaaaa1..."
	-ipcrm -Q 0x1aaaaaa1 2>/dev/null || echo "Queue not found or already removed."