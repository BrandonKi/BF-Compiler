#include <string>
#include <stack>
#include <cstring>
#include <iomanip>

#ifdef unix
#include <sys/mman.h>
#endif

#include "Backend.h"

class BF_x86_64_Backend : public Backend {
private:
    std::string code;
    std::stack<size_t> loop_stack;

public:
    BF_x86_64_Backend() : code()
    {
    }

    virtual void compile(std::string &code_p, Platform platform, Mode mode) {
        code = code_p;

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
            case Mode::jit: {
                // copy the vector.data() into a executable buffer and run
                // bin = { 0x55, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xEC, 0x30, 0x75, 0x00, 0x00, 0x48, 0x31, 0xDB, 0x48, 0xC7, 0x04, 0x1C, 0x41, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x42, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x43, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x44, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0xBA, 0x05, 0x00, 0x00, 0x00, 0x48, 0x89, 0xE6, 0xBF, 0x01, 0x00, 0x00, 0x00, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x8A, 0x44, 0x1C, 0xFF, 0xFE, 0xC0, 0x88, 0x44, 0x1C, 0xFF, 0xBA, 0x05, 0x00, 0x00, 0x00, 0x48, 0x89, 0xE6, 0xBF, 0x01, 0x00, 0x00, 0x00, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x5D, 0x48, 0x81, 0xc4, 0x30, 0x75, 0x00, 0x00, 0xc3};
                // void *block = allocMemory(bin.size());
                // emit(block, bin.data(), bin.size());

                // typedef void (*exe)( void );

                // exe func = (exe)makeExecutable(block);

                // func();

                internal_link();

                std::vector<uint8_t> preamble = {0x55, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xEC, 0x30, 0x75, 0x00, 0x00, 0x48, 0x31, 0xDB};

                std::vector<uint8_t> ret_postamble = {0x5D, 0x48, 0x81, 0xC4, 0x30, 0x75, 0x00, 0x00, 0x31, 0xc0, 0xC3};

                std::vector<uint8_t> exit_postamble = {0x5D, 0x48, 0x81, 0xC4, 0x30, 0x75, 0x00, 0x00, 0xB8, 0x3C, 0x00, 0x00, 0x00, 0x0F, 0x05};

                bin.insert(bin.begin(), preamble.begin(), preamble.end());

                bin.insert(bin.end(), exit_postamble.begin(), exit_postamble.end());

                std::cout << std::hex;
                for (uint8_t i : bin) {
                    std::cout << std::setw(2) << std::setfill('0') << (int)i << " ";
                }
                std::cout << std::endl;

                void *block = allocMemory(bin.size());
                emit(block, bin.data(), bin.size());

                typedef void (*exe)(void);

                exe func = (exe)makeExecutable(block);

                std::cout << std::endl;

                func();

                break;
            }
            case Mode::elf:
                break;
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
        std::vector<uint8_t> inst = {
            0x8A, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rbx - 30000]
            0x04, 0x01,                               // add al, 1
            0x88, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF  // mov byte ptr [rbp + rbx - 30000], al
        };
        bin.insert(bin.end(), inst.begin(), inst.end());
    }

    void dec() {
        std::vector<uint8_t> inst = {
            0x8A, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rbx - 30000]
            0x2c, 0x01,                               // sub al, 1
            0x88, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF  // mov byte ptr [rbp + rbx - 30000], al
        };
        bin.insert(bin.end(), inst.begin(), inst.end());
    }

    void out() {
        /*
            ; edx = length of buffer
            ; rsi = pointer to buffer
            ; edi = 1 for stdout
            ; eax = 1 for write()
            mov  edx, 1
            mov  rsi, rsp
            mov  edi, 1
            mov  eax, 1           ; write
            syscall
        */
        std::vector<uint8_t> inst = {
            0xBA, 0x01, 0x00, 0x00, 0x00, // mov  edx, 1
            0x48, 0x89, 0xE6,             // mov  rsi, rsp
            0xBF, 0x01, 0x00, 0x00, 0x00, // mov  edi, 1
            0xB8, 0x01, 0x00, 0x00, 0x00, // mov  eax, 1
            0x0F, 0x05                    // syscall
        };
        bin.insert(bin.end(), inst.cbegin(), inst.cend());
    }

    void in() {
        /*
            ; edx = length of buffer
            ; rsi = pointer to buffer
            ; edi = 0 for stdin
            ; eax = 1 for read()
            mov  edx, 1
            mov  rsi, rsp
            mov  edi, 0
            mov  eax, 0           ; read
            syscall
        */
        std::vector<uint8_t> inst = {
            0xBA, 0x01, 0x00, 0x00, 0x00,   // mov edx, 1
            0x48, 0x89, 0xE6,               // mov rsi, rsp
            0xBF, 0x00, 0x00, 0x00, 0x00,   // mov edi, 0
            0xB8, 0x00, 0x00, 0x00, 0x00,   // mov eax, 0
            0x0F, 0x05                      // syscall
        };
        bin.insert(bin.end(), inst.cbegin(), inst.cend());
    }

    void start_loop() {
        // jmp 0
        // this is just a placeholder to signify the start
        // of a loop
        std::vector<uint8_t> inst = {0x8A, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF, 0x84, 0xC0, 0x74, 0xfe};
        bin.insert(bin.end(), inst.cbegin(), inst.cend());
    }

    void end_loop() {
        // jmp 0xffffffff
        // this is just a placeholder to signify the end
        // of a loop
        std::vector<uint8_t> inst = {0xeb, 0xfe};
        bin.insert(bin.end(), inst.cbegin(), inst.cend());
    }

    void internal_link() {
        std::stack<size_t> loop_stack;
        uint8_t start[] = {0x74, 0xfe};
        uint8_t end[] = {0xeb, 0xfe};
        for (size_t i = 0; i + 1 < bin.size(); i++) {
            if (memcmp(bin.data() + i, start, 2) == 0) {
                loop_stack.push(i);
                std::cout << "loop start\n";
                // jump to end + 5 if byte at data pointer is 0
            }
            else if (memcmp(bin.data() + i, end, 2) == 0) {
                std::cout << "loop end\n";

                int64_t difference = i - loop_stack.top() + 2;

                emit_JMP(difference, i);

        // uint8_t address2 = 0xfe;
        // address2 += difference; // FIXME assumes that we aren't jumping more than 130ish characters

        // bin[loop_stack.top()] = 0x74;
        // bin[loop_stack.top() + 1] = address2;
        std::cout << "uuhh" << std::endl;
               emit_JE(difference, loop_stack.top());

                loop_stack.pop();
                // unconditional jump to i
            }
        }
    }

    void emit_JMP(int64_t distance, size_t pos) {

        if (distance >= 127) {
            std::cout << "fuck off\n";
            distance -= 130;
            uint32_t address = 0x7d - distance;
            std::vector<uint8_t> last_three_bytes = {
                static_cast<uint8_t>((address >> 16) & 0xff),
                static_cast<uint8_t>((address >> 8) & 0xff),
                static_cast<uint8_t>(address & 0xff)
            };
        }
        else {
            uint8_t address = 0xfe - distance;
            bin[pos] = 0xeb;
            bin[pos + 1] = address;
        }
    }

    void emit_JE(int64_t distance, size_t pos) {

        if (distance >= 127) {
            std::cout << "fuck off\n";

        }
        else {
            uint8_t address = 0xfe + distance;
            bin[pos] = 0x74;
            bin[pos + 1] = address;
        }
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

    void emit(void *m, unsigned char *code, size_t size) {
        std::memcpy(m, code, size);
    }

    void *makeExecutable(void *buf) {

        mprotect(buf, sizeof(*(char *)buf), PROT_READ | PROT_EXEC); // linux

        // DWORD old;
        // VirtualProtect(buf, sizeof(*(char*)buf), PAGE_EXECUTE_READ, &old);       // windows

        return buf;
    }
};
// ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.
