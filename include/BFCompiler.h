#ifndef BFCompiler_H_GUARD
#define BFCompiler_H_GUARD

#include <cstdint>
#include <string>
#include <vector>

#include "Backend.h"
#include "BF_Interpreter_Backend.h"
#include "BF_x86_64_Backend.h"

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

class BFCompiler {
    public:

        /**
         * @brief Construct a new BFCompiler object
         * 
         * @param input_file path to bf source file
         * @param platform Platform struct
         * @param mode Mode struct
         * @param output_file path to output file
         * 
         */
        BFCompiler(std::string& input_file, Platform platform, Mode mode, std::string& output_file, uint8_t);

        /**
         * compile the file and return the error code
         * 
         * @return exit code
         */
        i32 compile();

    private:
        std::string input_file;
        Platform platform;
        Mode mode;
        std::string output_file;
        uint8_t opt_level;

        Backend *backend;
        
        std::vector<u8> generate_raw();

};

#endif