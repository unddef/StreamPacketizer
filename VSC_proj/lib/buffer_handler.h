#ifndef Buffer_Handler_H
#define Buffer_Handler_H
#include <stdint.h>
#include <string>
#include <windows.h>

#include "debug.h"
#include "buffer_handler.h"

class Buffer_Handler {
    private:
    Custom_Debugger* ptrDebug;
    char* chrBuffer;
    DWORD bufferSize = 0;               //size of the internal buffer array
    DWORD bufferFilled = 0;             //nr of used array elements. (points to first free element)
    DWORD bufferScanned = 0;            //nr already scanned bytes (points to first unscanned byte)
    
    public:
    Buffer_Handler(DWORD, Custom_Debugger*);
    ~Buffer_Handler();

    DWORD get_buffer_size();
    DWORD buffer_bytes_filled(void);
    DWORD buffer_free_bytes_left(void);
    DWORD get_scanned(void);
    bool set_scanned(DWORD);
    uint8_t add_data(char*, DWORD);
    //get data from buffer.
    char* get_data(DWORD,DWORD);
    bool discard_data(DWORD);
    void dump_buffer_to_debug();
};
#endif