#ifndef BACKEND_H_GUARD
#define BACKEND_H_GUARD

#include <vector>
#include <string>
#include <cstdint>

enum Platform {
    x86_64,
    arm
};

enum Mode {
    interpret,
    jit,
    pe,
    elf
};

class Backend {
    private:
        std::vector<uint8_t> bin;

    public:
        Backend() {}

        virtual void compile(std::string*, Platform, Mode) = 0;

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