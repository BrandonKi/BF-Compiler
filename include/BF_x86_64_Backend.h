#include <string>
#include <stack>

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
            // xor eax, eax
            bin.push_back(0x31);
            bin.push_back(0xc0);

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
                case Mode::jit:
                    // copy the vector.data() into a executable buffer and run
                    break;
                case Mode::elf:
                    break;
                case Mode::pe:
                    break;
            }
        }

         void dp_inc() {
             // lea eax, [eax + 1]
             bin.push_back(0x67);
             bin.push_back(0x8d);
             bin.push_back(0x40);
             bin.push_back(0x01);
         }

         void dp_dec() {
             // lea eax, [eax - 1]
             bin.push_back(0x67);
             bin.push_back(0x8d);
             bin.push_back(0x40);
             bin.push_back(0xff);
         }

         void inc() {
             std::vector inst = { 
                 0x8A, 0x84, 0x0D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rcx - 30000]
                 0x04, 0x01, // add al, 1
                 0x88, 0x84, 0x0D, 0xD0, 0x8A, 0xFF, 0xFF // mov byte ptr [rbp + rcx - 30000], al
            };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void dec() {
             std::vector inst = { 
                 0x8A, 0x84, 0x0D, 0xD0, 0x8A, 0xFF, 0xFF, // mov al, byte ptr [rbp + rcx - 30000]
                 0x2c, 0x01, // sub al, 1
                 0x88, 0x84, 0x0D, 0xD0, 0x8A, 0xFF, 0xFF // mov byte ptr [rbp + rcx - 30000], al
            };
             bin.insert(bin.end(), inst.begin(), inst.end());
         }

         void out() {
             /*
                push rax
                mov     edx, 1       ; buffer length
                mov     ecx, byte ptr [rbp + rcx - 30000]      ; pointer to buffer
                mov     ebx,1

                mov     eax,4
                int     0x80 
                pop rax
             */
         }

         void in() {

         }

         void start_loop() {
             // jmp 0
             // this is just a placeholder to signify the start
             // of a loop
             bin.push_back(0xe9);
             bin.push_back(0x00);
             bin.push_back(0x00);
             bin.push_back(0x00);
             bin.push_back(0x00);
         }

         void end_loop() {
             // jmp 0xffffffff
             // this is just a placeholder to signify the end
             // of a loop
             bin.push_back(0xe9);
             bin.push_back(0xff);
             bin.push_back(0xff);
             bin.push_back(0xff);
             bin.push_back(0xff);
         }
};