CC = gcc
CFLAGS = -Wall -pthread -Iinclude
LDFLAGS = -lmicrohttpd -ljson-c
BIN_DIR = bin

all: $(BIN_DIR)/server

$(BIN_DIR)/server: server/main.c \
                   server/server_inet.c \
                   server/server_unix.c \
                   server/server_rest.c \
                   server/job_queue.c \
                   server/job_processor.c \
                   server/logging.c \
                   lib/ffmpeg_wrapper.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN_DIR)/server
