#ifndef BACKEND_H_GUARD
#define BACKEND_H_GUARD

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>

enum Platform {
    x86_64,
    arm64
};

enum Mode {
    interpret,
    jit,
    pe,
    elf
};

class Backend {
    protected:
        std::vector<uint8_t> bin;

        std::string read_file(const std::string& filepath) {
            std::ifstream file;
            file.open(filepath);
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            return buffer.str();
        }

    public:
        Backend() {}

        virtual void compile(std::string&, Platform, Mode, std::string&) = 0;

         void dp_inc();
         void dp_dec();
         void inc();
         void dec();
         void out();
         void in();
         void start_loop();
         void end_loop();

        std::vector<uint8_t> get_bin() { return bin; }

};

#endif