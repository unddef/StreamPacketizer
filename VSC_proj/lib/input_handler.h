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

const int MAX_COMPORT_READ_BLOCK_LENGTH = 1000;
const int MAX_IPPORT_READ_BLOCK_LENGTH = 1000;
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
        uint32_t com_baudrate;
        uint8_t com_stopbit;
        uint8_t com_parity;
        uint64_t bytes_received;
        uint8_t com_configure_port();
        uint8_t com_open_port();
        uint8_t com_read_bytes();
        uint8_t ip_open_socket();
        uint8_t ip_read_bytes();
            
    public:
        //bool read_bytes_running;
        Input_Handler(Custom_Debugger*, Buffer_Handler*);
        ~Input_Handler();
        uint8_t open_input_stream(std::string);
        uint8_t read_bytes();
        uint8_t close_input_stream();
        uint8_t com_configure_baudrate(uint32_t);
        uint8_t com_configure_stopbit(uint8_t);
        uint8_t com_configure_parity(uint8_t);
        uint64_t get_bytes_received();
        uint8_t add_bytes_received(uint32_t);
        uint8_t reset_bytes_received();

        enum class enumInputStreamType {
            COM_PORT,
            IP_PORT,
            FILE_PATH,
            UNKNOWN
        };

        enumInputStreamType input_type;

};

#endif