#include "debug.h"

Custom_Debugger::Custom_Debugger(){
    current_debuglevel = 3;
};
Custom_Debugger::Custom_Debugger(uint8_t debug_level2set){
    current_debuglevel = debug_level2set;
};
void Custom_Debugger::set_debug_level(uint8_t new_debug_level){
    if ( (new_debug_level >= 0) && (new_debug_level <= 4) ){
        current_debuglevel = new_debug_level;
    }
};

uint8_t Custom_Debugger::get_debug_level(){
    return current_debuglevel;
};
