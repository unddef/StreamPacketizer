#ifndef Output_Handler_H
#define Output_Handler_H
#include <string>
#include <fstream>
#include <fcntl.h>
#include <io.h>
//#include <stdio.h>

#include "debug.h"

class Output_Handler{
    public:
    enum class enumOutputStreamType {
        UNKNOWN,
        FILE,
        STDOUT
    };

    Output_Handler(Custom_Debugger*);
    ~Output_Handler();

    uint8_t open_output_stream(enumOutputStreamType output_type, std::string filepath = "./default_output_file.pcap");
    uint8_t write_data(const char*, uint16_t);
    uint8_t close_output_stream();

    private:
    Custom_Debugger* ptrDebug;
    std::ofstream FileHandler;      //file handler
    enumOutputStreamType currentOutputType = enumOutputStreamType::UNKNOWN;
    std::string currentOutputPath = "";

};

#endif