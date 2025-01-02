#include "Input_Handler.h"


#include <vector>
#include <iostream>
#include <stdint.h>
#include <string>
#include <Windows.h>


Input_Handler::Input_Handler(Custom_Debugger* ext_debug_handler, Buffer_Handler* used_buffer){
    ptrDebug = ext_debug_handler;
    ptrStreamBuffer = used_buffer;
};


uint8_t Input_Handler::com_configure_port(){
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);
    if (!GetCommState(h_Serial, &dcbSerialParameters)) {
        //ptrDebug->debug(1, "error while getting COM state");
        return(0);    
    }
    dcbSerialParameters.BaudRate = CBR_9600;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = EVENPARITY;
    //set configuration active
    if (!SetCommState(h_Serial, &dcbSerialParameters)) {
        //ptrDebug->debug(1, "error while setting COM state"); 
        return(0);
    }
    //set read timeouts
    COMMTIMEOUTS timeout = {0};
    timeout.ReadIntervalTimeout = 60;               //specifies the time that must pass between receiving characters before timing out (in milliseconds)
    timeout.ReadTotalTimeoutConstant = 60;          //provides the amount of time to wait before returning
    timeout.ReadTotalTimeoutMultiplier = 15;
    timeout.WriteTotalTimeoutConstant = 60;
    timeout.WriteTotalTimeoutMultiplier = 8;
    if (!SetCommTimeouts(h_Serial, &timeout)) {
        //ptrDebug->debug(1, "error while setting COM timeouts"); 
        return(0);
    };
    return(1);
};

uint8_t Input_Handler::com_open_port(){
    //string comID = "COM3";
    h_Serial = CreateFileA(comID.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);     //open port
    if (h_Serial == INVALID_HANDLE_VALUE) { //check port open operation
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // serial port not found. Handle error here.
            ptrDebug->debug(1,"could not open COM port. Port not found!");
            return(0);
        } else {
            ptrDebug->debug(1, "unknown error while opening COM port. Error code: ",0);
            ptrDebug->debug(1, GetLastError(),true,0);
            return(0);
        };
        // any other error. Handle error here.
    }
    ptrDebug->debug(3,"COM port opened sucessfully");
    return(1);
};


uint8_t Input_Handler::open_input_stream(std::string path,bool is_com_port = false){
    if(is_com_port){
        comID = path;
        ptrDebug->debug(3,"opening com port: ",false);
        ptrDebug->debug(3,comID,true,false);
        if(com_open_port()) {
            return(1);
            ptrDebug->debug(3,"com port open successful",false);
        }else{
            ptrDebug->debug(3,"com port open failed!",false);
            return(0);
        };
        
    }else{
        return(0);
    }
};

uint8_t Input_Handler::read_bytes(){
    
    DWORD errors;
    COMSTAT status;
    if (ClearCommError(h_Serial, &errors, &status)) {
        ptrDebug->debug(4,"Input_Handler : Bytes in COM buffer: ",false);
        ptrDebug->debug(4,status.cbInQue,false,false);
        /*if(status.cbInQue > 0){
        ptrDebug->debug(4,"  -- ",false,false);
        }else{
            ptrDebug->debug(4," ",true,false);
        }*/
    } else {
        ptrDebug->debug(1,"Input_Handler : Error fetching COM Status. Error code: ",false);
        ptrDebug->debug(1,GetLastError(),true,false);
    }
    
    //determine how many bytes to read
    DWORD bytesToRead = 0;
    ptrDebug->debug(4,"  /  free space in local buffer: ",false,false);
    ptrDebug->debug(4,ptrStreamBuffer->buffer_free_bytes_left(),false,false);
    if(ptrStreamBuffer->buffer_free_bytes_left() >= status.cbInQue ){
        //buffer has enough space to fit all com_buffer data
        if(status.cbInQue <= MAX_COMPORT_READ_BLOCK_LENGTH) {
            //que data smaller than blocksize
            bytesToRead = status.cbInQue;
        }else{
            //blocked com_buffer read
            bytesToRead = MAX_COMPORT_READ_BLOCK_LENGTH;
        }
    }else{
        //buffer almost full. reading only as many bytes from source as bytes free in buffer
        bytesToRead = ptrStreamBuffer->buffer_free_bytes_left();
    }
    ptrDebug->debug(4,"  /  ",false,false);
    ptrDebug->debug(4,bytesToRead,false, false);
    ptrDebug->debug(4," Bytes to read from COM buffer  /  ",false,false);
    
    //read bytes
    //char sBuff[300] = {0};
    char* ptrCharBuffer = new char[bytesToRead];
    DWORD dwRead = 0;
    if (!ReadFile(h_Serial, ptrCharBuffer, bytesToRead, &dwRead, NULL)) {
        ptrDebug->debug(1,"Input_Handler : error reading data from COM port. aborting...");
        delete ptrCharBuffer;
        return(0);
    }
    ptrDebug->debug(4, dwRead,false,false);
    ptrDebug->debug(4," bytes processed ", true, false);
      
    //process bytes
    /*for (int i=0;i<dwRead;i++){
        printf("%x ",ptrBuffer[i]);
    }*/
    //ptrDebug->debug(1,"",true,false);
    if(dwRead > 0) ptrStreamBuffer->add_data(ptrCharBuffer,bytesToRead);

    delete ptrCharBuffer;
    return(1);
};

uint8_t Input_Handler::close_input_stream(){
    CloseHandle(h_Serial);
    return(1);
};
 

