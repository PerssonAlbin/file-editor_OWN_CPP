ifeq ($(OS), Windows_NT)
    BIN := bin\\
    BUILD := build\\
    INCLUDES := include\\
    SOURCE := source\\
    DELETE := del

    EXECUTABLE := $(BIN)file_editor.exe
else
    BIN := bin/
    BUILD := build/
    INCLUDES := include/
    SOURCE := source/
    DELETE := rm

    EXECUTABLE := $(BIN)file_editor.out
endif

CPPFLAGS := -g -Werror -Wpedantic -Wall -Wextra -std=c++17

.PHONY: all
all: build 

.PHONY: build
build: $(EXECUTABLE) ## Build file editor
$(EXECUTABLE): $(BUILD)file_editor.o $(BUILD)send_debug.o $(BUILD)main.o
		g++ $(BUILD)main.o $(BUILD)file_editor.o $(BUILD)send_debug.o -I ./ -o $(EXECUTABLE)

$(BUILD)send_debug.o: $(SOURCE)send_debug.cpp $(INCLUDES)send_debug.hpp
		g++ $(CPPFLAGS) -c $(SOURCE)send_debug.cpp -I ./ -o $(BUILD)send_debug.o

$(BUILD)file_editor.o: $(SOURCE)file_editor.cpp $(INCLUDES)file_editor.hpp
		g++ $(CPPFLAGS) -c $(SOURCE)file_editor.cpp -I ./ -o $(BUILD)file_editor.o

$(BUILD)main.o: $(SOURCE)main.cpp
		g++ $(CPPFLAGS) -c $(SOURCE)main.cpp -I ./ -o $(BUILD)main.o

.PHONY: debug
debug: $(SOURCE)recv_debug.cpp ## Build debug receiver
		g++ $(CPPFLAGS) $(SOURCE)recv_debug.cpp -o $(BIN)recv.out

.PHONY: clean
clean: ## Clean up all build artifacts
        ifneq ("$(wildcard $(EXECUTABLE))","")
		    -$(DELETE) $(EXECUTABLE)
        endif
        ifneq ("$(wildcard $(BUILD)send_debug.o)","")
		    -$(DELETE) $(BUILD)send_debug.o
        endif
        ifneq ("$(wildcard $(BUILD)file_editor.o)","")
		    -$(DELETE) $(BUILD)file_editor.o
        endif
        ifneq ("$(wildcard $(BUILD)main.o)","")
		    -$(DELETE) $(BUILD)main.o
        endif

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'