
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


/*** TODOs****
- buffer klasse kann voll laufen. Wenn packetizer durchgelaufen, er aber keine daten wegschreibt (also kein frame gefunden hat) und der puffer voll ist, soll alles wegeschrieben werdne.
  alternativ: wenn buffer voller als max_telegram_size

- fehlgeschlagener tcp verbindungsaufbau wird nicht erkannt. sollte das programm beenden
//********/

const DWORD MAIN_BUFFER_SIZE = 330;
const DWORD MAX_TELEGRAM_LENGTH = 255;

//std::string inputStreamPath = "COM4";
std::string inputStreamPath = "127.0.0.1:8889";

Output_Handler::enumOutputStreamType output_type = Output_Handler::enumOutputStreamType::STDOUT;
//Output_Handler::enumOutputStreamType output_type = Output_Handler::enumOutputStreamType::FILE;
std::string outputStreamPath = "./test_output.pcap";

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
    outputStream.open_output_stream(output_type, outputStreamPath);
    pcapFile.write_file_header();

    //openinput source
    inputStream.open_input_stream(inputStreamPath);

    while(loopingEnabled){
        
        inputStream.read_bytes();
        streamBuffer.dump_buffer_to_debug();
        iecPacketizer.process_buffer();
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    debug.debug(1,"closing handlers");
    inputStream.close_input_stream();
    outputStream.close_output_stream();
    debug.debug(1,"program end");
}