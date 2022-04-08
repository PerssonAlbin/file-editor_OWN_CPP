main: main.cpp file_editor.hpp file_editor.cpp debug/send_debug.hpp
		g++ -g -w -o build/file_editor.out main.cpp file_editor.cpp -std=c++17

debug: debug/recv_debug.cpp
		g++ -g -w -o build/recv.out debug/recv_debug.cpp -std=c++17

clean:
		rm build/*.out