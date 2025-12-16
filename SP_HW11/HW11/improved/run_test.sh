#!/bin/bash

# 讓 Shell 忽略 SIGUSR1 (User defined signal 1)，防止 Group Signal 導致腳本終止
trap '' SIGUSR1

# --- 參數設定 (與 Makefile 中的保持一致) ---
CONSUMER_COUNT=3
# ... 腳本其餘部分不變 ...
MESSAGE_INTERVAL=10
PRODUCER_EXECUTABLE="./producer"
CONSUMER_EXECUTABLE="./consumer"

# --- 啟動 Producer (背景執行) ---
echo "--- 1. 啟動 Producer (背景執行，並記錄 PID) ---"
$PRODUCER_EXECUTABLE $MESSAGE_INTERVAL &
PRODUCER_PID=$!
echo "Producer 啟動 PID: $PRODUCER_PID"

echo "--- 2. 等待 Producer 建立 SHM ---"
sleep 1

# --- 啟動 Consumers (放入背景並設定 Group ID) ---
echo "--- 3. 啟動 Consumers (放入背景並設定 Group ID) ---"
for i in $(seq 0 $((CONSUMER_COUNT - 1))); do
    echo "Running Consumer $i..."
    $CONSUMER_EXECUTABLE $i &
    sleep 0.1
done

# --- 等待 Producer 結束 (核心步驟：wait) ---
echo "--- 4. 等待 Producer 跑完所有訊息 ---"
# 使用 wait 等待 Producer 結束
wait $PRODUCER_PID
if [ $? -ne 0 ]; then
    echo "錯誤：Producer 未正常退出！"
    exit 1
fi
echo "Producer 完成發送所有訊息。"

# --- 善後處理 ---
echo "--- 5. Producer 完成，等待 Consumers 處理結尾 ---"
sleep 3

echo "--- 6. 終止所有背景 Consumer 程序 ---"
# 終止所有背景中的 Consumer 进程
pkill -SIGKILL consumer || true

echo "測試完成。請查看 Consumer 終端機輸出的接收數量。"