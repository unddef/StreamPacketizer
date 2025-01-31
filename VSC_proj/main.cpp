
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
- fehlgeschlagener tcp verbindungsaufbau wird nicht erkannt. sollte das programm beenden
- abbruch der tcp verbindung sollte erkannt werden -> exit
- variablen:  link_addr_len, ASDU_addr_len, IOA_len in konstanten/als parameter

- file input einbauen?
- kurzquittierung einbauen?
//********/

const DWORD MAIN_BUFFER_SIZE = 100;
const uint16_t MAX_TELEGRAM_LENGTH = 255;
const int defaultDebugLevel = 2;

std::string inputStreamPath = "COM1"; //default value.
//std::string inputStreamPath = "127.0.0.1:8889";

std::string outputStreamPath = "./test_output.pcap";
//std::string outputStreamPath = "-";

Custom_Debugger debug(defaultDebugLevel);
Output_Handler  outputStream(&debug);
Pcap_Handler    pcapFile(&debug,&outputStream);
Buffer_Handler  streamBuffer(MAIN_BUFFER_SIZE,&debug);
Input_Handler   inputStream(&debug,&streamBuffer);
Packetizer_101  iecPacketizer(&debug,&streamBuffer,&pcapFile,1,2,3, MAX_TELEGRAM_LENGTH);


//handle ctrl+c and other external signals
std::atomic<bool> loopingEnabled(true);
void signalHandler(int signum){
    if(signum == 2){
        debug.debug(2,"handling SIG=2. closing program...");
        loopingEnabled = false;
    }
};


uint8_t main(int cmd_arg_count, char* CMD_arg_value[]){
    debug.debug(1,"program start with debug_level=" + debug.get_debug_level());
    //register signal handler for ctrl+c abortion
    std::signal(SIGINT, signalHandler);
    
    //evaluate commandline options
    for (int i = 1; i < cmd_arg_count; i++) { // Start from 1 to skip the program name
        std::string arg = CMD_arg_value[i];
        //option -i for input interface
        if ( arg == "-i" ) {
            if(i + 1 < cmd_arg_count){
                //setting inputstreampath to string from cmd line argument 
                inputStreamPath = CMD_arg_value[i+1];
            }else {
                debug.debug(1,"not enough arguments specified. exiting");
                exit(1);
            }            
        // option -d for debug  
        } else if (arg == "-d" ) {
            if(i + 1 < cmd_arg_count){
                int new_debug_lebel = std::stoi(CMD_arg_value[i+1]);
                if(new_debug_lebel >= 0 && new_debug_lebel <= 4){
                    debug.debug(3,"set new debug level to ",false);
                    debug.debug(3,new_debug_lebel,true,false);
                    debug.set_debug_level(new_debug_lebel);
                }else{
                    debug.debug(1,"wrong argument for -d  allowed values: 0-4");
                }   
            } else {
                debug.debug(1,"not enough arguments specified. exiting");
                exit(1);
            }
        //option -o for output interface
        }else if ( arg == "-o" ) {
            if(i + 1 < cmd_arg_count){
                //setting inputstreampath to string from cmd line argument 
                outputStreamPath = CMD_arg_value[i+1];
            }else {
                debug.debug(1,"not enough arguments specified. exiting");
                exit(1);
            }
        //option -b for baudrate
        }else if ( arg == "-b" ) {
            if(i + 1 < cmd_arg_count){
                //setting inputstreampath to string from cmd line argument
                uint32_t new_baudrate = std::stoi(CMD_arg_value[i+1]);
                if (new_baudrate == 110 || new_baudrate == 300 || new_baudrate == 600 || new_baudrate == 1200 || new_baudrate == 2400 || new_baudrate == 4800 || new_baudrate == 9600 || new_baudrate == 14400 || new_baudrate == 19200 || new_baudrate == 38400 || new_baudrate == 56000 || new_baudrate == 57600 || new_baudrate == 115200 || new_baudrate == 128000 ||new_baudrate == 256000){

                };
            }else {
                debug.debug(1,"not enough arguments specified. exiting");
                exit(1);
            }
        }
        

    }

    //open pcap file
    outputStream.open_output_stream(outputStreamPath);
    pcapFile.write_file_header();

    //openinput source
    inputStream.open_input_stream(inputStreamPath);

    while(loopingEnabled){
        
        inputStream.read_bytes();
        //streamBuffer.dump_buffer_to_debug();
        iecPacketizer.process_buffer();
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    debug.debug(1,"closing handlers");
    inputStream.close_input_stream();
    outputStream.close_output_stream();
    debug.debug(1,"program end");
}