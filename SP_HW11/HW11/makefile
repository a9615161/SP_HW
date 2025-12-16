CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lrt

TARGET = program
DATA_FILE = loss_data.csv
CSV1 = plot1_data.csv
IMG1 = plot1_R.png
CSV2 = plot2_data.csv
IMG2 = plot2_B.png
SCRIPT = plot_hw11.py

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LDFLAGS)

clean:
	rm -f $(TARGET) $(DATA_FILE) *.png
	rm *.csv

# 既有的測試指令
test: $(TARGET)
	./$(TARGET) 100 0 100 10
	./$(TARGET) 100 10 100 10

# 既有的效能測試
test-time: $(TARGET)
	time ./$(TARGET) 100 0 100 1
	strace -c ./$(TARGET) 100 0 100 1

# ==========================================
# 新增目標：自動化測試、產生 CSV 並畫圖
# ==========================================
graph: $(TARGET)
	@echo "開始收集數據..."
	@# 1. 建立 CSV Header
	@echo "R,LossRate" > $(DATA_FILE)
	
	@# 2. 使用迴圈測試不同的 R (傳送速率)，例如 0, 5, 10, ... 50
	@# 參數設定: D=1000, C=100, B=1 (可依需求自行修改)
	@for r in 0 5 10 20 30 40 50; do \
		echo -n "$$r," >> $(DATA_FILE); \
		echo "Running with R=$$r..."; \
		./$(TARGET) 100 $$r 100 10 | grep "Loss rate" | awk -F': ' '{print $$2}' >> $(DATA_FILE); \
	done
	
	@echo "數據已儲存至 $(DATA_FILE)"
	
	@# 3. 呼叫 Python 畫圖
	@echo "正在繪製折線圖..."
	@python3 plot.py
# ========================================================
# 目標 1: 變動 R (10-50)，固定 B=3 [cite: 45]
# ========================================================
plot1: $(TARGET)
	@echo "正在收集 Plot 1 數據 (Loss vs R)..."
	@echo "X,C,LossRate" > $(CSV1)
	
	@# 巢狀迴圈: 外層跑不同的 Consumer 數量 (10, 100, 1000)
	@for c in 10 100 1000; do \
		# 內層跑不同的 R (傳送速率) \
		for r in 10 20 30 40 50; do \
			echo "Running: D=100, R=$$r, C=$$c, B=3"; \
			LOSS=$$(./$(TARGET) 100 $$r $$c 3 | grep "Loss rate" | awk -F': ' '{print $$2}'); \
			echo "$$r,$$c,$$LOSS" >> $(CSV1); \
		done \
	done
	
	@echo "正在繪製 Plot 1..."
	@python3 $(SCRIPT) $(CSV1) $(IMG1) "Loss Rate vs Transmission Rate (Fixed B=3)" "Transmission Rate R (ms)"

# ========================================================
# 目標 2: 變動 B (1-10)，固定 R=500 
# ========================================================
plot2: $(TARGET)
	@echo "正在收集 Plot 2 數據 (Loss vs Buffer)..."
	@echo "X,C,LossRate" > $(CSV2)
	
	@# 巢狀迴圈: 外層跑不同的 Consumer 數量 (10, 100, 1000)
	@for c in 10 100 1000; do \
		# 內層跑不同的 Buffer Size \
		for b in 1 3 5 7 10; do \
			echo "Running: D=100, R=30, C=$$c, B=$$b"; \
			LOSS=$$(./$(TARGET) 100 30 $$c $$b | grep "Loss rate" | awk -F': ' '{print $$2}'); \
			echo "$$b,$$c,$$LOSS" >> $(CSV2); \
		done \
	done
	
	@echo "正在繪製 Plot 2..."
	@python3 $(SCRIPT) $(CSV2) $(IMG2) "Loss Rate vs Buffer Size (Fixed R=500)" "Buffer Size (B)"

# 一鍵產生所有圖表
graphs: plot1 plot2