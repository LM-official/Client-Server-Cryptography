# ============================================================================
#  Decipher - Client/Server XOR cipher over TCP
#  Build both the client and the server from src/ into bin/.
# ============================================================================

# ---- Toolchain & flags -----------------------------------------------------
CC       ?= gcc
CSTD     ?= gnu11
CFLAGS   ?= -Wall -Wextra -O2 -std=$(CSTD) -pthread
CPPFLAGS := -Iinclude
LDLIBS   := -pthread

# ---- Layout ----------------------------------------------------------------
SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin

COMMON_OBJ := $(BUILD_DIR)/common.o
CLIENT_BIN := $(BIN_DIR)/client
SERVER_BIN := $(BIN_DIR)/server

# Runtime output files written by the server:
#   <prefix><client-ip>:<YYYY-MM-DD;HH:MM:SS>.txt
# The timestamp signature keeps this from matching input files (e.g. message.txt).
OUTPUT_FILES := *:????-??-??;??:??:??.txt

# ---- Default goal ----------------------------------------------------------
.PHONY: all
all: $(CLIENT_BIN) $(SERVER_BIN)  ## Build both client and server (default)

.PHONY: client
client: $(CLIENT_BIN)             ## Build only the client

.PHONY: server
server: $(SERVER_BIN)             ## Build only the server

# ---- Link rules ------------------------------------------------------------
$(CLIENT_BIN): $(BUILD_DIR)/client.o $(COMMON_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(SERVER_BIN): $(BUILD_DIR)/server.o $(COMMON_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# ---- Compile rule ----------------------------------------------------------
# -MMD -MP generates a .d file per object listing its header dependencies, so
# editing a header (e.g. include/common.h) correctly triggers a rebuild.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# ---- Output directories ----------------------------------------------------
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# ---- Convenience targets ---------------------------------------------------
.PHONY: run-server
run-server: $(SERVER_BIN)         ## Run the server: make run-server ARGS="4 out_ 8"
	./$(SERVER_BIN) $(ARGS)

.PHONY: run-client
run-client: $(CLIENT_BIN)         ## Run the client: make run-client ARGS="file.txt 12345678 4 127.0.0.1 49153"
	./$(CLIENT_BIN) $(ARGS)

.PHONY: clean-out
clean-out:                        ## Remove server-generated output files
	find . -maxdepth 1 -type f -name '$(OUTPUT_FILES)' -delete

.PHONY: clean
clean: clean-out                  ## Remove build artifacts, binaries and run output
	$(RM) -r $(BUILD_DIR) $(BIN_DIR)

.PHONY: help
help:                             ## Show this help
	@grep -hE '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-14s\033[0m %s\n", $$1, $$2}'

# ---- Auto-generated header dependencies ------------------------------------
-include $(wildcard $(BUILD_DIR)/*.d)
