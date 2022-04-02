main: main.cpp file_editor.hpp
		g++ -g -w -o build/file_editor.out main.cpp -std=c++17

debug: main.cpp file_editor.hpp debug/recv_debug.cpp debug/send_debug.hpp
		g++ -g -w -o build/file_editor.out main.cpp -std=c++17
		g++ -g -w -o build/recv.out debug/recv_debug.cpp -std=c++17

clean:
		rm build/*.out