#ifndef Input_Handler_H
#define Input_Handler_H
#include <stdint.h>
#include <string>
#include <windows.h>
#include <regex>
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib") // Link Winsock library
#include "./debug.h"
#include "buffer_handler.h"

const int MAX_COMPORT_READ_BLOCK_LENGTH = 15;
const int MAX_IPPORT_READ_BLOCK_LENGTH = 100;
//using namespace std;

class Input_Handler {
    private:
        Custom_Debugger* ptrDebug;
        Buffer_Handler* ptrStreamBuffer;
        std::string input_path;
        std::string tcp_input_ip = ""; 
        uint16_t tcp_input_port = 0;
        DCB dcbSerialParameters = {0};
        HANDLE h_Serial;
        SOCKET h_tcpSocket;
        uint8_t com_configure_port();
        uint8_t com_open_port();
        uint8_t com_read_bytes();
        uint8_t ip_open_socket();
        uint8_t ip_read_bytes();
            
    public:
        Input_Handler(Custom_Debugger*, Buffer_Handler*);
        ~Input_Handler();
        uint8_t open_input_stream(std::string);
        uint8_t read_bytes();
        uint8_t close_input_stream();

        enum class enumInputStreamType {
            COM_PORT,
            IP_PORT,
            FILE_PATH,
            UNKNOWN
        };

        enumInputStreamType input_type;

};

#endif