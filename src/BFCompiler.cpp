#include "BFCompiler.h"

BFCompiler::BFCompiler(std::string& input_file, Platform platform, Mode mode, std::string& output_file):
    input_file(input_file), platform(platform), mode(mode), output_file(output_file)
{
    
}

i32 BFCompiler::compile() {
    if(mode == interpret)
        backend = new BF_Interpreter_Backend{};
    else {
        switch(platform) {
            case x86_64:
                backend = new BF_x86_64_Backend();
                break;
            case arm64:
                // backend = new BF_ARM64_Backend(mode);
                break;
        }
    }
    generate_raw();
    return 0;
}

std::vector<u8> BFCompiler::generate_raw() {
    backend->compile(input_file, platform, mode, output_file);
    return backend->get_bin();
}