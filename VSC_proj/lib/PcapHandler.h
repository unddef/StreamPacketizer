#ifndef Pcap_Handler_H
#define Pcap_Handler_H
#include "debug.h"

//#include <iostream>
#include <fstream>

class PcapHandler {

    private:
        Custom_Debugger* ptrDebug;
        std::ofstream pcapFileHandler;      //file handler
        uint32_t tcp_sequence_master = 0;
        uint32_t tcp_sequence_slave = 1000;
        uint32_t tcp_ack_nr = 0;
        // PCAP file header structure (24 bytes)
    
    struct PcapFileHeader {
        uint32_t magic_number   = 0xa1b2c3d4;   // Magic number for PCAP
        uint16_t version_major  = 2;            // PCAP format major version
        uint16_t version_minor  = 4;            // PCAP format minor version
        int32_t  thiszone       = 0;            // UTC timezone
        uint32_t sigfigs        = 0;            // Accuracy of timestamps
        uint32_t snaplen        = 65535;        // Max length of captured packets
        uint32_t network        = 1;            // Data link type
    };
    // PCAP packet header structure (16 bytes)
    struct PcapPacketHeader {
        uint32_t ts_sec         = 0;    // Timestamp seconds
        uint32_t ts_usec        = 0;    // Timestamp microseconds
        uint32_t incl_len       = 0;    // Number of bytes of packet saved in file
        uint32_t orig_len       = 0;    // Actual length of the packet
    };
    // Ethernet header structure
    struct EthernetHeader {
        uint8_t destination[6];  // Destination MAC address
        uint8_t source[6];       // Source MAC address
        uint16_t ethertype;      // EtherType field
    };
    // IPv4 header structure
    struct IPv4Header {
        uint8_t  version_ihl;    // Version and Internet Header Length
        uint8_t  tos;            // Type of Service
        uint16_t total_length;   // Total length of the IP packet
        uint16_t id;             // Identification
        uint16_t flags_fragment; // Flags and Fragment Offset
        uint8_t  ttl;            // Time to Live
        uint8_t  protocol;       // Protocol
        uint16_t checksum;       // Header checksum
        uint32_t source_ip;      // Source IP address
        uint32_t destination_ip; // Destination IP address
    };

    // TCP header structure
    struct TcpHeader {
        uint16_t source_port;    // Source port
        uint16_t dest_port;      // Destination port
        uint32_t seq_num;        // Sequence number
        uint32_t ack_num;        // Acknowledgment number
        uint8_t  data_offset;    // Data offset and reserved bits
        uint8_t  flags;          // Control flags
        uint16_t window_size;    // Window size
        uint16_t checksum;       // Checksum
        uint16_t urgent_pointer; // Urgent pointer
    };

    public:
        PcapHandler(Custom_Debugger*);
        ~PcapHandler();
        
        uint16_t loc_htons(uint16_t);
        uint32_t loc_htonl(uint32_t);
        bool open_file(const std::string& filepath);
        bool close_file(void);
        bool write_file_header();
        bool write_packet(const char* , uint32_t, bool );



};
#endif