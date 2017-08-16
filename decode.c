#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "lib/pcap.h"
#include "lib/zerg.h"
#include "lib/tree.h"

int main(int argc, char **argv)
{
    FILE *fp;
    long file_len, packet_end;
    int packet_num;
    PcapHeader_t pcap;
    PcapPackHeader_t pcap_pack;
    ZergHeader_t zh;

    /* Arguments check */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pcap file>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "%s: Error opening file.\n", argv[0]);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    rewind(fp);

    (void) fread(&pcap, sizeof(pcap), 1, fp);
    if (pcap.magic_num != 0xa1b2c3d4) {
        fprintf(stderr, "Please supply a valid pcap file.\n");
        fclose(fp);
        return 1;
    }

    packet_num = 1;
    /* Main decode loop */
    Node *root = mknode();
    while (ftell(fp) < file_len) {
        (void) fread(&pcap_pack, sizeof(pcap_pack), 1, fp);
        packet_end = pcap_pack.recorded_len + ftell(fp);
        EthHeader_t eth;
        (void) fread(&eth, sizeof(eth), 1, fp);
        printf("DEBUG: ETH DEST HOST IS %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                                                eth.eth_dhost[0],
                                                eth.eth_dhost[1],
                                                eth.eth_dhost[2],
                                                eth.eth_dhost[3],
                                                eth.eth_dhost[4],
                                                eth.eth_dhost[5]);
        printf("DEBUG: ETH SRC HOST IS %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                                                eth.eth_shost[0],
                                                eth.eth_shost[1],
                                                eth.eth_shost[2],
                                                eth.eth_shost[3],
                                                eth.eth_shost[4],
                                                eth.eth_shost[5]);
        printf("DEBUG: ETHERTYPE IS: 0x%.2x\n", eth.eth_type);

        if (ntohs(eth.eth_type) == 0x0800) {
            fseek(fp, 20, SEEK_CUR);
        } else if (ntohs(eth.eth_type) == 0x86dd) {
            fseek(fp, 40, SEEK_CUR);
        } else {
            fprintf(stderr, "Usupported EtherType Version\n");
            goto cleanup;
        }

        fseek(fp, 8, SEEK_CUR);

        (void) fread(&zh, sizeof(zh), 1, fp);
        if ((zh.zh_vt >> 4) != 1) {
            fprintf(stderr, "Usupported Psychic Capture version\n");
            goto cleanup;
        }

        printf("*** Packet %d ***\n", packet_num);
        printf("Version : %x\n", zh.zh_vt >> 4);
        /* This program only supports version 1 */
        printf("Sequence : %u\n", ntohl(zh.zh_seqid));
        printf("From : %d\n", ntohs(zh.zh_src));
        printf("To : %d\n", ntohs(zh.zh_dest));

        /* Call type-specific decoder routines */
        /* Make a temp ZergBlock and insert into tree*/
        ZergBlock_t *zb = mkblk();
        uint8_t type = zh.zh_vt & 0xFF;
        switch (type) {
            case 0x10 :
                fseek(fp, (NTOH3(zh.zh_len)) - ZERG_SIZE, SEEK_CUR);
                break;
            case 0x11 :
                z_status_parse(fp, &zh, zb);
                nadd(root, zb);
                break;
            case 0x12 :
                fseek(fp, (NTOH3(zh.zh_len)) - ZERG_SIZE, SEEK_CUR);
                break;
            case 0x13 :
                z_gps_parse(fp, &zh); //TODO: get GPS into zb here
                break;
            default :
                fprintf(stderr, "%s: error reading psychic capture.\n", argv[0]);
                break;
        }

    cleanup:
        packet_num++;
        fseek(fp, packet_end, SEEK_SET);

    }
    rmtree(root);
    fclose(fp);
    return 0;
}
