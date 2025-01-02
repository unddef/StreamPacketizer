#include "Input_Handler.h"


#include <vector>
#include <iostream>
#include <stdint.h>
#include <string>
#include <Windows.h>


Input_Handler::Input_Handler(Custom_Debugger* ext_debug_handler, Buffer_Handler* used_buffer){
    ptrDebug = ext_debug_handler;
    ptrStreamBuffer = used_buffer;
    input_type = enumInputStreamType::UNKNOWN;
};

Input_Handler::~Input_Handler(){
    close_input_stream();
};

uint8_t Input_Handler::com_configure_port(){
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);
    if (!GetCommState(h_Serial, &dcbSerialParameters)) {
        ptrDebug->debug(1, "error while getting COM state");
        return(1);    
    }
    dcbSerialParameters.BaudRate = CBR_9600;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = EVENPARITY;
    //set configuration active
    if (!SetCommState(h_Serial, &dcbSerialParameters)) {
        ptrDebug->debug(1, "error while setting COM state"); 
        return(1);
    }
    //set read timeouts
    COMMTIMEOUTS timeout = {0};
    timeout.ReadIntervalTimeout = 60;               //specifies the time that must pass between receiving characters before timing out (in milliseconds)
    timeout.ReadTotalTimeoutConstant = 60;          //provides the amount of time to wait before returning
    timeout.ReadTotalTimeoutMultiplier = 15;
    timeout.WriteTotalTimeoutConstant = 60;
    timeout.WriteTotalTimeoutMultiplier = 8;
    if (!SetCommTimeouts(h_Serial, &timeout)) {
        ptrDebug->debug(1, "error while setting COM timeouts"); 
        return(1);
    };
    return(0);
};

uint8_t Input_Handler::com_open_port(){
    h_Serial = CreateFileA(input_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);     //open port
    if (h_Serial == INVALID_HANDLE_VALUE) { //check port open operation
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // serial port not found. Handle error here.
            ptrDebug->debug(1,"could not open COM port. Port not found!");
            return(1);
        } else {
            ptrDebug->debug(1, "unknown error while opening COM port. Error code: ",0);
            ptrDebug->debug(1, GetLastError(),true,0);
            return(1);
        };
        // any other error. Handle error here.
    }
    ptrDebug->debug(3,"COM port opened sucessfully");
    return(0);
};

uint8_t Input_Handler::com_read_bytes(){
    
    DWORD errors;
    COMSTAT status;
    if (ClearCommError(h_Serial, &errors, &status)) {
        ptrDebug->debug(4,"Input_Handler : Bytes in COM buffer: ",false);
        ptrDebug->debug(4,status.cbInQue,false,false);
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
        return(1);
    }
    ptrDebug->debug(4, dwRead,false,false);
    ptrDebug->debug(4," bytes processed ", true, false);
    
    if(dwRead > 0) ptrStreamBuffer->add_data(ptrCharBuffer,bytesToRead);

    delete ptrCharBuffer;
    return(0);
};

uint8_t Input_Handler::ip_open_socket(){
    ptrDebug->debug(2,"open_input_stream: opening TCP socket to "+tcp_input_ip+":",false);
    ptrDebug->debug(2,tcp_input_port,true,false);

    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    //Did something happen?
    if (error) {
        ptrDebug->debug(1,"ip_open_socket: error at WSAStartup");
        return(1);
    }
    //Did we get the right Winsock version?
    if(wsadata.wVersion != 0x0202){
        WSACleanup(); //Clean up Winsock
        ptrDebug->debug(1,"ip_open_socket: error - wrong version");
        return(1);
    }
    //Fill out the information needed to initialize a socket�
    SOCKADDR_IN target; //Socket address information
    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons (4444); //Port to connect on
    target.sin_addr.s_addr = inet_addr ("198.18.2.5"); //Target IP
    h_tcpSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (h_tcpSocket == INVALID_SOCKET){
        ptrDebug->debug(1,"ip_open_socket: error creating socket");
        return(1); //Couldn't create the socket
    }  

    //Try connecting...
    if (connect(h_tcpSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR){
        ptrDebug->debug(2,"ip_open_socket: could not connect to IP");
        exit(1);
        return(1); //Couldn't connect
    }else{
        ptrDebug->debug(2,"ip_open_socket: TCP connection established");
        return(0); //Success
    }

    //cleanup
    

};

uint8_t Input_Handler::ip_read_bytes(){
     ptrDebug->debug(4,"ip_read_bytes: reading");
    return(0);
}

uint8_t Input_Handler::open_input_stream(std::string path){
    
    //separate path string and check what input type to use
    //R"(COM\d+)"
    std::regex com_regex(R"((COM)(\d+))");                              // Regular expression for COM port (e.g., "COM10")
    std::regex ip_port_regex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))");       // Regular expression for IP:port (e.g., "192.168.1.1:8080")                      
    std::smatch match;
    
    if (std::regex_match(path, match, com_regex)) {                // path matches the COM regex
        if (match.size() == 3) { // match[0] is the whole match, match[1] is "COM", match[2] is the number
            input_path = "\\\\.\\COM" + match[2].str();
            ptrDebug->debug(3,"open_input_stream: opening com port: ",false);
            ptrDebug->debug(3,input_path,true,false);
            if(com_open_port() == 0) {
                ptrDebug->debug(2,"open_input_stream: successfully opened COM port ",false);
                ptrDebug->debug(2,input_path,true,false);
                input_type = enumInputStreamType::COM_PORT;

                return(0);
            }else{
                ptrDebug->debug(1,"open_input_stream: failed to open COM port ",false);
                ptrDebug->debug(1,input_path,true,false);
                input_type = enumInputStreamType::UNKNOWN;
                return(1);
            };
        }
        return(1);
    }
    
    if (std::regex_match(path, match, ip_port_regex)) {            // path matches the IP:port regex
        if (match.size() == 3) { // match[0] is the whole match, match[1] is the IP, match[2] is the port
            ptrDebug->debug(3,"open_input_stream: IP:PORT regex match");
            tcp_input_ip = match[1].str();
            tcp_input_port = stoi(match[2].str());
            input_type = enumInputStreamType::IP_PORT;
            //ptrDebug->debug(2,"open_input_stream: opening IP input stream from "+tcp_input_ip+":",false);
            //ptrDebug->debug(2,tcp_input_port,true,false);
            ip_open_socket();
            return(0);
        }
        
        return(1);
    }

    ptrDebug->debug(3,"open_input_stream: unkown input type specified. cant open!");
    return(1);
};

uint8_t Input_Handler::read_bytes(){
    switch(input_type){
        case enumInputStreamType::COM_PORT:
        com_read_bytes();
        return(0);
        break;

        case enumInputStreamType::IP_PORT:
        ip_read_bytes();
        return(0);
        break;

        default:
        return(1);
    }
    return(1);
}

uint8_t Input_Handler::close_input_stream(){
    if(input_type == enumInputStreamType::COM_PORT){
        CloseHandle(h_Serial);
        input_type = enumInputStreamType::UNKNOWN;
        return(0);
    }
    //cleanup TCP session
    if (h_tcpSocket) closesocket(h_tcpSocket);
    WSACleanup(); //Clean up Winsock
    return(1);
};
 
//heper functions

