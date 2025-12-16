# 編譯器設定
CC = gcc
CFLAGS = -Wall -O2

# -----------------------------------------------------------
# 檔案名稱定義
# -----------------------------------------------------------
# 執行檔名稱
EXE_SERVER_ORIG = server_orig
EXE_CLIENT_ORIG = client_orig
EXE_SERVER_IMPR = server_impr
EXE_CLIENT_IMPR = client_impr
EXE_CLIENT_DOS  = client_dos

# -----------------------------------------------------------
# 編譯規則
# -----------------------------------------------------------
.PHONY: all clean test1 test2 test3 test4 test5 help reset

all: $(EXE_SERVER_ORIG) $(EXE_CLIENT_ORIG) $(EXE_SERVER_IMPR) $(EXE_CLIENT_IMPR) $(EXE_CLIENT_DOS)

# 1. 原始版本 Server (Book Listing 44-7)
$(EXE_SERVER_ORIG): fifo_seqnum_server.c fifo_seqnum.h
	$(CC) $(CFLAGS) -o $@ fifo_seqnum_server.c

# 2. 原始版本 Client (Book Listing 44-8)
$(EXE_CLIENT_ORIG): fifo_seqnum_client.c fifo_seqnum.h
	$(CC) $(CFLAGS) -o $@ fifo_seqnum_client.c

# 3. 改進版 Server (防禦 DoS)
$(EXE_SERVER_IMPR): server.c fifo_seqnum.h
	$(CC) $(CFLAGS) -o $@ server.c

# 4. 改進版 Client (配合非阻塞 Server)
$(EXE_CLIENT_IMPR): client.c fifo_seqnum.h
	$(CC) $(CFLAGS) -o $@ client.c

# 5. 惡意 DoS Client
$(EXE_CLIENT_DOS): client_dos.c fifo_seqnum.h
	$(CC) $(CFLAGS) -o $@ client_dos.c

# 清除編譯檔案與殘留的 FIFO
clean:
	rm -f $(EXE_SERVER_ORIG) $(EXE_CLIENT_ORIG) $(EXE_SERVER_IMPR) $(EXE_CLIENT_IMPR) $(EXE_CLIENT_DOS)
	rm -f /tmp/seqnum_sv /tmp/seqnum_cl.* *.pid

# -----------------------------------------------------------
# 測試場景指令碼
# -----------------------------------------------------------

# 輔助：重置環境 (使用 -x 避免誤殺 make)
reset:
	@pkill -x $(EXE_SERVER_ORIG) || true
	@pkill -x $(EXE_SERVER_IMPR) || true
	@pkill -x $(EXE_CLIENT_DOS)  || true
	@rm -f /tmp/seqnum_sv /tmp/seqnum_cl.* *.pid
	@sleep 0.5

# 場景 1: 原版 Server + 原版 Client (預期：成功)
test1: all reset
	@echo "=================================================="
	@echo "測試 1: 原版 Server + 原版 Client (預期: 正常輸出)"
	@echo "=================================================="
	@./$(EXE_SERVER_ORIG) & echo $$! > server.pid
	@sleep 1
	@echo ">> 啟動 client_orig..."
	@./$(EXE_CLIENT_ORIG) 1 || echo "!! 執行失敗 !!"
	@echo ">> 清理 Server..."
	@kill `cat server.pid` && rm server.pid
	@echo "--------------------------------------------------"

# 場景 2: 原版 Server + 惡意 Client (預期: DoS 成功/Server卡住)
test2: all reset
	@echo "=================================================="
	@echo "測試 2: 原版 Server + 惡意 Client (預期: DoS 成功/Server卡住)"
	@echo "=================================================="
	@./$(EXE_SERVER_ORIG) & echo $$! > server.pid
	@sleep 1
	@echo ">> 啟動惡意 client_dos (背景執行)..."
	@./$(EXE_CLIENT_DOS) > /dev/null &
	@sleep 1
	@echo ">> 嘗試啟動正常 client_orig 驗證 Server 狀態..."
	@# 使用 timeout 指令，如果 3 秒內沒反應代表 Server 已被 DoS
	@if command -v timeout >/dev/null; then \
		timeout 3s ./$(EXE_CLIENT_ORIG) 1 && echo "!! 失敗: Server 竟然活著 (不符合預期)" || echo ">> 成功驗證: Client Timeout，代表 Server 已被 DoS 卡死！"; \
	else \
		echo "警告: 系統無 timeout 指令，無法自動驗證卡死狀態"; \
	fi
	@echo ">> 清理 Server 與惡意 Client..."
	@# 【修正】這裡原本有 pkill -f，現在改用更安全的清理方式
	@kill `cat server.pid` 2>/dev/null || true
	@rm server.pid
	@pkill -x $(EXE_CLIENT_DOS) || true
	@echo "--------------------------------------------------"

# 場景 3: 改進版 Server + 惡意 Client (預期: 惡意請求被拒絕，Server 存活)
test3: all reset
	@echo "=================================================="
	@echo "測試 3: 改進版 Server + 惡意 Client (預期: Server 存活)"
	@echo "=================================================="
	@./$(EXE_SERVER_IMPR) & echo $$! > server.pid
	@sleep 1
	@echo ">> 啟動惡意 client_dos..."
	@./$(EXE_CLIENT_DOS) > /dev/null &
	@sleep 1
	@echo ">> 嘗試啟動正常 client_impr 驗證 Server 狀態..."
	@./$(EXE_CLIENT_IMPR) 1 && echo ">> 成功驗證: 正常 Client 獲得回應，Server 未被 DoS" || echo "!! 失敗: Server 無回應"
	@echo ">> 清理..."
	@kill `cat server.pid` && rm server.pid
	@pkill -x $(EXE_CLIENT_DOS) || true
	@echo "--------------------------------------------------"

# 場景 4 (壓力測試): 改進版 Server + 10 個原版 Client
# 目的：透過並發執行增加 CPU 負載，強制觸發競態條件 (Race Condition)。
# 預期：會有部分 Client 成功 (運氣好)，但會有部分 Client 失敗/超時 (因為動作太慢被 Server 放棄)。
test4: all reset
	@echo "=================================================="
	@echo "測試 4 (壓力測試): 改進版 Server + 10 個原版 Client"
	@echo "=================================================="
	@./$(EXE_SERVER_IMPR) & echo $$! > server.pid
	@sleep 1
	@echo ">> 啟動 10 個原版 client_orig (並發執行)..."
	@# 使用子 shell 與 seq 產生 10 個背景執行的 client
	@# 每個 client 都有 2 秒的 timeout，如果被 Server 拒絕，Client 會卡在 open() 直到超時
	@for i in `seq 1 10`; do \
		(timeout 2s ./$(EXE_CLIENT_ORIG) 1 > /dev/null \
			&& echo "  [OK] Client $$i 成功獲取序號") \
			|| echo "  [FAIL] Client $$i 失敗 (Timeout/被拒絕)" & \
	done; \
	wait
	@echo ">> (若出現 [FAIL]，證明原版 Client 配合非阻塞 Server 是不安全的)"
	@echo ">> 清理..."
	@kill `cat server.pid` && rm server.pid
	@echo "--------------------------------------------------"
# 場景 5: 改進版 Server + 改進版 Client (預期: 成功)
test5: all reset
	@echo "=================================================="
	@echo "測試 5: 改進版 Server + 改進版 Client (預期: 正常輸出)"
	@echo "=================================================="
	@./$(EXE_SERVER_IMPR) & echo $$! > server.pid
	@sleep 1
	@echo ">> 啟動改進版 client_impr..."
	@./$(EXE_CLIENT_IMPR) 1 && echo ">> 成功: 互動正常" || echo "!! 失敗 !!"
	@echo ">> 清理..."
	@kill `cat server.pid` && rm server.pid
	@echo "--------------------------------------------------"

# 執行所有測試
test-all: test1 test2 test3 test4 test5