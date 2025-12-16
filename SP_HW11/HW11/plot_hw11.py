import matplotlib
matplotlib.use('Agg') # 不使用 GUI，直接存圖，避免 Server 端錯誤
import matplotlib.pyplot as plt
import csv
import sys

# 使用方式: python3 plot_hw11.py [csv檔名] [輸出圖片檔名] [圖表標題] [X軸標籤]
if len(sys.argv) < 5:
    print("Usage: python3 plot_hw11.py [csv] [output] [title] [xlabel]")
    sys.exit(1)

csv_file = sys.argv[1]
output_file = sys.argv[2]
chart_title = sys.argv[3]
x_label = sys.argv[4]

# 資料結構: data[c_value] = [(x, loss), (x, loss)...]
data = {}

try:
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            c_val = int(row['C'])
            x_val = int(row['X']) 
            loss = float(row['LossRate'])
            
            if c_val not in data:
                data[c_val] = []
            data[c_val].append((x_val, loss))

    # 繪圖
    plt.figure(figsize=(10, 6))
    
    # 確保圖例 (Legend) 依照 C 的大小排序
    sorted_cs = sorted(data.keys())
    
    for c in sorted_cs:
        # 確保連線是依照 X 軸排序的
        points = sorted(data[c], key=lambda k: k[0])
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        plt.plot(xs, ys, marker='o', label=f'C={c}')

    plt.title(chart_title)
    plt.xlabel(x_label)
    plt.ylabel('Loss Rate')
    plt.ylim(-0.05, 1.05) # 設定 Y 軸範圍 0~1 比較好看
    plt.grid(True)
    plt.legend(title="Consumers")
    
    plt.savefig(output_file)
    print(f"圖表已建立: {output_file}")

except Exception as e:
    print(f"繪圖發生錯誤: {e}")