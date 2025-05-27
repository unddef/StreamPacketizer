#ifndef CUSTOM_DEBUGGER_H
#define CUSTOM_DEBUGGER_H
#include <ctime>
#include <iomanip>
#include <iostream>
#include <chrono>

///////////////////
//Debug section
///////////////////
//es gibt Debuglevel:
//  0:  keinerlei Ausgabe
//  1:  nur die wichtigsten Meldungen/Fehler
//  2:  etwas mehr Infos
//  3:  alles(ohne repetitive task debug)
//  4:  wirklich alles (mit repetitive task debug - viel!)
//dies ist eine template funktionen, damit die funktion mit verschiedenen parameter typen aufgerufen werden kann

class Custom_Debugger {
    private:
        int current_debuglevel;
        //time_t last_msg;

    public:
        Custom_Debugger();
        Custom_Debugger(uint8_t);
        void set_debug_level(uint8_t);
        uint8_t get_debug_level();
    
        

        template <typename any2> void debug( uint8_t debug_level, any2 msg, bool new_line = true, uint8_t timestamp = 1);
        
};

template <typename any2> void Custom_Debugger::debug( uint8_t debug_level, any2 msg, bool new_line, uint8_t timestamp ){
    if(current_debuglevel >= debug_level){
        if(timestamp != 0){
            //std::time_t now = std::time(nullptr);
            //const std::tm time = *std::localtime(std::addressof(now));
            auto now = std::chrono::system_clock::now();    //get "now" timestamp
            std::time_t now_c = std::chrono::system_clock::to_time_t(now); //convert to time_t
            std::tm* local_time = std::localtime(&now_c);  //convert to local time
             auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;  //calculate milliseconds
            std::cerr << "[" << std::setfill('0') << std::setw(2) << local_time->tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time->tm_min << ":" << std::setfill('0') << std::setw(2) << local_time->tm_sec << ":" << std::setfill('0') << std::setw(3) << milliseconds << "] : ";
        }
        if(new_line){
        std::cerr << msg << std::endl;
        } else {
            std::cerr << msg;
        };
    };
};

#endif;