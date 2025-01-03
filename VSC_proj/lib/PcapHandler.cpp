#include "PcapHandler.h"

Pcap_Handler::Pcap_Handler(Custom_Debugger* ext_debugger, Output_Handler* extOutputStream){
    ptrDebug = ext_debugger;
    ptrOutputStream = extOutputStream;
};

Pcap_Handler::~Pcap_Handler(){
};

// Utility function to convert 16-bit integer to network byte order
uint16_t Pcap_Handler::loc_htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}

// Utility function to convert 32-bit integer to network byte order
uint32_t Pcap_Handler::loc_htonl(uint32_t hostlong) {
    return ((hostlong >> 24) & 0xff) |
            ((hostlong >> 8) & 0xff00) |
            ((hostlong << 8) & 0xff0000) |
            ((hostlong << 24) & 0xff000000);
}

bool Pcap_Handler::write_file_header() {
    PcapFileHeader pcapHeader;
    pcapHeader.magic_number = 0xa1b2c3d4; // Magic number for PCAP
    pcapHeader.version_major = 2;        // PCAP format major version
    pcapHeader.version_minor = 4;        // PCAP format minor version
    pcapHeader.thiszone = 0;             // UTC timezone
    pcapHeader.sigfigs = 0;              // Accuracy of timestamps
    pcapHeader.snaplen = 65535;          // Max packet length
    pcapHeader.network = 1;              // Ethernet link-layer
    //write header
    ptrOutputStream->write_data(reinterpret_cast<char*>(&pcapHeader), sizeof(pcapHeader));
    return(1);
};

 // Writes a packet to the PCAP file
    bool Pcap_Handler::write_packet(const char* packet_data, uint32_t packet_length, bool is_msg_from_master) {
        PcapPacketHeader packet_header;
        EthernetHeader eth_header = {};
        IPv4Header ip_header = {};
        TcpHeader tcp_header = {};

        // fixed ethernet header (to be added before payload data)
        //std::memset(eth_header.destination, 0xbb, 6); // Broadcast MAC
        //std::memset(eth_header.source, 0xaa, 6);      // Example source MAC
        eth_header.ethertype = loc_htons(0x0800);         // EtherType for IPv4  (use htons()???)
        
        // IPv4 header setup
        ip_header.version_ihl = (4 << 4) | 5;         // IPv4, Header Length = 5 words (20 bytes)
        ip_header.tos = 0;                            // Default TOS
        ip_header.total_length = loc_htons(sizeof(ip_header) + sizeof(TcpHeader) + packet_length);
        ip_header.id = loc_htons(1);                      // Packet ID
        ip_header.flags_fragment = 0;                 // No fragmentation
        ip_header.ttl = 64;                           // Time to Live
        ip_header.protocol = 6;                       // Protocol (TCP = 6)
        ip_header.checksum = 0;                       // Checksum (set to 0 for simplicity)
        //ip_header.source_ip = loc_htonl(0xc0a80001);      // Source IP (192.168.0.1)
        //ip_header.destination_ip = loc_htonl(0xc0a80002); // Destination IP (192.168.0.2)

        // TCP header setup
        //tcp_header.source_port = loc_htons(1111);        // Source port
        //tcp_header.dest_port = loc_htons(2222);             // Destination port
        //tcp_header.seq_num = loc_htonl(1000);  // Sequence number
        //tcp_header.ack_num = loc_htonl(1000);       // Acknowledgment number
        tcp_header.data_offset = (sizeof(TcpHeader) / 4) << 4; // Data offset
        tcp_header.flags = 0x18;                      // PSH + ACK flags
        tcp_header.window_size = loc_htons(1024);         // Window size
        tcp_header.checksum = 0;                      // Checksum (set to 0 for simplicity)
        tcp_header.urgent_pointer = 0;                // No urgent data
        
        //variables that change depending on direction
        char src_mac = 0xA4, dst_mac = 0xF9;
        uint16_t src_port = 2424, dst_port = 1212;      //view from master

        if(is_msg_from_master){
            ptrDebug->debug(4,"writing packet from master");
            std::memset(eth_header.destination, dst_mac, 6);
            std::memset(eth_header.source, src_mac, 6);
            ip_header.source_ip = loc_htonl(0xc0a80001);
            ip_header.destination_ip =  loc_htonl(0xc0a80002);
            tcp_header.source_port = loc_htons(src_port);        
            tcp_header.dest_port = loc_htons(dst_port);
            tcp_header.seq_num = loc_htonl(tcp_sequence_master); 
            tcp_header.ack_num = loc_htonl(tcp_sequence_slave);
            tcp_sequence_master = tcp_sequence_master + packet_length;

        }else{
            ptrDebug->debug(4,"writing packet from slave");
            std::memset(eth_header.destination, src_mac, 6);
            std::memset(eth_header.source, dst_mac, 6);
            ip_header.source_ip = loc_htonl(0xc0a80002);
            ip_header.destination_ip =  loc_htonl(0xc0a80001);
            tcp_header.source_port = loc_htons(dst_port);        
            tcp_header.dest_port = loc_htons(src_port);
            tcp_header.seq_num = loc_htonl(tcp_sequence_slave); 
            tcp_header.ack_num = loc_htonl(tcp_sequence_master);
            tcp_sequence_slave = tcp_sequence_slave + packet_length;
        }
        


        //tcp_sequence_nr += packet_length;
        //tcp_ack_nr = tcp_sequence_nr;

        // Calculate the packet size
        size_t packet_size = sizeof(eth_header) + sizeof(ip_header) + sizeof(tcp_header) + packet_length;
        
        //pcap packet header
        packet_header.ts_sec = static_cast<uint32_t>(time(nullptr));    // Current time in seconds
        packet_header.ts_usec = 0;                                      // Microseconds (set to 0)
        packet_header.incl_len = packet_size;                           // Captured packet length
        packet_header.orig_len = packet_size;                           // Original packet length
        
        
        
        // Write packet header
        ptrOutputStream->write_data(reinterpret_cast<char*>(&packet_header), sizeof(packet_header));
        // Write Ethernet header
        ptrOutputStream->write_data(reinterpret_cast<char*>(&eth_header), sizeof(eth_header));
        // Write ip header
        ptrOutputStream->write_data(reinterpret_cast<char*>(&ip_header), sizeof(ip_header));
        // Write tcp header
        ptrOutputStream->write_data(reinterpret_cast<char*>(&tcp_header), sizeof(tcp_header));
        // Write packet data
        ptrOutputStream->write_data(packet_data, packet_length);
        return(1);
    }