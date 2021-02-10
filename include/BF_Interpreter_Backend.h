#ifndef BF_Interpreter_Backend_H_GUARD
#define BF_Interpreter_Backend_H_GUARD

#include <iostream>
#include <stack>

#include "Backend.h"

class BF_Interpreter_Backend : public Backend {
    private:
        char data[4000];
        size_t data_pointer;
        size_t instruction_pointer;
        std::stack<size_t> loop_stack;
        std::string* code;

    public:
        BF_Interpreter_Backend():
            data(), data_pointer(0), instruction_pointer(0), loop_stack()
        {

        }

        virtual void compile(std::string* code_p, Platform platform, Mode mode) {
            code = code_p;
            for(instruction_pointer = 0; instruction_pointer < code->size(); instruction_pointer++) {
                switch((*code)[instruction_pointer]) {
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
        }

        virtual void jit_compile(std::string* code_p) {
            
        }

        void dp_inc() {
            data_pointer += 1;
        }
        void dp_dec() {
            data_pointer -= 1;
        }
        void inc() {
            data[data_pointer] += 1;
        }
        void dec() {
            data[data_pointer] -= 1;
        }
        void out() {
            std::cout << data[data_pointer];
        }
        void in() {
            data[data_pointer] = static_cast<char>(getchar());
        }
        void start_loop() {
            if(data[data_pointer] == 0)
                jump_to_end_of_loop();
            loop_stack.push(instruction_pointer);
        }
        void end_loop() {
            if(loop_stack.empty()) {
                std::cout << "bruh wtf are you doing" << std::endl;
                std::terminate();
            }
            instruction_pointer = loop_stack.top() - 1;
            loop_stack.pop();
        }

        private:
            void jump_to_end_of_loop() {
                size_t loop_depth = 0;
                instruction_pointer++;
                for(; instruction_pointer < code->size(); instruction_pointer++) {
                    switch((*code)[instruction_pointer]) {
                        case '[':
                            loop_depth++;
                            break;
                        case ']':
                            if(loop_depth == 0)
                                return;
                            else
                                loop_depth--;
                            break;
                    }
                }
            }
};

#endif