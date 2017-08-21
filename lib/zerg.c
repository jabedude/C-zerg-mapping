#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "zerg.h"
#include "pcap.h"

#ifdef DEBUG
static const ZergData_t breeds[] = {
    {0, "Overmind"}, {1, "Larva"},
    {2, "Cerebrate"}, {3, "Overlord"},
    {4, "Queen"}, {5, "Drone"},
    {6, "Zergling"}, {7, "Lurker"},
    {8, "Broodling"}, {9, "Hydralisk"},
    {10, "Guardian"}, {11, "Scourge"},
    {12, "Ultralisk"}, {13, "Mutalisk"},
    {14, "Defiler"}, {15, "Devourer"},
};
#endif

void z_status_parse(FILE *fp, ZergHeader_t *zh, ZergBlock_t *zb)
{
    int len = 0;
    ZergStatPayload_t zsp;

    len = NTOH3(zh->zh_len);
    len -= ZERG_SIZE;
#ifdef DEBUG
    int hp = 0;
    unsigned int max_hp = 0;
    char name[128];
    printf("DEBUG: ZERG V 1 // TYPE 1\n");
    printf("DEBUG: PAYLOAD IS %d\n", len);
#endif

    (void) fread(&zsp, sizeof(zsp), 1, fp);
    if (zsp.zsp_ztype > 15) {
        fprintf(stderr, "Unknown breed\n");
        return;
    }

#ifdef DEBUG
    hp = NTOH3(zsp.zsp_hp);
    max_hp = NTOH3(zsp.zsp_maxhp);
    printf("DEBUG: HP IS: %d\n", hp);
    printf("DEBUG: ZERG TYPE IS: %d\n", zsp.zsp_ztype);
    printf("DEBUG: NAME LENGTH IS: %d\n", len - ZERG_STAT_LEN);
    printf("HP : %d\n", hp);
    printf("Max-HP : %u\n", max_hp);
    printf("Type : %s\n", breeds[zsp.zsp_ztype].data);
    printf("Armor : %u\n", zsp.zsp_armor);
    printf("Speed(m/s) : %6.4f\n", ieee_convert32(ntohl(zsp.zsp_speed)));
    (void) fread(name, sizeof(char), len - ZERG_STAT_LEN, fp);
    printf("Name : ");
    for (int i = 0; i < len - ZERG_STAT_LEN; i++)
        putchar(name[i]);

    putchar('\n');
#endif

#ifndef DEBUG
    (void) fseek(fp, len - ZERG_STAT_LEN, SEEK_CUR);
#endif

    /* Fill ZergBlock here */
    zb->z_id = zh->zh_src;
    zb->z_hp[0] = zsp.zsp_hp[0];
    zb->z_hp[1] = zsp.zsp_hp[1];
    zb->z_hp[2] = zsp.zsp_hp[2];
    zb->z_maxhp[0] = zsp.zsp_maxhp[0];
    zb->z_maxhp[1] = zsp.zsp_maxhp[1];
    zb->z_maxhp[2] = zsp.zsp_maxhp[2];

    return;
}

void z_gps_parse(FILE *fp, ZergHeader_t *zh, ZergBlock_t *zb)
{
    int len = 0;
    ZergGpsPayload_t zgp;

    len = NTOH3(zh->zh_len);
    len -= ZERG_SIZE;
#ifdef DEBUG
    double longitude, latitude;
    int degrees, minutes;
    float seconds;
    printf("DEBUG ZERG V 1 // TYPE 3\n");
    printf("DEBUG: PAYLOAD IS %d\n", len);
#endif

    (void) fread(&zgp, len, 1, fp);

#ifdef DEBUG
    longitude = ieee_convert64(ntoh64(zgp.zgp_long));
    latitude = ieee_convert64(ntoh64(zgp.zgp_lat));
    printf("Longitude : %6.4f deg\n", longitude);
    degrees = (int) longitude;
    minutes = (int) ((longitude - degrees) * 60);
    seconds = (((longitude)) - degrees - (float) minutes / 60) * 3600;
    printf("%d° %d' %6.4f\"\n", degrees, minutes, seconds);
    printf("Latitude : %6.4f deg\n", latitude);
    degrees = (int) latitude;
    minutes = (int) ((latitude - degrees) * 60);
    seconds = (((latitude)) - degrees - (float) minutes / 60) * 3600;
    printf("%d° %d' %6.4f\"\n", degrees, minutes, seconds);
    if (zgp.zgp_alt)
        printf("Altitude : %6.4f m\n", ieee_convert32(ntohl(zgp.zgp_alt)));
    else
        printf("Altitude : %6.4f m\n", 0.0);
    printf("Bearing : %6.4f deg\n", ieee_convert32(ntohl(zgp.zgp_bearing)));
    printf("Speed : %6.4f m/s\n", ieee_convert32(ntohl(zgp.zgp_speed)));
    printf("Accuracy : %6.4f m\n", ieee_convert32(ntohl(zgp.zgp_acc)));
#endif

    /* Fill ZergBlock here */
    zb->z_id = zh->zh_src;
    zb->z_long = zgp.zgp_long;
    zb->z_lat = zgp.zgp_lat;
    zb->z_alt = zgp.zgp_alt;

    return;
}
