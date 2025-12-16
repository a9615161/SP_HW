# 設定要使用的編譯器
CC = gcc
# 設定你想要的編譯參數
# -Wall = 開啟所有警告
# -g    = 加入除錯資訊，方便 GDB 使用
CFLAGS = -Wall -g

# 1. 找出當前目錄下所有的 .c 檔案
#    wildcard 函數會找出所有符合 *.c 模式的檔案
SOURCES = $(wildcard *.c)

# 2. 根據 .c 檔案列表，產生對應的 .out 目標檔案列表
#    這會把 "file1.c file2.c" 轉換成 "file1.out file2.out"
TARGETS = $(SOURCES:.c=.out)

# 3. "all" 是預設目標
#    當你只輸入 `make` 時，它會試圖建立 `all`
#    `all` 依賴於所有的 .out 檔案，所以 make 會去設法產生所有 .out 檔
all: $(TARGETS)

# 4. 模式規則：告訴 make 如何從 .c 檔案產生 .out 檔案
#    %.out: %.c  的意思是：
#    「任何一個 .out 檔案 (%.out) 都依賴於同名的 .c 檔案 (%.c)」
%.out: %.c

	$(CC) $(CFLAGS) -o $@ $<

# 5. "clean" 目標，用來清除所有編譯出來的檔案
#    執行 `make clean` 就會執行下面的指令
clean:
	rm -f *.out

# 6. .PHONY 告訴 make，"all" 和 "clean" 並不是真正的檔案名稱
#    這樣可以避免你剛好有名為 "clean" 的檔案時，make clean 會出錯
.PHONY: all clean