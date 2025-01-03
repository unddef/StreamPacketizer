#ifndef Output_Handler_H
#define Output_Handler_H
#include <string>
#include "debug.h"

class Output_Handler{
    public:
    enum class enumOutputStreamType {
        UNKNOWN,
        FILE_PATH,
        STDOUT
    };

    Output_Handler();
    ~Output_Handler();

    uint8_t set_output_type(enumOutputStreamType);
    uint8_t set_output_path(std::string);
    uint8_t open_output_stream();
    uint8_t write_data(char*, uint16_t);
    uint8_t close_output_stream();

    private:
    enumOutputStreamType currentInputType = enumOutputStreamType::UNKNOWN;
    std::string currentOutputPath = "";
    
};

#endif