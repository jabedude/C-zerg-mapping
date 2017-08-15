#ifndef CODEC_PCAP_HEADER
#define CODEC_PCAP_HEADER

#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>

/* Defines */
#define ETH_SIZE 14
#define UDP_SIZE 0xC
#define ZERG_SIZE 12
#define MAX_LINE_SIZE 128

/* Structs for reading a pcap file */

typedef struct pcap_header {
    uint32_t magic_num;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
} PcapHeader_t;


typedef struct pcap_pack_header {
    uint32_t epoch;
    uint32_t ms_from_epoch;
    uint32_t recorded_len;
    uint32_t orig_len;
} PcapPackHeader_t;

typedef struct eth_header {
    uint8_t eth_dhost[6];
    uint8_t eth_shost[6];
    uint16_t eth_type;
} EthHeader_t;

typedef struct ip_header {
    uint8_t  ip_vhl;
    uint8_t  ip_tos;
    uint16_t ip_len;
    uint16_t ip_id;
    uint16_t ip_off;
#define IP_RF 0x8000
#define IP_DF 0x4000
#define IP_MF 0x2000
#define IP_OFFMASK 0x1fff
    uint8_t  ip_ttl;
    uint8_t  ip_p;
    uint16_t ip_sum;
    in_addr_t ip_src,ip_dst;
} IpHeader_t;

typedef struct udp_header {
    uint16_t uh_sport;
    uint16_t uh_dport;
    uint16_t uh_ulen;
    uint16_t uh_sum;
} UdpHeader_t;

typedef struct zerg_header {
    uint8_t zh_vt;
    uint8_t zh_len[3];
    uint16_t zh_src;
    uint16_t zh_dest;
    uint32_t zh_seqid;
} ZergHeader_t;

#endif
