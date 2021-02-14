#include <string>
#include <stack>
#include <cstring>

#ifdef unix
#include <sys/mman.h>
#endif

#include "Backend.h"

class BF_x86_64_Backend : public Backend {
    private:
        std::string code;
        std::stack<size_t> loop_stack;

    public:
        BF_x86_64_Backend():
            code()
        {

        }

        virtual void compile(std::string& code_p, Platform platform, Mode mode) {
            code = code_p;

            for(int i = 0; i < code.size(); i++) {
                switch(code[i]) {
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

            switch(mode) {
                case Mode::jit:{
                    // copy the vector.data() into a executable buffer and run
                    bin = { 0x55, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xEC, 0x30, 0x75, 0x00, 0x00, 0x48, 0x31, 0xDB, 0x48, 0xC7, 0x04, 0x1C, 0x41, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x42, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x43, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0x48, 0xC7, 0x04, 0x1C, 0x44, 0x00, 0x00, 0x00, 0x48, 0xFF, 0xC3, 0xBA, 0x05, 0x00, 0x00, 0x00, 0x48, 0x89, 0xE6, 0xBF, 0x01, 0x00, 0x00, 0x00, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x8A, 0x44, 0x1C, 0xFF, 0xFE, 0xC0, 0x88, 0x44, 0x1C, 0xFF, 0xBA, 0x05, 0x00, 0x00, 0x00, 0x48, 0x89, 0xE6, 0xBF, 0x01, 0x00, 0x00, 0x00, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x5D, 0x48, 0x81, 0xc4, 0x30, 0x75, 0x00, 0x00, 0xc3};
                    void *block = allocMemory(bin.size());
                    emit(block, bin.data(), bin.size());

                    typedef void (*exe)( void );

                    exe func = (exe)makeExecutable(block);

                    func();

                    std::cout << std::endl;
                    break;}
                case Mode::elf:
                    break;
                case Mode::pe:
                    break;
            }
        }

         void dp_inc() {
             // lea rbx, [rbx + 1]
             std::vector<uint8_t> inst = { 0x48, 0x8D, 0x5B, 0x01 };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void dp_dec() {
             // lea rbx, [rbx - 1]
            std::vector<uint8_t> inst = { 0x48, 0x8D, 0x5B, 0xFF };
            bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void inc() {
             std::vector<uint8_t> inst = { 
                 0x8A, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rbx - 30000]
                 0x04, 0x01, // add al, 1
                 0x88, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF // mov byte ptr [rbp + rbx - 30000], al
            };
            bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void dec() {
             std::vector<uint8_t> inst = { 
                 0x8A, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rbx - 30000]
                 0x2c, 0x01, // sub al, 1
                 0x88, 0x84, 0x1D, 0xD0, 0x8A, 0xFF, 0xFF // mov byte ptr [rbp + rbx - 30000], al
            };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void out() {
             /*
                ; edx = length of buffer
                ; rsi = pointer to buffer
                ; edi = 1 for stdout
                ; eax = 1 for write()
                mov  edx, 5
                mov  rsi, rsp
                mov  edi, 1
                mov  eax, 1           ; write
                syscall
             */
            std::vector<uint8_t> inst = { 
                0xBA, 0x05, 0x00, 0x00, 0x00,   // mov  edx, 5
                0x48, 0x89, 0xE6,       // mov  rsi, rsp
                0xBF, 0x01, 0x00, 0x00, 0x00,   // mov  edi, 1
                0xB8, 0x01, 0x00, 0x00, 0x00,   // mov  eax, 1
                0x0F, 0x05      // syscall
            };
            bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void in() {

         }

         void start_loop() {
             // jmp 0
             // this is just a placeholder to signify the start
             // of a loop
             std::vector<uint8_t> inst = { 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00 };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void end_loop() {
             // jmp 0xffffffff
             // this is just a placeholder to signify the end
             // of a loop
             std::vector<uint8_t> inst = { 0x0F, 0x84, 0xff, 0xff, 0xff, 0xff };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

        void internal_link() {
            std::stack<size_t> loop_stack;
            char start[] = { 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00 };
            char end[] = { 0x0F, 0x84, 0xff, 0xff, 0xff, 0xff };
            for(size_t i = 0; i + 5 < bin.size(); i++) {
                if(memcmp(bin.data(), start, 5)) {
                    loop_stack.push(i);
                    // jump to end + 5 if byte at data pointer is 0
                }
                else if(memcmp(bin.data(), end, 5)) {
                    // unconditional jump to i
                }
            }

        }

        void* allocMemory(size_t size) {
            void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);     // linux
            // void* ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);               // windows
            if (ptr == (void*)-1) {
                std::cerr << "ALLOC_ERROR";
                return nullptr;
            }
            return ptr;
        }

        void emit(void* m, unsigned char* code, size_t size) {        
            std::memcpy(m, code, size);
        }

        void* makeExecutable(void* buf){

            mprotect(buf, sizeof(*(char*)buf), PROT_READ | PROT_EXEC);            // linux
            
            // DWORD old;
            // VirtualProtect(buf, sizeof(*(char*)buf), PAGE_EXECUTE_READ, &old);       // windows

            return buf;
        }
};