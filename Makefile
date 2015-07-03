# Makefile for opencv_test.c
# example 1
# opencv_test: opencv_test.c
# 	gcc ./opencv_test.c -o opencv_test `pkg-config opencv --cflags --libs`

# example 2
# opencv_test: opencv_test.o
# 	gcc -o $@ $< `pkg-config --libs opencv`
# .c.o:
# 	gcc -o $@ -c $< `pkg-config --cflags opencv`

# コンパイル・リンクして作られる最終的な実行ファイルの名前
TARGET = opencv_test

# オブジェクトファイルを全て並べる
OBJS = $(TARGET).o

# 出力ディレクトリー
OUTDIR = build

# コンパイラに渡すフラグ
CXX = gcc
CXXFLAGS = `pkg-config --cflags opencv`
LDFLAGS  = `pkg-config --libs opencv`

# 生成規則
all: $(TARGET)

$(TARGET): $(OBJS)
	@if [ ! -d $(OUTDIR) ]; then \
		echo ";; mkdir $(OUTDIR)"; mkdir $(OUTDIR); \
	fi
	$(CXX) -o $(OUTDIR)/$@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OUTDIR)/$(TARGET) $(OBJS) *~

# サフィックス
.c.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)
