#include "BFCompiler.h"

BFCompiler::BFCompiler(std::string& data, Platform platform, Mode mode):
    data(data), platform(platform), mode(mode)
{
    
}

i32 BFCompiler::compile() {
    if(mode == interpret)
        backend = new BF_Interpreter_Backend{};
    else {
        switch(platform) {
            case x86_64:
                backend = new BF_x86_64_Backend(mode);
                break;
            case arm64:
                // backend = new BF_ARM64_Backend(mode);
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