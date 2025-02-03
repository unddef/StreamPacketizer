# IEC60870-5-101 Sniffer - 101StreamPacketizer
This program is designed for sniffing serial IEC60870-5-101 traffic. The captured byte stream is packetized and saved in [PCAP file format](https://wiki.wireshark.org/Development/LibpcapFileFormat) for further analysis in [Wireshark] (https://wireshark.com/). The motivation for developing this program originates from the existence of an IEC60870-5-101 dissector already available for Wireshark. However, there was previously no convenient method for capturing serial data from a COM port into Wireshark. That has now changed!

## data input
The input data (raw stream) can be fed into the program in several ways. The first option is to open a COM port on the system. Alternatively, a TCP stream can be used as the input source, which is particularly useful in combination with netcat. You can open a COM port (or ttyX) and pipe the data to a TCP listening socket using netcat. This allows you to connect the 101-sniffer to the TCP port over the network and capture traffic from a remote host.

## packetizing process
To split the incoming byte stream into telegrams, the program searches for start bytes. The protocol standard defines variable-length telegrams that begin with 0x68 and fixed-length telegrams that start with 0x10. For fixed-length telegrams, the program checks whether the stop byte 0x16 is present after the fixed length. For variable-length telegrams, the length is extracted from the telegram. If a valid telegram is identified, it is written as a packet to the PCAP file.

## data output
The packetized data is safed in PCAP file format for further processing in wireshark. When a new file is started, the PCAP file header is written first, followed by the captured traffic. Each recognized IEC 60870-5-101 telegram is written as a separate packet. To ensure proper display in Wireshark, dummy MAC, IP, and TCP headers are added to each packet.
Garbage data from the input stream is not discarded but also written to the file to ensure easy diagnostics of faulty communication lines.
It is not only possible to write output data to a file but also to use STDOUT as output destination. This enables live traffic captures by piping the output to wireshark's STDIN input interface.

## TODOs beeing worked on
 - [] set com port parameters by cmdline
 - [] think about COMTIMEOUTS in input_handler.cpp 

## current limitations
 - Short ACK (0xE5 & 0xA2) not supported/implemented yet
 - input-source "file" not implemented yet
 - "TCP connection failed" at program startup not detected
 - "TCP connection failed" while captureing not detected
 - link_addr_len, ASDU_addr_len, IOA_len not configurable (as parameters) 
 
## commandline options
### -d <0-4> / Debug level
The debug level can be set from 0 (=no output) to 4(=everything).
Debug output is send to STDERR. This normally ends up on the command line. But you can redirect the debug output by "2> <file>" if needed.

### -i "inputPath" / input source
With this option you can specify the input source. Possible sources are: COM port, TCP stream or file (not implemented yet!).
The input path is specified as a string. If a COM port should be opend use: -i COM2
For TCP stream input use -i <IP:PORT> like this -i 10.0.0.1:8888
To use a file as input source specify the file path: -i "c:\users\test\desktop\capture.pcap"

### -o "outputPath" / output destination
currently there are two output options: file or STDOUT
To write the captured traffic to a file specify the output path: -o "c:\users\test\desktop\capture.pcap"
If "-" is used as outputPath STDOUT is used for data output. This can be redirected by ">" and by pipe "|". 

### -b <baudrate> / COM port baudrate
allowed baudrates 110,300,600,1200,2400,4800,9600,14400,19200,38400,56000,57600,115200,256000

### -s <1-2> / stopbits

### -p <0-2> / parity
0 = none, 1 = odd, 2 = even

## Usage examples
### open COM port and write to file
```
101sniffer.exe -d 3 -i COM1 -o outputfile.pcap
```

### open TCP connection and write to file
```
101sniffer.exe -d 3 -i 10.0.0.1:8888 -o outputfile.pcap
```
### open TCP connection and pipe to Wireshark
```
101sniffer.exe -d 3 -i 10.0.0.1:8888 -o - | "c:\Program Files\Wireshark\Wireshark.exe" -k -i -
```
### redirect STDOUT to file
```
101sniffer.exe -d 3 -i COM1 -o outputDatafile.pcap 2> errorlog.txt
```