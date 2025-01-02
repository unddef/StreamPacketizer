#include "buffer_handler.h"

Buffer_Handler::Buffer_Handler(DWORD buffer_size, Custom_Debugger* ext_debugger){
    ptrDebug = ext_debugger;
    bufferSize = buffer_size;
    chrBuffer = new char[bufferSize];
}

Buffer_Handler::~Buffer_Handler(){
    delete[] chrBuffer;
};

DWORD Buffer_Handler::get_buffer_size(void){
    return(bufferSize);
};
DWORD Buffer_Handler::buffer_bytes_filled(void){
    return(bufferFilled);
};

DWORD Buffer_Handler::buffer_free_bytes_left(void){
    return(bufferSize-bufferFilled);
};

DWORD Buffer_Handler::get_scanned(void){
    return(bufferScanned);
};

bool Buffer_Handler::set_scanned(DWORD value_to_set){
    if(value_to_set < 0 || value_to_set > (get_buffer_size() - 1) ){
        ptrDebug->debug(1,"Buffer_Handler: set_scanned(): illegal value. aborting...");
        return(0);
    } else{
    bufferScanned = value_to_set;
    return(true);
    }
};



uint8_t Buffer_Handler::add_data(char* data_to_add, DWORD data_length){
    //log data
    char* dataString = new char[data_length * 3 + 1];
    for (size_t i = 0; i < data_length; ++i) {
        sprintf(dataString + i * 3, "%02X ", static_cast<unsigned char>(data_to_add[i]));
    }
    ptrDebug->debug(4,"Buffer_Handler : data added - RAW DATA: ",false);
    ptrDebug->debug(4,dataString,true, false);

    if(buffer_free_bytes_left() - data_length >= 0){
        ///hier add data to buffer. todo!!
        std::memcpy(chrBuffer + bufferFilled,data_to_add,data_length);  //copy data from incoming char* to internal buffer
        bufferFilled = bufferFilled + data_length;
    }else{
        ptrDebug->debug(1,"Buffer_Handler : error adding data. Buffer is full! Lost data: ", false);
        ptrDebug->debug(1,dataString,true,false);
        return(0);
    }
    delete dataString;
    return(1);
};

char* Buffer_Handler::get_data(DWORD start_position, DWORD data_len){
    if(data_len <= 0) return(nullptr);
    //check if accessed data address is valid
    if (start_position >= (get_buffer_size() - 1) || start_position + data_len > (get_buffer_size() ) ) {           //-1 for offsetting array access starting at 0
        ptrDebug->debug(1,"Buffer_Handler / get_data(): access to illegal memory address!");
        //ptrDebug->debug(1,start_position,false,false);
        //ptrDebug->debug(1," data_length: ",false,false);
        //ptrDebug->debug(1,data_len,true,false);
        return nullptr;
    }
    //allocate memory and copy buffer data
    char* returnData = new char[data_len]; 
    std::memcpy(returnData, chrBuffer + start_position, data_len);
    return returnData;
};


void Buffer_Handler::dump_buffer_to_debug(){
    char* dumpString = new char[buffer_bytes_filled() * 3 + 1];
    for (size_t i = 0; i < buffer_bytes_filled(); ++i) {
        sprintf(dumpString + i * 3, "%02X ", static_cast<unsigned char>(chrBuffer[i]));
    }
    ptrDebug->debug(4,"Buffer_Handler : ",false);
    ptrDebug->debug(4,buffer_bytes_filled(), false, false);
    ptrDebug->debug(4," bytes in buffer: ",false,false);
    if(buffer_bytes_filled()>0){
        ptrDebug->debug(4,dumpString,true, false);
    }else{
        ptrDebug->debug(4,"",true,false);
    };
};

//discard data from the buffer. starts at buffer[0] and deletes length bytes. afterwards moves remaining data to the front
bool Buffer_Handler::discard_data(DWORD length){
    if(length < 0) return(1);
    if(length == 0) return(0);
    if(length > bufferSize){
        ptrDebug->debug(1,"Buffer_Handler: discard_data(): illegal data address. length to big! aborting");
        return(1);
    };
    memmove(chrBuffer, chrBuffer + length, get_buffer_size() - length);
    bufferFilled = bufferFilled - length;
    return(0);
};

