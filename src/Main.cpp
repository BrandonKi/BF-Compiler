#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "BFCompiler.h"

struct args_t {
    std::string filepath;
    Platform platform = Platform::x86_64;
    Mode mode = Mode::interpret;
    std::string output_filepath = "a.out";
};

args_t parse_args(int argc, char *argv[]) {
    args_t result{};
    std::vector<std::string_view> arg_vector(argv + 1, argv + argc);

    for(const auto& str : arg_vector) {
        if(str.substr(0, 2) == "-p") {
            if(str == "-px86" || str == "-px64" || str == "-px86_x64")  // 32 bit x86 isn't supported
                result.platform = Platform::x86_64;     
            else if(str == "-parm" || str == "-parm64")                 // 32 bit arm isn't supported
                result.platform = Platform::arm64;
        }
        else if(str.substr(0, 2) == "-m") {
            if(str == "-minterpret")
                result.mode = Mode::interpret;
            else if(str == "-mjit")
                result.mode = Mode::jit;
            else if(str == "-melf")
                result.mode = Mode::elf;
            else if(str == "-mpe")
                result.mode = Mode::pe;
        }
        else if(str.substr(0, 2) == "-o") {
            result.output_filepath = str.substr(2);
        }
        else if(str.substr(0, 2) == "-O") {
            // optimization levels???
        }
        else {
            if(str == arg_vector.back())
                result.filepath = str;
            else {
                std::cout << "Unknown command line argument: " << str << '\n';
                std::cout << "usage: [-pPlatform] [-mMode] [-oOutput_Path] input_file\n";
                std::exit(-1);
            }
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    args_t args = parse_args(argc, argv);
    BFCompiler compiler(args.filepath, args.platform, args.mode, args.output_filepath);
    compiler.compile();
}