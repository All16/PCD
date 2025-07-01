CC = gcc
CFLAGS = -Wall -pthread -Iinclude
LDFLAGS = -lmicrohttpd -ljson-c
BIN_DIR = bin

# === TARGET implicit ===
all: $(BIN_DIR)/server $(BIN_DIR)/client_admin $(BIN_DIR)/client_user

# === SERVER ===
$(BIN_DIR)/server: \
    server/main.c \
    server/server_inet.c \
    server/server_unix.c \
    server/server_rest.c \
    server/job_queue.c \
    server/job_processor.c \
    server/logging.c \
    server/client_tracker.c \
    lib/ffmpeg_wrapper.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# === CLIENT ADMIN ===
$(BIN_DIR)/client_admin: \
    client_admin/main.c \
    client_admin/admin_interface.c \
    client_admin/admin_commands.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# === CLIENT USER ===
$(BIN_DIR)/client_user: \
    client_user/main.c \
    client_user/user_interface.c \
    client_user/command_sender.c \
    client_user/file_transfer.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# === CLEAN ===
clean:
	rm -f $(BIN_DIR)/server
	rm -f $(BIN_DIR)/client_admin
	rm -f $(BIN_DIR)/client_user
