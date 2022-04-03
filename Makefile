main: main.cpp file_editor.hpp
		g++ -g -w -o build/file_editor.out main.cpp -std=c++17

debug: debug/recv_debug.cpp
		g++ -g -w -o build/recv.out debug/recv_debug.cpp -std=c++17

clean:
		rm build/*.out