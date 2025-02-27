#include "Packetizer_101.h"

Packetizer_101::Packetizer_101(Custom_Debugger* ext_debug_obj, Buffer_Handler* ext_buffer_obj,Pcap_Handler* ext_pcap_obj, uint16_t ext_max_tel_len=255){
    ptrDebug = ext_debug_obj;
    ptrBuffer = ext_buffer_obj;
    ptrPcapWriter = ext_pcap_obj;
    max_telegram_length = ext_max_tel_len;
    
};

Packetizer_101::~Packetizer_101(){
    
};

uint8_t Packetizer_101::set_link_addr_len(uint8_t link_len){
    if(link_len >= 0 && link_len <=2){
        link_addr_len = link_len;
        return(0);
    }else{
        ptrDebug->debug(1,"Packetizer: wrong link addr length. exiting ...");
        exit(1);
    }
};

uint8_t Packetizer_101::set_asdu_addr_len(uint8_t asdu_len){
    if(asdu_len >= 1 && asdu_len <=2){
        asdu_addr_len = asdu_len;
        return(0);
    }else{
        ptrDebug->debug(1,"Packetizer: wrong asdu address length. exiting ...");
        exit(1);
    }
};
uint8_t Packetizer_101::set_ioa_addr_len(uint8_t ioa_len){
    if(ioa_len >= 1 && ioa_len <=3){
        ioa_addr_len = ioa_len;
        return(0);
    }else{
        ptrDebug->debug(1,"Packetizer: wrong IOA address length. exiting ...");
        exit(1);
    }
};

uint8_t Packetizer_101::process_buffer(){
    DWORD data_length = ptrBuffer->buffer_bytes_filled();

    if(data_length<=0){ return(1);};    //no data in buffer

    char* retrieved_buffer_data = new char[data_length];
    retrieved_buffer_data = ptrBuffer->get_data(0,data_length);
    
    if(retrieved_buffer_data == nullptr){return(0);};   //error getting data from buffer

    //----debug--------
    if(ptrDebug->get_debug_level() >= 4){
        char* dumpString = new char[data_length * 3 + 1];
        for (size_t i = 0; i < data_length; i++) {
            sprintf(dumpString + i * 3, "%02X ", static_cast<unsigned char>(retrieved_buffer_data[i]));
        }
        ptrDebug->debug(4,"Packetizer: got ",false);
        ptrDebug->debug(4,data_length,false,false);
        ptrDebug->debug(4," bytes to analyse: ",false,false);
        ptrDebug->debug(4,dumpString,true,false);
        delete[] dumpString;
    }
    //---- end debug ------
   
    uint8_t fixed_frame_length = link_addr_len + 4;
    DWORD written_up_to_byte = 0;       // points to first _not_ written byte. (offset=+1 -> 0=no byte written. 1=data[0] written, 2=data[0-1] written etc)

    for (size_t i = 0; i < data_length; i++) {
        
        //fixed length frame 
        if(retrieved_buffer_data[i] == 0x10){       //0x10 -> start of fixed length frame
            if(i + fixed_frame_length-1 <= data_length-1){
                if(retrieved_buffer_data[i+fixed_frame_length-1] = 0x16 ){     //fixed length = 6 byte. 6th byte must be 0x16 (end byte)
                    //found fixed-length-frame
                    ptrDebug->debug(3,"found frame(fixed length) at buffer[",false);
                    ptrDebug->debug(3,i,false,false);
                    ptrDebug->debug(3,"-",false,false);
                    ptrDebug->debug(3,i+fixed_frame_length-1,false,false);
                    ptrDebug->debug(3,"]",true,false);
                    if(i > written_up_to_byte){     //garbage data before frame start. (dont drop but write to file)
                        ptrDebug->debug(3,"writing garbage from buffer[",false);
                        ptrDebug->debug(3,written_up_to_byte,false,false);
                        ptrDebug->debug(3,"-",false,false);
                        ptrDebug->debug(3,i,false,false);
                        ptrDebug->debug(3,"]",true,false);
                        ptrPcapWriter->write_packet(retrieved_buffer_data + written_up_to_byte,i - written_up_to_byte,false);
                    }
                    //write fixed length frame to file
                    ptrPcapWriter->write_packet(retrieved_buffer_data + i,fixed_frame_length,get_prm_bit(retrieved_buffer_data[i+1]));
                    written_up_to_byte = i + fixed_frame_length;
                    i = i + fixed_frame_length -1;                  //-1 offset because i is incremented after loop automaticly
                    ptrDebug->debug(4,"jumping to byte: ",false);
                    ptrDebug->debug(4,i+1,true,false);
                    continue;
                }
            } else{
                //frame not in buffer yet. skip scanning last chars in buffer (<5)
                break;
            } 
        }
        //END fixed length frame

        //variable length frame detector
        if(retrieved_buffer_data[i] == 0x68){       //0x10 -> start of fixed length frame
            if(i + 3 <= data_length){               //telegram header format: 0x68 len len 0x68
                if(retrieved_buffer_data[i+3] == 0x68){         //frame start byte found
                    uint16_t block_length=0;
                    if(retrieved_buffer_data[i+1] == retrieved_buffer_data[i+2] ){
                        block_length = retrieved_buffer_data[i+1];
                    }else{
                        //not a variable length telegram! proceed to next i iteration
                        ptrDebug->debug(4,"Packetizer: not a var length telegram. goto next i itteration");
                        continue;
                    }

                    //test if telegram is completely in buffer.
                    //total byte length in buffer: 4byte (startbyte1, 2x block lenth, startbyte2) + block_length + 2byte (checksum + stopbyte)
                    DWORD telegram_total_buffer_length = 4 + block_length + 2; 
                    if(i + telegram_total_buffer_length-1 > data_length-1){     //last byte is not in buffer! -1 offset because i starts at 0
                        ptrDebug->debug(4,"Packetizer: process_buffer(): var len telegram not fully in buffer");
                        break;                                                  //jump out of char-analysis for loop to wait for more chars in buffer   
                    };
                    
                    if(retrieved_buffer_data[i+telegram_total_buffer_length-1] == 0x16){     
                        //stop byte found. this is a telegram!
                        ptrDebug->debug(3,"found variable length frame at buffer[",false);
                        ptrDebug->debug(3,i,false,false);
                        ptrDebug->debug(3,"-",false,false);
                        ptrDebug->debug(3,i+telegram_total_buffer_length-1,false,false);
                        ptrDebug->debug(3,"]",true,false);

                        //garbage data before frame start. (dont drop but write to file)
                        if(i > written_up_to_byte){     
                        ptrPcapWriter->write_packet(retrieved_buffer_data + written_up_to_byte,i - written_up_to_byte,false);
                        }
                        //write found frame
                        ptrPcapWriter->write_packet(retrieved_buffer_data + i,telegram_total_buffer_length,get_prm_bit(retrieved_buffer_data[i+4]));
                        written_up_to_byte = i + telegram_total_buffer_length;
                        i = i + telegram_total_buffer_length-1;
                        ptrDebug->debug(4,"jumping to byte: ",false);
                        ptrDebug->debug(4,i+1,true,false);

                    }else{
                        //start of frame looked good, but there is no stopbyte after given data_length
                        ptrDebug->debug(4,"Packetizer - process_buffer(): not a var length telegram. goto next i itteration");
                        continue;
                    }
                    
                }
            }else{
                //header not completely in buffer yet
            }
        }
        //end of variable length frame

    }

    if ((written_up_to_byte == 0) && (ptrBuffer->buffer_free_bytes_left() <= 0) ){
        //buffer full but no telegram in buffer. need to write garbage data to output to free buffer space for new input
        ptrDebug->debug(2,"Packetizer: Buffer full but no telegram only garbage. Freeing buffer space.");
        uint16_t chunk_size = ptrBuffer->buffer_bytes_filled();
        if (chunk_size > max_telegram_length){
            chunk_size = max_telegram_length;
        }
        ptrPcapWriter->write_packet(retrieved_buffer_data,chunk_size,false);
        written_up_to_byte = chunk_size;
    }
    
    ptrDebug->debug(4,"written to byte: ",false);
    ptrDebug->debug(4,written_up_to_byte,true,false);
    if(written_up_to_byte > 0){
        ptrBuffer->discard_data(written_up_to_byte);
    }
    fflush(stdout);
    delete[] retrieved_buffer_data;
    

    return(1);
}

bool Packetizer_101::get_prm_bit(char lpci_control_field){
    if(lpci_control_field & 0x40){
        return(true);
    }else{
        return(false);
    }
}