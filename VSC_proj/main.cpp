
//#include <iostream>
//#include <vector>
#include <csignal>
#include <string>
#include <chrono>
#include <thread>
#include <stdint.h>


#include "./lib/debug.h"
#include "./lib/input_handler.h"
#include "./lib/PcapHandler.h"
#include "./lib/Packetizer_101.h"

const DWORD MAIN_BUFFER_SIZE = 330;
const DWORD MAX_TELEGRAM_LENGTH = 255;

//std::string inputStreamPath = "COM4";
std::string inputStreamPath = "198.18.2.5:4444";

Custom_Debugger debug(4);
PcapHandler     pcapFile(&debug);
Buffer_Handler  streamBuffer(MAIN_BUFFER_SIZE,&debug);
Input_Handler   inputStream(&debug,&streamBuffer);
Packetizer_101  iecPacketizer(&debug,&streamBuffer,&pcapFile,1,2,3);


//handle ctrl+c and other external signals
std::atomic<bool> loopingEnabled(true);
void signalHandler(int signum){
    if(signum == 2){
        debug.debug(2,"handling SIG=2. closing program...");
        loopingEnabled = false;
    }
};


uint8_t main(){
    debug.debug(1,"program start");
    //register signal handler for ctrl+c abortion
    std::signal(SIGINT, signalHandler);
    
    //open pcap file
    pcapFile.open_file("./test.pcap");
    pcapFile.write_file_header();

    //open com port
    inputStream.open_input_stream(inputStreamPath);

    while(loopingEnabled){
        
        inputStream.read_bytes();
        streamBuffer.dump_buffer_to_debug();
        iecPacketizer.process_buffer();
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    inputStream.close_input_stream();
    pcapFile.close_file();
}