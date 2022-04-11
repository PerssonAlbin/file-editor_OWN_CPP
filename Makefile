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

all: $(EXECUTABLE)

$(EXECUTABLE): $(BUILD)file_editor.o $(BUILD)send_debug.o $(BUILD)main.o
		g++ $(BUILD)main.o $(BUILD)file_editor.o $(BUILD)send_debug.o -o $(EXECUTABLE)

$(BUILD)send_debug.o: $(SOURCE)send_debug.cpp $(INCLUDES)send_debug.hpp
		g++ $(CPPFLAGS) -c $(SOURCE)send_debug.cpp -I $(INCLUDES) -o $(BUILD)send_debug.o

$(BUILD)file_editor.o: $(SOURCE)file_editor.cpp $(INCLUDES)file_editor.hpp
		g++ $(CPPFLAGS) -c $(SOURCE)file_editor.cpp -I $(INCLUDES) -o $(BUILD)file_editor.o

$(BUILD)main.o: $(SOURCE)main.cpp
		g++ $(CPPFLAGS) -c $(SOURCE)main.cpp -I $(INCLUDES) -o $(BUILD)main.o

debug: $(SOURCE)recv_debug.cpp
		g++ $(CPPFLAGS) $(SOURCE)recv_debug.cpp -o $(BIN)recv.out

clean:
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