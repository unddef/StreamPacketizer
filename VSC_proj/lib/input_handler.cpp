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
uint8_t Input_Handler::com_configure_baudrate(uint32_t new_baudrate){
    //check baudrate
    if (new_baudrate == 110 || new_baudrate == 300 || new_baudrate == 600 || new_baudrate == 1200 || new_baudrate == 2400 || new_baudrate == 4800 || new_baudrate == 9600 || new_baudrate == 14400 || new_baudrate == 19200 || new_baudrate == 38400 || new_baudrate == 56000 || new_baudrate == 57600 || new_baudrate == 115200 || new_baudrate == 128000 ||new_baudrate == 256000){
        com_baudrate = new_baudrate;
    }else {
        ptrDebug->debug(1,"not supported baudrate. exiting");
        exit(1);
    };
    return(0);
}

uint8_t Input_Handler::com_configure_stopbit(uint8_t new_stopbit){
    //check stopbit
    if ( new_stopbit == 1 ){
        com_stopbit = ONESTOPBIT;
    }else if( new_stopbit == 2){
        com_stopbit = TWOSTOPBITS;
    }else {
        ptrDebug->debug(1,"not supported stopbit setting. allowed <1-2>. exiting");
        exit(1);
    };
};

uint8_t Input_Handler::com_configure_parity(uint8_t new_parity){
    //check parity
    if ( new_parity == 0 ){
        com_parity = NOPARITY;
    }else if( new_parity == 1){
        com_parity = ODDPARITY;
    }else if( new_parity == 2){
        com_parity = EVENPARITY;
    }else {
        ptrDebug->debug(1,"not supported parity setting. allowed <0=none/1=odd/2=even>. exiting");
        exit(1);
    };
}


uint8_t Input_Handler::com_configure_port(){
    //ptrDebug->debug(4, "input_handler: configure COM port"); 
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);
    //get com state
    if (!GetCommState(h_Serial, &dcbSerialParameters)) {
        ptrDebug->debug(1, "input_handler: error while getting COM state");
        exit(1);    
    }
    //default settings
    dcbSerialParameters.BaudRate = com_baudrate;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = com_stopbit;
    dcbSerialParameters.Parity = com_parity;

    //set configuration active
    if (!SetCommState(h_Serial, &dcbSerialParameters)) {
        ptrDebug->debug(1, "input_handler: error while setting COM state"); 
        exit(1);
    }
    //set read timeouts
    COMMTIMEOUTS timeout = {0};
    timeout.ReadIntervalTimeout = 60;               //specifies the time that must pass between receiving characters before timing out (in milliseconds)
    timeout.ReadTotalTimeoutConstant = 60;          //provides the amount of time to wait before returning
    timeout.ReadTotalTimeoutMultiplier = 15;
    timeout.WriteTotalTimeoutConstant = 60;
    timeout.WriteTotalTimeoutMultiplier = 8;
    if (!SetCommTimeouts(h_Serial, &timeout)) {
        ptrDebug->debug(1, "input_handler: error while setting COM timeouts"); 
        exit(1);
    };
    return(0);
};

uint8_t Input_Handler::com_open_port(){
    h_Serial = CreateFileA(input_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);     //open port
    if (h_Serial == INVALID_HANDLE_VALUE) { //check port open operation
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // serial port not found. Handle error here.
            ptrDebug->debug(1,"input_handler: could not open COM port. Port not found!");
            exit(1);
        } else {
            ptrDebug->debug(1, "input_handler: unknown error while opening COM port. Error code: ",0);
            ptrDebug->debug(1, GetLastError(),true,0);
            exit(1);
        };
        // any other error. Handle error here.
    }
    ptrDebug->debug(2,"input_handler: COM port opened sucessfully");
    com_configure_port();
    return(0);
};

uint8_t Input_Handler::com_read_bytes(){
    
    DWORD errors;
    COMSTAT status;
    if (ClearCommError(h_Serial, &errors, &status)) {
        ptrDebug->debug(4,"input_Handler: Bytes in COM buffer: ",false);
        ptrDebug->debug(4,status.cbInQue,false,false);
    } else {
        ptrDebug->debug(1,"input_Handler: Error fetching COM Status. Error code: ",false);
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
        ptrDebug->debug(1,"input_Handler: error reading data from COM port. aborting...");
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
    ptrDebug->debug(3,"ip_open_socket: opening TCP socket to "+tcp_input_ip+":",false);
    ptrDebug->debug(3,tcp_input_port,true,false);

    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    //Did something happen?
    if (error) {
        ptrDebug->debug(1,"ip_open_socket: error at WSAStartup");
        exit(1);
    }
    //Did we get the right Winsock version?
    if(wsadata.wVersion != 0x0202){
        WSACleanup(); //Clean up Winsock
        ptrDebug->debug(1,"ip_open_socket: error - wrong version");
        exit(1);
    }
    //Fill out the information needed to initialize a socket
    SOCKADDR_IN dstAddr; //Socket address information
    dstAddr.sin_family = AF_INET; // address family Internet
    dstAddr.sin_port = htons (tcp_input_port); //Port to connect on
    dstAddr.sin_addr.s_addr = inet_addr (tcp_input_ip.c_str()); //Target IP
    h_tcpSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (h_tcpSocket == INVALID_SOCKET){
        ptrDebug->debug(1,"ip_open_socket: error creating socket. ErrorID: ",false);
        ptrDebug->debug(1,WSAGetLastError(),true,false);
        exit(1); //Couldn't create the socket
    }  

    //Try connecting...
    if (connect(h_tcpSocket, (SOCKADDR *)&dstAddr, sizeof(dstAddr)) == SOCKET_ERROR){
        ptrDebug->debug(1,"ip_open_socket: could not connect to IP. Error code: ",false);
        ptrDebug->debug(1,WSAGetLastError(),true,false);
        exit(1); //Couldn't connect
    }
    
    //connect should have worked. check again
    ptrDebug->debug(2,"ip_open_socket: TCP connection established to "+tcp_input_ip+":",false);
    ptrDebug->debug(2,tcp_input_port,true,false);
    input_type = enumInputStreamType::IP_PORT;
    return(0); //Success

};

uint8_t Input_Handler::ip_read_bytes(){
    //find out how many bytes in tcp buffer
    u_long bytesAvailable;
    int call_result = ioctlsocket(h_tcpSocket, FIONREAD, &bytesAvailable);     
    if (call_result == SOCKET_ERROR) {
        ptrDebug->debug(1,"ip_read_bytes: error reading size of TCP buffer. Error code: ",false);
        ptrDebug->debug(1,WSAGetLastError(),true,false);
        return(1);
    };

    if(bytesAvailable <= 0){
        ptrDebug->debug(4,"ip_read_bytes: no bytes in TCP buffer.");
        return(0);
    };


    //determine how many bytes to read. if charBuffer has enough room -> read MAX_IPPORT_READ_BLOCK_LENGTH. 
    //else read as many bytes as fit in chrBuffer
    DWORD bytesToRead = 0;
    if(ptrStreamBuffer->buffer_free_bytes_left() >= bytesAvailable ){
        //buffer has enough space to fit all tcp_buffer data
        if(bytesAvailable <= MAX_IPPORT_READ_BLOCK_LENGTH) {
            //que data smaller than blocksize
            bytesToRead = bytesAvailable;
        }else{
            //blocked tcp_buffer read
            bytesToRead = MAX_IPPORT_READ_BLOCK_LENGTH;
        }
    }else{
        //buffer almost full. reading only as many bytes from source as bytes free in buffer
        bytesToRead = ptrStreamBuffer->buffer_free_bytes_left();
    }

    ptrDebug->debug(3,"ip_read_bytes: " + std::to_string(bytesAvailable) + " bytes in TCP buffer / " + std::to_string(ptrStreamBuffer->buffer_free_bytes_left()) + " bytes free in localBuffer / " + std::to_string(bytesToRead) + " bytes to process");
    //ptrDebug->debug(3," bytes free in localBuffer: ",false,false);
    //ptrDebug->debug(3,ptrStreamBuffer->buffer_free_bytes_left(),false,false);
    //ptrDebug->debug(3," bytes to process: ",false,false);
    //ptrDebug->debug(3,bytesToRead,true,false);

    //const int bufferSize = 1024;
    char* tmpBuf = new char[bytesToRead];
    int bytesReceived = recv(h_tcpSocket, tmpBuf, bytesToRead, 0);
    
    if (bytesReceived < 0) {
        ptrDebug->debug(1,"ip_read_bytes: error reading bytes from TCP socket. Error code: ",false);
        ptrDebug->debug(1,WSAGetLastError(),true,false);
        delete[] tmpBuf;
        return(1);
    } else {
        ptrDebug->debug(4,"ip_read_bytes: bytes received: "  + std::to_string(bytesReceived));
        //ptrDebug->debug(4,bytesReceived,true,false);
        //std::cout << "Received data: " << tmpBuf << std::endl;
        ptrStreamBuffer->add_data(tmpBuf,bytesToRead);
    }
    delete[] tmpBuf;
    return(0);
};

uint8_t Input_Handler::open_input_stream(std::string path){
    
    //separate path string and check what input type to use
    std::regex com_regex(R"((COM)(\d+))");                              // Regular expression for COM port (e.g., "COM10")
    std::regex ip_port_regex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))");       // Regular expression for IP:port (e.g., "192.168.1.1:8080")                      
    std::smatch match;
    
    if (std::regex_match(path, match, com_regex)) {                // path matches the COM regex
        if (match.size() == 3) { // match[0] is the whole match, match[1] is "COM", match[2] is the number
            input_path = "\\\\.\\COM" + match[2].str();
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
            //ptrDebug->debug(3,"open_input_stream: IP:PORT regex match");
            tcp_input_ip = match[1].str();
            tcp_input_port = stoi(match[2].str());
            input_type = enumInputStreamType::IP_PORT;
            ip_open_socket();
            return(0);
        }
        
        return(1);
    }

    ptrDebug->debug(1,"open_input_stream: unkown input type specified. cant open! exiting");
    exit(1);
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
        ptrDebug->debug(1,"no input stream open. exiting program.");
        exit(1);
    }
    return(1);
}

uint8_t Input_Handler::close_input_stream(){
    //close file handler
    if(h_Serial) CloseHandle(h_Serial);
    //close  TCP session
    if (h_tcpSocket) closesocket(h_tcpSocket);
    WSACleanup(); //Clean up Winsock
    input_type = enumInputStreamType::UNKNOWN;
    return(1);
};
 


