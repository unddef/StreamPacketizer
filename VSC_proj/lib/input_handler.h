#ifndef Input_Handler_H
#define Input_Handler_H
#include <stdint.h>
#include <string>
#include <windows.h>

#include "./debug.h"
#include "buffer_handler.h"

const int MAX_COMPORT_READ_BLOCK_LENGTH = 15;
//using namespace std;

class Input_Handler {
    private:
        Custom_Debugger* ptrDebug;
        Buffer_Handler* ptrStreamBuffer;
        std::string comID;
        DCB dcbSerialParameters = {0};
        HANDLE h_Serial;
        uint8_t com_configure_port();
        uint8_t com_open_port();
            
    public:
        Input_Handler(Custom_Debugger*, Buffer_Handler*);
        uint8_t open_input_stream(std::string,bool);
        uint8_t read_bytes();
        uint8_t close_input_stream();
};

#endif