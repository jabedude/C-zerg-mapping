#include <byteswap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pcap.h"
#include "zerg.h"

/* STATIC TEMPLATES */
static const PcapHeader_t st_pcap = {0xa1b2c3d4, 2, 4, 0, 0, MAX_PACKET_CAPTURE, 1};
static const PcapPackHeader_t st_pack = {0x582b59dc, 0x000701d2, 0x00000000, 0x00000000};
static const EthHeader_t st_eth = {{0xea, 0x7e, 0xa7, 0xfa, 0x55, 0xc5}, {0xea, 0x7e, 0xa7, 0x8e, 0x16, 0x48}, 0x0008};
static const IpHeader_t st_ip = {0x45, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x11, 0x0000, 0x720f000a, 0x3015000a};
static const UdpHeader_t st_udp = {0x4281, 0xa70e, 0x0000, 0x0000};

static uint16_t ip_checksum(const void *ip, size_t len)
{   /* Calculates + returns IPv4 header checksum. */
    /* http://www.netfor2.com/ipsum.htm */
    unsigned long sum = 0;
    const uint16_t *ip1;

    ip1 = ip;
    while (len > 1)
    {
        sum += *ip1++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return(~sum);
}

static uint16_t udp_checksum(const void *udp, size_t len, in_addr_t src, in_addr_t dst)
{   /* Calculates and returns UDP checksum. */
    /* http://www4.ncsu.edu/~mlsichit/Teaching/407/Resources/udpChecksum.html */
    const uint16_t *buf = udp;
    uint16_t *ip_src=(void *)&src, *ip_dst=(void *)&dst;
    uint32_t sum;
    size_t i;

    sum = 0;

    sum += *(ip_src++);
    sum += *ip_src;
    sum += *(ip_dst++);
    sum += *ip_dst;
    sum += 0x0011;
    sum += len;

    for (i = 0; i < len/2; i++) {
        /* Hannah made me realize I needed to bswap these 2-byte values */
        sum += bswap_16(buf[i]);
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }

    if (len & 1)
            sum += bswap_16((uint16_t) buf[i]);

    /* Add carries to least sig byte */
    while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16);

    /* One's complement */
    return htons((uint16_t) (sum ^ 0xFFFF));
}

static void write_msg(FILE *pfp, ZergHeader_t *zh, char *msg)
{   /* Writes a message payload zerg packet to pfp */
    PcapPackHeader_t pack = st_pack;
    IpHeader_t ip = st_ip;
    UdpHeader_t udp = st_udp;
    const size_t msg_len = strlen(msg);
    uint8_t *datagram;

    pack.recorded_len = sizeof(st_eth) + sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + msg_len;
    pack.orig_len = pack.recorded_len;

    ip.ip_len = htons(sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + msg_len);
    ip.ip_sum = ip_checksum(&ip, 20);

    udp.uh_ulen = htons(sizeof(udp) + sizeof(ZergHeader_t) + msg_len);
    datagram = (uint8_t *) malloc(ntohs(udp.uh_ulen) + 1);
    memcpy(datagram, &udp, sizeof(UdpHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t)], zh, sizeof(ZergHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t) + sizeof(ZergHeader_t)], msg, msg_len);
    udp.uh_sum = udp_checksum(datagram, ntohs(udp.uh_ulen), htonl(ip.ip_src), htonl(ip.ip_dst));
    free(datagram);
    /* EVERYTHING ABOVE THIS ARE INITIALIZERS */

    fwrite(&pack, sizeof(pack), 1, pfp);
    fwrite(&st_eth, sizeof(st_eth), 1, pfp);
    fwrite(&ip, sizeof(ip), 1, pfp);
    fwrite(&udp, sizeof(udp), 1, pfp);
    fwrite(zh, sizeof(ZergHeader_t), 1, pfp);
    fwrite(msg, sizeof(char), msg_len, pfp);

    return;
}

static void write_stat(FILE *pfp, ZergHeader_t *zh, ZergStatPayload_t *zsp, char *name)
{   /* Writes a status payload zerg packet to pfp */
    PcapPackHeader_t pack = st_pack;
    IpHeader_t ip = st_ip;
    UdpHeader_t udp = st_udp;
    uint8_t *datagram;
    const size_t name_len = strlen(name);

    pack.recorded_len = sizeof(st_eth) +
                        sizeof(ip) +
                        sizeof(udp) +
                        sizeof(ZergHeader_t) +
                        sizeof(ZergStatPayload_t) +
                        name_len;
    pack.orig_len = pack.recorded_len;

    ip.ip_len = htons(sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + sizeof(ZergStatPayload_t) + name_len);
    ip.ip_sum = ip_checksum(&ip, 20);

    udp.uh_ulen = htons(sizeof(udp) + + sizeof(ZergHeader_t) + sizeof(ZergStatPayload_t) + name_len);
    datagram = (uint8_t *) malloc(ntohs(udp.uh_ulen) + 1);
    memcpy(datagram, &udp, sizeof(UdpHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t)], zh, sizeof(ZergHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t) + sizeof(ZergHeader_t)], zsp, sizeof(ZergStatPayload_t));
    memcpy(&datagram[sizeof(UdpHeader_t) + sizeof(ZergHeader_t) + sizeof(ZergStatPayload_t)], name, name_len);
    udp.uh_sum = udp_checksum(datagram, ntohs(udp.uh_ulen), htonl(ip.ip_src), htonl(ip.ip_dst));
    free(datagram);
    /* EVERYTHING ABOVE THIS ARE INITIALIZERS */

    fwrite(&pack, sizeof(pack), 1, pfp);
    fwrite(&st_eth, sizeof(st_eth), 1, pfp);
    fwrite(&ip, sizeof(ip), 1, pfp);
    fwrite(&udp, sizeof(udp), 1, pfp);
    fwrite(zh, sizeof(ZergHeader_t), 1, pfp);
    fwrite(zsp, sizeof(ZergStatPayload_t), 1, pfp);
    fwrite(name, sizeof(char), name_len, pfp);
    return;
}

static void write_cmd(FILE *pfp, ZergHeader_t *zh, ZergCmdPayload_t *zcp)
{   /* Writes a command payload zerg packet to pfp */
    PcapPackHeader_t pack = st_pack;
    IpHeader_t ip = st_ip;
    UdpHeader_t udp = st_udp;
    uint8_t *datagram;

    pack.recorded_len = sizeof(st_eth) + sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + sizeof(ZergCmdPayload_t);
    pack.orig_len = pack.recorded_len;

    ip.ip_len = htons(sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + sizeof(ZergCmdPayload_t));
    ip.ip_sum = ip_checksum(&ip, 20);

    udp.uh_ulen = htons(sizeof(udp) + + sizeof(ZergHeader_t) + sizeof(ZergCmdPayload_t));
    datagram = (uint8_t *) malloc(ntohs(udp.uh_ulen) + 1);
    memcpy(datagram, &udp, sizeof(UdpHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t)], zh, sizeof(ZergHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t) + sizeof(ZergHeader_t)], zcp, sizeof(ZergCmdPayload_t));
    udp.uh_sum = udp_checksum(datagram, ntohs(udp.uh_ulen), htonl(ip.ip_src), htonl(ip.ip_dst));
    free(datagram);
    /* EVERYTHING ABOVE THIS ARE INITIALIZERS */

    fwrite(&pack, sizeof(pack), 1, pfp);
    fwrite(&st_eth, sizeof(st_eth), 1, pfp);
    fwrite(&ip, sizeof(ip), 1, pfp);
    fwrite(&udp, sizeof(udp), 1, pfp);
    fwrite(zh, sizeof(ZergHeader_t), 1, pfp);
    fwrite(zcp, sizeof(ZergCmdPayload_t), 1, pfp);
    return;
}

static void write_gps(FILE *pfp, ZergHeader_t *zh, ZergGpsPayload_t *zgp)
{   /* Writes a gps data payload packet to pfp */
    PcapPackHeader_t pack = st_pack;
    IpHeader_t ip = st_ip;
    UdpHeader_t udp = st_udp;
    uint8_t *datagram;

    pack.recorded_len = sizeof(st_eth) + sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + sizeof(ZergGpsPayload_t);
    pack.orig_len = pack.recorded_len;

    ip.ip_len = htons(sizeof(ip) + sizeof(udp) + sizeof(ZergHeader_t) + sizeof(ZergGpsPayload_t));
    ip.ip_sum = ip_checksum(&ip, 20);

    udp.uh_ulen = htons(sizeof(udp) + + sizeof(ZergHeader_t) + sizeof(ZergGpsPayload_t));
    datagram = (uint8_t *) malloc(ntohs(udp.uh_ulen) + 1);
    memcpy(datagram, &udp, sizeof(UdpHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t)], zh, sizeof(ZergHeader_t));
    memcpy(&datagram[sizeof(UdpHeader_t) + sizeof(ZergHeader_t)], zgp, sizeof(ZergGpsPayload_t));
    udp.uh_sum = udp_checksum(datagram, ntohs(udp.uh_ulen), htonl(ip.ip_src), htonl(ip.ip_dst));
    free(datagram);
    /* EVERYTHING ABOVE THIS ARE INITIALIZERS */

    fwrite(&pack, sizeof(pack), 1, pfp);
    fwrite(&st_eth, sizeof(st_eth), 1, pfp);
    fwrite(&ip, sizeof(ip), 1, pfp);
    fwrite(&udp, sizeof(udp), 1, pfp);
    fwrite(zh, sizeof(ZergHeader_t), 1, pfp);
    fwrite(zgp, sizeof(ZergGpsPayload_t), 1, pfp);
    return;
}

void read_input(FILE *fp, FILE *pfp)
{   /* Reads input file and encodes psychic capture */
    int pack_num;
    uint8_t zerg_version;
    uint16_t zerg_src, zerg_dst;
    uint32_t zerg_sequence;
    ZergHeader_t zh;
    char str[MAX_LINE_SIZE], line[MAX_LINE_SIZE];
    uint32_t zerg_hp, zerg_max_hp;
    uint8_t zerg_armor;
    union Fto32 {
        uint32_t b;
        int32_t i;
        float f;
    };
    union Fto32 stat_speed;
    char name[MAX_LINE_SIZE];
    union Dto64 {
        uint64_t b;
        double d;
    };
    union Dto64 dto64;
    union Dto64 latto64;
    union Fto32 altitude, bearing, speed, acc;

    /* Write the Psychic Capture Header */
    fwrite(&st_pcap, sizeof(st_pcap), 1, pfp);

    /* Encoding loop */
    while (fgets(line, MAX_LINE_SIZE, fp)) {
        if (sscanf(line, "*** Packet %d ***", &pack_num))
            fprintf(stderr, "PACKET NUMBER IS %d\n", pack_num);
        else if (sscanf(line, "Version : %hhu", &zerg_version))
            fprintf(stderr, "VERSION IS %u\n", zerg_version);
        else if (sscanf(line, "Sequence : %u", &zerg_sequence))
            fprintf(stderr, "SEQUENCE IS %u\n", zerg_sequence);
        else if (sscanf(line, "From : %hu", &zerg_src))
            fprintf(stderr, "SOURCE IS %u\n", zerg_src);
        else if (sscanf(line, "To : %hu", &zerg_dst))
            fprintf(stderr, "DESTINATION IS %u\n", zerg_dst);
        else if (sscanf(line, "Message : %[^\n]", str)) {
#ifdef DEBUG
            printf("DEBUG: MESSAGE IS %s\n", str);
#endif
            uint32_t len = strlen(str);
            len += ZERG_SIZE;
            zh.zh_len[0] = (len >> 16) & 0xFF;
            zh.zh_len[1] = (len >> 8) & 0xFF;
            zh.zh_len[2] = len & 0xFF;
            zh.zh_vt = 0x10;
            zh.zh_src = htons(zerg_src);
            zh.zh_dest = htons(zerg_dst);
            zh.zh_seqid = htonl(zerg_sequence);
            write_msg(pfp, &zh, str);
#ifdef DEBUG
            printf("DEBUG: fp is at %ld\n", ftell(fp));
#endif
        }
        else if (sscanf(line, "HP : %u", &zerg_hp)) {
            for (int i = 0; i < 5; i++) {
                (void) fgets(line, MAX_LINE_SIZE, fp);
                (void) sscanf(line, "Max-HP : %u", &zerg_max_hp);
                (void) sscanf(line, "Type : %[^\n]", str);
                (void) sscanf(line, "Armor : %hhu", &zerg_armor);
                (void) sscanf(line, "Speed(m/s) : %f", &stat_speed.f);
                (void) sscanf(line, "Name : %[^\n]", name);
            }
#ifdef DEBUG
            printf("DEBUG: THIS IS A STATUS PACKET\n");
            printf("VER IS %d\nSEQ IS %d\nSRC IS %d\n", zerg_version, zerg_sequence, zerg_src);
            printf("DST IS %d\nHP IS %d\nMAX-HP IS %d\nTYPE IS %s\n", zerg_dst, zerg_hp, zerg_max_hp, str);
            printf("ARMOR IS %d\nSPEED IS %lf\nNAME IS %s\n", zerg_armor, stat_speed.f, name);
#endif

            uint32_t len = strlen(name) + 12;
            len += ZERG_SIZE;
            zh.zh_len[0] = (len >> 16) & 0xFF;
            zh.zh_len[1] = (len >> 8) & 0xFF;
            zh.zh_len[2] = len & 0xFF;
            zh.zh_vt = 0x11;
            zh.zh_src = htons(zerg_src);
            zh.zh_dest = htons(zerg_dst);
            zh.zh_seqid = htonl(zerg_sequence);

            ZergStatPayload_t zsp = (const ZergStatPayload_t) {0};
            const ZergData_t breeds[] = {
                {0, "Overmind"}, {1, "Larva"},
                {2, "Cerebrate"}, {3, "Overlord"},
                {4, "Queen"}, {5, "Drone"},
                {6, "Zergling"}, {7, "Lurker"},
                {8, "Broodling"}, {9, "Hydralisk"},
                {10, "Guardian"}, {11, "Scourge"},
                {12, "Ultralisk"}, {13, "Mutalisk"},
                {14, "Defiler"}, {15, "Devourer"},
            };

            zsp.zsp_hp[0] = (zerg_hp >> 16) & 0xFF;
            zsp.zsp_hp[1] = (zerg_hp >> 8) & 0xFF;
            zsp.zsp_hp[2] = zerg_hp & 0xFF;
            zsp.zsp_armor = zerg_armor;
            zsp.zsp_maxhp[0] = (zerg_max_hp >> 16) & 0xFF;
            zsp.zsp_maxhp[1] = (zerg_max_hp >> 8) & 0xFF;
            zsp.zsp_maxhp[2] = zerg_max_hp & 0xFF;
            for (int i = 0; i < 16; i++) {
                if (!strcmp(breeds[i].data, str)) {
                    zsp.zsp_ztype = i;
                    break;
                }
                i++;
            }
            zsp.zsp_speed = htonl(stat_speed.b);
            write_stat(pfp, &zh, &zsp, name);

#ifdef DEBUG
            printf("DEBUG: fp is at %ld\n", ftell(fp));
#endif
        }
        else if (sscanf(line, "Longitude : %le deg", &dto64.d)) {
            for (int i = 0; i < 8; i++) {
                (void) fgets(line, MAX_LINE_SIZE, fp);
                (void) sscanf(line, "Latitude : %le deg", &latto64.d);
                (void) sscanf(line, "Altitude : %e m", &altitude.f);
                (void) sscanf(line, "Bearing : %e deg", &bearing.f);
                (void) sscanf(line, "Speed : %e m/s", &speed.f);
                (void) sscanf(line, "Accuracy : %e m", &acc.f);
            }
#ifdef DEBUG
            printf("DEBUG: THIS IS A GPS PACKET\n");
            printf("VER IS %d\nSEQ IS %d\nSRC IS %d\nDST IS %d\n", zerg_version, zerg_sequence, zerg_src, zerg_dst);
            printf("LONG IS %6.4f\nLAT IS %6.4f\nALT IS %6.4f\n", dto64.d, latto64.d, altitude.f);
            printf("BEARING IS %6.4f\nSPEED IS %6.4f\nACCURACY IS %6.4f\n", bearing.f, speed.f, acc.f);
#endif
            ZergGpsPayload_t zgp = (const ZergGpsPayload_t) {0};

            uint32_t len = 32 + ZERG_SIZE;
            zh.zh_len[0] = (len >> 16) & 0xFF;
            zh.zh_len[1] = (len >> 8) & 0xFF;
            zh.zh_len[2] = len & 0xFF;
            zh.zh_vt = 0x13;
            zh.zh_src = htons(zerg_src);
            zh.zh_dest = htons(zerg_dst);
            zh.zh_seqid = htonl(zerg_sequence);

            zgp.zgp_long = bswap_64(dto64.b);
            zgp.zgp_lat = bswap_64(latto64.b);
            zgp.zgp_alt = htonl(altitude.b);
            zgp.zgp_bearing = htonl(bearing.b);
            zgp.zgp_speed = htonl(speed.b);
            zgp.zgp_acc = htonl(acc.b);
            write_gps(pfp, &zh, &zgp);
#ifdef DEBUG
            printf("DEBUG: fp is at %ld\n", ftell(fp));
#endif
        }
        else if (sscanf(line, "%[^\n]", str)) {
#ifdef DEBUG
            printf("DEBUG: THIS IS A COMMAND PACKET\n");
            printf("VER IS %d\nSEQ IS %d\nSRC IS %d\nDST IS %d\nCOMMAND IS %s\n",
                    zerg_version, zerg_sequence, zerg_src, zerg_dst, str);
#endif
            ZergCmdPayload_t zcp = (const ZergCmdPayload_t) {0};
            const ZergData_t cmds[] = {
                {0, "GET_STATUS"}, {1, "GOTO"},
                {2, "GET_GPS"}, {3, "NONE"},
                {4, "RETURN"}, {5, "SET_GROUP"},
                {6, "STOP"}, {7, "REPEAT"},
            };
            uint32_t len = 2;
            len += ZERG_SIZE;
            zh.zh_len[0] = (len >> 16) & 0xFF;
            zh.zh_len[1] = (len >> 8) & 0xFF;
            zh.zh_len[2] = len & 0xFF;
            zh.zh_vt = 0x12;
            zh.zh_src = htons(zerg_src);
            zh.zh_dest = htons(zerg_dst);
            zh.zh_seqid = htonl(zerg_sequence);

            int i;
            for (i = 0; i < 16; i++) {
                if (!strcmp(cmds[i].data, str)) {
                    zcp.zcp_command = htons(i);
                    break;
                }
            }
            if (i % 2 == 0) {
                /* No parameters passed */
                write_cmd(pfp, &zh, &zcp);
#ifdef DEBUG
                printf("DEBUG: fp is at %ld\n", ftell(fp));
#endif
            } else {
                /* Params passed */
                uint32_t len = 8;
                len += ZERG_SIZE;
                zh.zh_len[0] = (len >> 16) & 0xFF;
                zh.zh_len[1] = (len >> 8) & 0xFF;
                zh.zh_len[2] = len & 0xFF;
                uint16_t param_one = 0;
                union Fto32 param_two;
                (void) fgets(line, MAX_LINE_SIZE, fp);
                if (i == 1) {
                    (void) sscanf(line, "Move %hu m at bearing %f", &param_one, &param_two.f);
                    zcp.zcp_param_one = htons(param_one);
                    zcp.zcp_param_two = htonl(param_two.b);
                }
                else if (i == 5) {
                    (void) sscanf(line, "%s to/from group ID %d", str, &param_two.i);
                    if (!strcmp(str, "ADD"))
                        param_one = 1;
                    zcp.zcp_param_one = htons(param_one);
                    zcp.zcp_param_two = ~param_two.i + 1;
                }
                else if (i == 7) {
                    (void) sscanf(line, "Re-send %u", &param_two.b);
                    zcp.zcp_param_one = htons(param_one);
                    zcp.zcp_param_two = htonl(param_two.b);
                }

#ifdef DEBUG
                printf("DEBUG: PARAM ONE IS %d\n", param_one);
                printf("DEBUG: PARAM TWO IS %f\n", param_two.f);
#endif
                write_cmd(pfp, &zh, &zcp);
#ifdef DEBUG
                printf("DEBUG: fp is at %ld\n", ftell(fp));
#endif
            }
        }
    }
    return;
}
