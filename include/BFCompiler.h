#ifndef BFCompiler_H_GUARD
#define BFCompiler_H_GUARD

#include <cstdint>
#include <string>
#include <vector>
#include <ranges>

#include "Backend.h"
#include "BF_Interpreter_Backend.h"

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;

class BFCompiler {
    public:

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

        BFCompiler(std::string&, Platform, Mode);

        /**
         * compile the file and return the error code
         */
        i32 compile();
        std::string get_error_message();

    private:
        std::string data;
        Platform platform;
        Mode mode;

        Backend *backend;

        
        std::vector<u8> generate_raw();

};

#endif