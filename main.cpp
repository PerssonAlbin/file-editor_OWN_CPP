
#include "file_editor.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    std::string arg_copy = "";
    if(argc > 1)
    {
        arg_copy = argv[1];
    }
    else if(argc < 2) 
    {
        std::cout << "Max 2 arguments allowed\n";
        return -1;
    }
    try
    {
        FileEditor editor(argc, arg_copy);
        editor.runtime();
    }
    catch(std::invalid_argument& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    
    //std::fstream my_file;
    //my_file.open(path, std::ios_base::in | std::ios_base::out);
    //my_file.close();
    return 0;
}