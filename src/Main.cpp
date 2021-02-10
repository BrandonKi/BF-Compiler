#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "BFCompiler.h"

std::string readFile(const std::string&);

int main(int argc, char* argv[]) {
    std::string data = readFile(argv[1]);
    BFCompiler compiler(data, BFCompiler::Platform::x86_64, BFCompiler::Mode::interpret);
    compiler.compile();
    std::cin.get();
}

std::string readFile(const std::string& filepath) {
    std::ifstream file;
    file.open(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}