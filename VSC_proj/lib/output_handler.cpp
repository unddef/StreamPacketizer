#include "output_handler.h"

Output_Handler::Output_Handler(Custom_Debugger* ext_debug_obj){
    ptrDebug = ext_debug_obj;
};

Output_Handler::~Output_Handler(){
    
}


uint8_t Output_Handler::open_output_stream(enumOutputStreamType output_type , std::string filepath){
    
    //stream output to file
    switch (output_type){
        case enumOutputStreamType::FILE :
            ptrDebug->debug(3,"output_handler: opening output file: " + filepath);
            FileHandler.open(filepath, std::ios::out | std::ios::binary);
            if (!FileHandler.is_open()) {
                ptrDebug->debug(1,"Failed to open file: ", false);
                ptrDebug->debug(1,filepath,true,false);;
                currentOutputType = enumOutputStreamType::UNKNOWN;
                exit(1);
            }else{
                currentOutputType = enumOutputStreamType::FILE;
                currentOutputPath = filepath;
            }

        break;

        case enumOutputStreamType::STDOUT :
            ptrDebug->debug(3,"using STDOUT as output stream");
            currentOutputType = enumOutputStreamType::STDOUT;
            currentOutputPath = "";
        break;
        default:
            ptrDebug->debug(1,"no output stream selected. exiting");
            currentOutputType = enumOutputStreamType::UNKNOWN;
            currentOutputPath = "";
            exit(1);
    }
    return(0);
};

uint8_t Output_Handler::write_data(const char* data_to_write, uint16_t datalen){
    //write data to file
    switch(currentOutputType){
        case enumOutputStreamType::FILE :
            if (FileHandler.is_open()) {
                FileHandler.write(data_to_write, datalen);
            }else{
                ptrDebug->debug(1,"output_handler: no file open for writing! exiting...");
                exit(1);
            }
        break;

        case enumOutputStreamType::STDOUT :
            //std::cout.write(data_to_write,datalen);
            int wb;
            wb = fwrite(data_to_write, sizeof(char), datalen,stdout);
            ptrDebug->debug(3,"output_handler: writing to stdout bytecount: ",false);
            ptrDebug->debug(3,datalen,false,false);
            ptrDebug->debug(3," - written by fwrite: ",false,false);
            ptrDebug->debug(3,wb,true,false);
        break;

        default:
            ptrDebug->debug(1,"output_handler: no output stream active.");
            return(1);

    }
    return(0);
};
uint8_t Output_Handler::close_output_stream(){
    //close file
    if (!FileHandler.is_open()) {
        FileHandler.close();
        return(1);
    }
    return(0);
};
