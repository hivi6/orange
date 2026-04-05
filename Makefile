BIN_NAME   := orange
BUILD_PATH := build
BIN_PATH   := $(BUILD_PATH)/$(BIN_NAME)

C_FILES := $(wildcard src/*.c)
H_FILES := $(wildcard include/*.h)

C_FLAGS ?=

$(BIN_PATH): $(BUILD_PATH) $(C_FILES) $(H_FILES)
	$(CC) $(C_FLAGS) $(C_FILES) -Iinclude -o $(BIN_PATH) 

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)

