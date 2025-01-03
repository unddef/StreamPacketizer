
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
#include "./lib/output_handler.h"

const DWORD MAIN_BUFFER_SIZE = 330;
const DWORD MAX_TELEGRAM_LENGTH = 255;

//std::string inputStreamPath = "COM4";
std::string inputStreamPath = "127.0.0.1:8889";

std::string outputStreamPath = "127.0.0.1:8889";

Custom_Debugger debug(4);
Output_Handler  outputStream(&debug);
Pcap_Handler     pcapFile(&debug,&outputStream);
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
    //outputStream.open_output_stream(Output_Handler::enumOutputStreamType::FILE, "./test2.pcap");
    outputStream.open_output_stream(Output_Handler::enumOutputStreamType::STDOUT);
    //pcapFile.open_file("./test.pcap");
    pcapFile.write_file_header();

    //openinput source
    inputStream.open_input_stream(inputStreamPath);

    while(loopingEnabled){
        
        inputStream.read_bytes();
        streamBuffer.dump_buffer_to_debug();
        iecPacketizer.process_buffer();
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    inputStream.close_input_stream();
    outputStream.close_output_stream();
}