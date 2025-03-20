# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -g -march=native `pkg-config --cflags glib-2.0`
LDFLAGS = -lcurl -lcjson -lglib-2.0 -lpthread -lasound `pkg-config --libs glib-2.0`

# 소스 및 헤더 디렉토리
SRC_DIR = srcs
INC_DIR = incs

# 소스 파일 목록
SOURCES = $(SRC_DIR)/audio_capture.c \
          $(SRC_DIR)/connection.c \
          $(SRC_DIR)/main.c \
          $(SRC_DIR)/STT_thread.c \
          $(SRC_DIR)/TTS_handler.c \
          $(SRC_DIR)/STT_handler.c

# 오브젝트 파일 목록 (소스 파일과 동일한 이름, .o 확장자)
OBJECTS = $(SOURCES:.c=.o)

# 실행 파일 이름
TARGET = Raspberry_client

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# .o 파일 생성 규칙
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(INC_DIR)

# 정리 규칙
clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re