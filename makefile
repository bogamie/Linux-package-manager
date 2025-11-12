# 컴파일러 및 옵션 설정
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lncurses

# 실행 파일 이름
TARGET = package_manager

# 소스 파일 및 오브젝트 파일
SRCDIR = src
OBJDIR = obj
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# 기본 규칙
all: $(TARGET)

# 실행 파일 빌드 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 오브젝트 파일 빌드 규칙
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 클린 규칙
clean:
	rm -rf $(OBJDIR) $(TARGET)
	clear

# 재빌드 규칙
rebuild: clean all

# 디버그용 규칙
debug: CFLAGS += -g
debug: rebuild

# 헬프 메시지
help:
	@echo "사용 가능한 명령:"
	@echo "  all      : 프로그램 빌드 (기본)"
	@echo "  clean    : 빌드 파일 제거"
	@echo "  rebuild  : 프로그램 재빌드"
	@echo "  debug    : 디버그 정보 포함하여 빌드"
	@echo "  help     : 명령 목록 출력"
