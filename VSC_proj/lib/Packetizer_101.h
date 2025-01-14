#include "debug.h"
#include "buffer_handler.h"
#include "pcapHandler.h"

class Packetizer_101 {

    public:
        Packetizer_101(Custom_Debugger*, Buffer_Handler*,Pcap_Handler*,uint8_t,uint8_t,uint8_t,uint16_t);
        ~Packetizer_101();

        uint8_t process_buffer();
    private:
        Custom_Debugger* ptrDebug;
        Buffer_Handler* ptrBuffer;
        Pcap_Handler* ptrPcapWriter;

        uint8_t link_addr_len;
        uint8_t asdu_addr_len;
        uint8_t ioa_addr_len;
        uint16_t max_telegram_length;

        bool get_prm_bit(char);
};