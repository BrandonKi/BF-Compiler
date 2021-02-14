#include "BFCompiler.h"

BFCompiler::BFCompiler(std::string& data, Platform platform, Mode mode):
    data(data), platform(platform), mode(mode)
{
    
}

i32 BFCompiler::compile() {
    if(mode == Mode::interpret) {
        backend = new BF_Interpreter_Backend{};
    } else {
        switch(mode) {
            case jit:
            case pe:
            case elf:
                backend = new BF_x86_64_Backend{};
                break;
        }
        switch(platform) {
            case x86_64:
            case arm:
                break;
        }
    }
    generate_raw();
    return 0;
}

std::string BFCompiler::get_error_message() {
    return std::string{};
}

std::vector<u8> BFCompiler::generate_raw() {
    backend->compile(data, platform, mode);
    return backend->get_bin();
}