#include "output_handler.h"

Output_Handler::Output_Handler(Custom_Debugger* ext_debug_obj){
    ptrDebug = ext_debug_obj;
};

Output_Handler::~Output_Handler(){
    
}


uint8_t Output_Handler::open_output_stream(std::string filepath){
    enumOutputStreamType test_output_type = enumOutputStreamType::UNKNOWN;
    if(filepath == "-"){
        //output should go to stdout
        test_output_type = enumOutputStreamType::STDOUT;
    }else {
        //set output
        test_output_type = enumOutputStreamType::FILE;
    }   

    //stream output to file
    switch (test_output_type){
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
            ptrDebug->debug(3,"output_handler: using STDOUT as output stream");
            currentOutputType = enumOutputStreamType::STDOUT;
            currentOutputPath = "";
            _setmode(_fileno(stdout), O_BINARY);        //set output mode to binary to avoid problems with "random" 0x0d (new line) insertion
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
        
        
        case enumOutputStreamType::STDOUT : {
            //std::cout.write(data_to_write,datalen);
            int wb;   
            wb = fwrite(data_to_write, sizeof(char), datalen,stdout);
            //fflush(stdout); --> dont flush needed so data is not accumulated in stdout buffer. but flush after every write might be slow. -> better flush after packet is done. this is done in packetizer class!
            if(ptrDebug->get_debug_level() >= 4){
                ptrDebug->debug(4,"output_handler: writing to stdout bytecount: ",false);
                ptrDebug->debug(4,datalen,false,false);
                ptrDebug->debug(4," - written by fwrite: ",false,false);
                ptrDebug->debug(4,wb,true,false);

                char* dataString2 = new char[datalen * 3 + 1];
                for (size_t i = 0; i < datalen; ++i) {
                    sprintf(dataString2 + i * 3, "%02X ", static_cast<unsigned char>(data_to_write[i]));
                }
                ptrDebug->debug(4,"output_Handler : write STDOUT - RAW DATA: ",false);
                ptrDebug->debug(4,dataString2,true, false);
                delete[] dataString2;
            }
            break;
        }

        default:
            ptrDebug->debug(1,"output_handler: no output stream active.");
            return(1);

    }
    return(0);
};

uint8_t Output_Handler::close_output_stream(){
    //flush stdout+stderr buffer to send last data from buffers
    fflush(stdout); 
    fflush(stderr); 
    //close file handler if still open
    if (!FileHandler.is_open()) {
        FileHandler.close();
        return(1);
    }
    return(0);
};
