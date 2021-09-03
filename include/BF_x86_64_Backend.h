#ifndef BF_x86_64_Backend_H_GUARD
#define BF_x86_64_Backend_H_GUARD

#include <fstream>
#include <string>
#include <stack>
#include <cstring>
#include <iomanip>

#ifdef unix
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include "Backend.h"

#define DEBUG_BUILD

class BF_x86_64_Backend : public Backend {
    private:
        std::string code;
        std::stack<size_t> loop_stack;

        const std::vector<uint8_t> preamble = {0x55, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xEC, 0x30, 0x75, 0x00, 0x00, 0x48, 0x89, 0xe3};
        const std::vector<uint8_t> ret_postamble = {0x48, 0x81, 0xC4, 0x30, 0x75, 0x00, 0x00, 0x5D, 0x31, 0xc0, 0xC3};
        const std::vector<uint8_t> exit_postamble = {0x48, 0x81, 0xC4, 0x30, 0x75, 0x00, 0x00, 0x5D, 0xB8, 0x3C, 0x00, 0x00, 0x00, 0x0F, 0x05};

    public:
        BF_x86_64_Backend():
            code()
        {
        
        }

        virtual void compile(std::string& input_file, Platform platform, Mode mode, std::string& output_file, uint8_t opt_level) {
            code = read_file(input_file);

            for (int i = 0; i < code.size(); i++) {
                switch (code[i]) {
                case '>':
                    dp_inc();
                    break;
                case '<':
                    dp_dec();
                    break;
                case '+':
                    inc();
                    break;
                case '-':
                    dec();
                    break;
                case '.':
                    out();
                    break;
                case ',':
                    in();
                    break;
                case '[':
                    start_loop();
                    break;
                case ']':
                    end_loop();
                    break;
                }
            }

            switch (mode) {
                case Mode::interpret: // interpret shouldn't be possible but fallthrough to jit just in case
                case Mode::jit: 
                {
                    if(opt_level > 0)
                        peepholeOptimization(bin);

                    internal_link();

                    bin.insert(bin.begin(), preamble.begin(), preamble.end());
                    bin.insert(bin.end(), ret_postamble.begin(), ret_postamble.end());


                    #ifdef DEBUG_BUILD
                    std::cout << std::hex;
                    for (uint8_t i : bin) {
                        std::cout << std::setw(2) << std::setfill('0') << (int)i << " ";
                    }
                    std::cout << std::endl;
                    #endif

                    void *block = allocMemory(bin.size());
                    emit(block, bin.data(), bin.size());
                    typedef void (*exe)(void);
                    exe func = (exe)makeExecutable(block);
                    func();
                    dealloc(block, bin.size());
                    break;
                }
                case Mode::elf:
                {
                    if(opt_level > 0)
                        peepholeOptimization(bin);

                    internal_link();

                    bin.insert(bin.begin(), preamble.begin(), preamble.end());
                    bin.insert(bin.end(), exit_postamble.begin(), exit_postamble.end());

                    // be able to change filename in the future
                    write_elf(output_file, bin);
                    set_permissions(output_file);
                    break;
                }
                case Mode::pe:
                    break;
            }
        }

        void dp_inc() {
            // lea rbx, [rbx + 1]
            std::vector<uint8_t> inst = {0x48, 0x8D, 0x5B, 0x01};
            bin.insert(bin.end(), inst.begin(), inst.end());
        }

        void dp_dec() {
            // lea rbx, [rbx - 1]
            std::vector<uint8_t> inst = {0x48, 0x8D, 0x5B, 0xFF};
            bin.insert(bin.end(), inst.begin(), inst.end());
        }

        void inc() {
            std::vector<uint8_t> inst = { 0x80, 0x03, 0x01 };
            bin.insert(bin.end(), inst.begin(), inst.end());
        }

        void dec() {
            std::vector<uint8_t> inst = { 0x80, 0x2b, 0x01 };
            bin.insert(bin.end(), inst.begin(), inst.end());
        }

        void out() {
            std::vector<uint8_t> inst = {
                0xBA, 0x01, 0x00, 0x00, 0x00, // mov  edx, 1
                0x48, 0x89, 0xDE,             // mov  rsi, rbx
                0xBF, 0x01, 0x00, 0x00, 0x00, // mov  edi, 1
                0xB8, 0x01, 0x00, 0x00, 0x00, // mov  eax, 1
                0x0F, 0x05                    // syscall
            };
            bin.insert(bin.end(), inst.cbegin(), inst.cend());
        }

        void in() {
            std::vector<uint8_t> inst = {
                0xBA, 0x01, 0x00, 0x00, 0x00,   // mov edx, 1
                0x48, 0x89, 0xDE,               // mov rsi, rbx
                0xBF, 0x00, 0x00, 0x00, 0x00,   // mov edi, 0
                0xB8, 0x00, 0x00, 0x00, 0x00,   // mov eax, 0
                0x0F, 0x05                      // syscall
            };
            bin.insert(bin.end(), inst.cbegin(), inst.cend());
        }

        void start_loop() {
            // je 0x0
            // this is just a placeholder to signify the start
            std::vector<uint8_t> inst = {0x80, 0x3B, 0x00, 0x74, 0xfe, 0x00, 0x00, 0x00, 0x00};
            bin.insert(bin.end(), inst.cbegin(), inst.cend());
        }

        void end_loop() {
            // jmp 0x0
            // this is just a placeholder to signify the end
            std::vector<uint8_t> inst = {0xe9, 0x00, 0x00, 0x00, 0x00};
            bin.insert(bin.end(), inst.cbegin(), inst.cend());
        }

        void internal_link() {
            std::stack<size_t> loop_stack;
            uint8_t start[] = {0x74, 0xfe, 0x00, 0x00, 0x00, 0x00};
            uint8_t end[] = {0xe9, 0x00, 0x00, 0x00, 0x00};
            for (size_t i = 0; i + 1 < bin.size(); i++) {
                if (memcmp(bin.data() + i, start, 2) == 0) {
                    loop_stack.push(i);
                }
                else if (memcmp(bin.data() + i, end, 2) == 0) {

                    int64_t difference = i - loop_stack.top();

                    emit_JMP(difference, i);
                    emit_JE(difference, loop_stack.top());

                    loop_stack.pop();
                }
            }
        }

        void emit_JMP(int64_t distance, size_t pos) {

                // 8 is the size of all instructions to skip over
                distance = -distance - 8;

                bin[pos] = 0xe9;
                bin[pos + 1] = distance & 0xff;
                bin[pos + 2] = static_cast<uint8_t>((distance >> 8) & 0xff);
                bin[pos + 3] = static_cast<uint8_t>((distance >> 16) & 0xff);
                bin[pos + 4] = static_cast<uint8_t>((distance >> 24) & 0xff);
        }

        void emit_JE(int64_t distance, size_t pos) {
                --distance;
                bin[pos] = 0x0f;
                bin[pos + 1] = 0x84;
                bin[pos + 2] = static_cast<uint8_t>(distance & 0xff);
                bin[pos + 3] = static_cast<uint8_t>((distance >> 8) & 0xff);
                bin[pos + 4] = static_cast<uint8_t>((distance >> 16) & 0xff);
                bin[pos + 5] = static_cast<uint8_t>((distance >> 24) & 0xff);
        }

        void *allocMemory(size_t size) {
            void *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // linux
            // void* ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);               // windows
            if (ptr == (void *)-1) {
                std::cerr << "ALLOC_ERROR";
                return nullptr;
            }
            return ptr;
        }

        void dealloc(void *block, size_t size) {
            munmap(block, size);
            // VirtualFree(block, size, MEM_RELEASE);
        }

        void emit(void *m, unsigned char *code, size_t size) {
            std::memcpy(m, code, size);
        }

        void *makeExecutable(void *buf) {

            mprotect(buf, sizeof(*(char *)buf), PROT_READ | PROT_EXEC); // linux

            // DWORD old;
            // VirtualProtect(buf, sizeof(*(char*)buf), PAGE_EXECUTE_READ, &old);  // windows

            return buf;
        }

        void write_elf(std::string filename, std::vector<uint8_t> bin) {
            std::vector<uint8_t> header = {
                0x7F, 0x45, 0x4C, 0x46, 0x02, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x10, 0x02, 0x00, 0x3E, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x78, 0x80, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x08,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
                0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            bin.insert(bin.begin(), header.begin(), header.end());
            std::ofstream file(filename, std::ios::out|std::ios::binary|std::ios::trunc);
            if(file.is_open())
                file.write((char *)bin.data(), bin.size());
            file.close();
        }

        void set_permissions(std::string filename) {
            chmod(filename.c_str(), S_IRWXU);
        }

        // these transformations would have been trivial if I used an IR
        // but alas I did not, so instead I'm directly optimizing the machine code
        void peepholeOptimization(std::vector<uint8_t>& block) {
            uint8_t dp_inc[] = {0x48, 0x8D, 0x5B, 0x01};
            uint8_t dp_dec[] = {0x48, 0x8D, 0x5B, 0xFF};
            uint8_t inc[] = {0x80, 0x03, 0x01};
            uint8_t dec[] = {0x80, 0x2b, 0x01};

            auto fold_dp_expr = [&](size_t& i) {
                size_t start_i = i;
                int16_t sum = 0;

                while (true) {
                    if(i >= block.size())
                        break;
                    if(memcmp(block.data() + i, dp_inc, 4) == 0) {
                        // uses int8_t instead of uint8_t because of lea
                        if(sum + 1 > static_cast<int16_t>(std::numeric_limits<int8_t>::max()))
                            break;
                        sum += 1;
                    }
                    else if(memcmp(block.data() + i, dp_dec, 4) == 0) {
                        // uses int8_t instead of uint8_t because of lea
                        if(sum - 1 < static_cast<int16_t>(std::numeric_limits<int8_t>::min()))
                            break;
                        sum -= 1;
                    }
                    else
                        break;
                    i += 4;
                }
                if(sum == 0) {
                    block.erase(block.begin() + start_i, block.begin() + i);
                    i = start_i - 4;
                }
                else {
                    // add 4 to keep the first dp instruction
                    block.erase(block.begin() + start_i + 4, block.begin() + i);
                    // std::cout << sum << '\n';
                    block[start_i + 3] = sum;
                    i = start_i + 3;
                }
            };

            auto fold_expr = [&](size_t& i){
                size_t start_i = i;
                int16_t sum = 0;
                
                while (true) {
                    if(i >= block.size())
                        break;
                    else if(memcmp(block.data() + i, inc, 3) == 0) {
                        if(sum + 1 > static_cast<int16_t>(std::numeric_limits<uint8_t>::max()))
                            break;
                        sum += 1;
                    }
                    else if(memcmp(block.data() + i, dec, 3) == 0) {
                        if(sum - 1 < -1 * static_cast<int16_t>(std::numeric_limits<uint8_t>::max()))
                            break;
                        sum -= 1;
                    }
                    else
                        break;
                    i += 3;
                }
                if(sum == 0) {
                    block.erase(block.begin() + start_i, block.begin() + i);
                    i = start_i;
                }
                else if (sum < 0) {
                    // add 3 to keep the first instruction
                    block.erase(block.begin() + start_i + 3, block.begin() + i);
                    // change to sub instruction
                    block[start_i + 1] = 0x2b;
                    block[start_i + 2] = -1 * sum;
                    i = start_i + 2;
                }
                else {
                    // add 3 to keep the first instruction
                    block.erase(block.begin() + start_i + 3, block.begin() + i);
                    block[start_i + 2] = sum;
                    i = start_i + 2;
                }
            };

            // these are only possible because the compiler outputs a small subset of x86_64
            for(size_t i = 0; i < block.size(); ++i) { 
                switch(block[i]) {
                    case 0x48:
                        if(memcmp(block.data() + i, dp_inc, 4) == 0 || memcmp(block.data() + i, dp_dec, 4) == 0)
                            fold_dp_expr(i);
                        break;
                    case 0x80:
                        if(memcmp(block.data() + i, inc, 3) == 0 || memcmp(block.data() + i, dec, 3) == 0)
                            fold_expr(i);
                        break;
                    default:
                        break;
                }
            }
        }
};

#endif