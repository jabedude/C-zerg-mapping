#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "lib/pcap.h"
#include "lib/zerg.h"
#include "lib/tree.h"
#include "lib/graph.h"

int main(int argc, char **argv)
{
    /* Arguments check */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pcap file>\n", argv[0]);
        return 1;
    }

    FILE *fp;
    long file_len, packet_end;
    int packet_num;
    PcapHeader_t pcap;
    PcapPackHeader_t pcap_pack;
    ZergHeader_t zh;

    int opt = 0;
    int h_val = 10;

    /* get command line args */
    while ((opt = getopt(argc, argv, ":h:")) != -1) {
        switch (opt) {
            case 'h' :
                h_val = atoi(optarg);
                if (!h_val) {
                    fprintf(stderr, "Invalid health percentage.\n");
                    return -1;
                }
        }
    }

    if (!(argc - optind)) {
        fprintf(stderr, "Usage: %s <pcap file>\n", argv[0]);
        return 1;
    }

    /* Process files one by one */
    Node *root = mknode();
    unsigned int node_c = 0;

    for (int f = optind; f < argc; f++) {
        fp = fopen(argv[f], "rb");
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
        while (ftell(fp) < file_len) {
            (void) fread(&pcap_pack, sizeof(pcap_pack), 1, fp);
            packet_end = pcap_pack.recorded_len + ftell(fp);
            //EthHeader_t eth;
            //(void) fread(&eth, sizeof(eth), 1, fp);
            fseek(fp, sizeof(EthHeader_t), SEEK_CUR);
            IpHeader_t ip;
            fread(&ip, sizeof(IpHeader_t), 1, fp);

            if ((ip.ip_vhl >> 4) == 4) {
                ; // PASS
            } else if ((ip.ip_vhl >> 4) == 6) {
                fseek(fp, 20, SEEK_CUR);
            } else {
                fprintf(stderr, "Usupported IP Version\n");
                goto cleanup;
            }

            fseek(fp, 8, SEEK_CUR);

            (void) fread(&zh, sizeof(zh), 1, fp);
            if ((zh.zh_vt >> 4) != 1) {
                fprintf(stderr, "Usupported Psychic Capture version\n");
                goto cleanup;
            }
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
                    z_gps_parse(fp, &zh, zb); //TODO: get GPS into zb here; memleak HERE
                    nadd(root, zb);
                    break;
                default :
                    fprintf(stderr, "%s: error reading psychic capture.\n", argv[0]);
                    break;
            }

        cleanup:
            packet_num++;
            fseek(fp, packet_end, SEEK_SET);
        }

        fclose(fp);
    }

    node_c = nodecount(root);
    printf("Node count is %u\n", node_c);
    if (node_c > 1) { // http://www.geeksforgeeks.org/construct-ancestor-matrix-from-a-given-binary-tree/
        Graph_t *graph = mkgraph(node_c);
        initgraph(graph, root);

        printgraph(graph);
        printf("GRAPH HAS %d EDGES.\n", graph->edges);
        if (isconn(graph))
            puts("GRAPH IS CONNECTED");
        else
            puts("GRAPH IS NOT CONNECTED");
        rmgraph(graph); //TODO: FREE() GRAPH
    }

    ordprint(root);
    rmtree(root);
    return 0;
}
