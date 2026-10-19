# Chặng 1: Khởi tạo các biến đường dẫn thư mục
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests
RESULTS_DIR = results
# Chặng 2: Cấu hình trình biên dịch
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR)