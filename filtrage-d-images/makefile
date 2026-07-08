CC = gcc

CFLAGS = -std=c2x \
         -D_POSIX_C_SOURCE=200809L \
         -Wpedantic -Wall -Wextra -Wconversion -Werror \
         -fstack-protector-all -fpie -O2 \
         -D_FORTIFY_SOURCE=2 \
         -MMD -MP \
         -isystem include

LDFLAGS = -lrt -lm -pthread

OBJ_DIR = build
SRC_DIR = src

TARGET_CLIENT = $(OBJ_DIR)/client
TARGET_SERVER = $(OBJ_DIR)/server
TARGET_WORKER = $(OBJ_DIR)/worker

all: $(TARGET_CLIENT) $(TARGET_SERVER) $(TARGET_WORKER)


$(TARGET_CLIENT): $(OBJ_DIR)/client.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): $(OBJ_DIR)/server.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_WORKER): $(OBJ_DIR)/worker.o $(OBJ_DIR)/img.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJ_DIR)/*.d


clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER) $(TARGET_WORKER)

.PHONY: all clean


