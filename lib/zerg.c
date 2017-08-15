#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "zerg.h"
#include "pcap.h"

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

static double ieee_convert64(uint64_t num)
{
    /* All credit to droberts */
    uint8_t sign;
    uint16_t exponent;
    uint64_t mantisa;
    double result = 0;

    sign = num >> 63;
    exponent = (num >> 52 & 0x7FF) - 1023;
    mantisa = num & 0xFFFFFFFFFFFFF;
    result = (mantisa *pow(2, -52)) + 1;
    result *= pow(1, sign) * pow(2, exponent);
    return result;
}

static double ieee_convert32(uint32_t num)
{
    /* All credit to droberts */
    uint8_t sign, exponent;
    uint32_t mantisa;
    double result = 0;

    sign = num >> 31;
    exponent = (num >> 23 & 0xFF) - 127;
    mantisa = num & 0x7FFFFF;

    result = (mantisa *pow(2, -23)) + 1;
    result *= pow(1, sign) * pow(2, exponent);

    return result;
}

static uint64_t ntoh64(uint64_t val)
{
    /* https://stackoverflow.com/a/2637138/5155574 */
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

void z_status_parse(FILE *fp, ZergHeader_t *zh)
{
    int len = 0;
    int hp = 0;
    unsigned int max_hp = 0;
    char name[128];
    ZergStatPayload_t zsp;

    len = NTOH3(zh->zh_len);
    len -= ZERG_SIZE;
#ifdef DEBUG
    printf("DEBUG: ZERG V 1 // TYPE 1\n");
    printf("DEBUG: PAYLOAD IS %d\n", len);
#endif

    (void) fread(&zsp, sizeof(zsp), 1, fp);
    hp = NTOH3(zsp.zsp_hp);
    max_hp = NTOH3(zsp.zsp_maxhp);
    if (zsp.zsp_ztype > 15) {
        fprintf(stderr, "Unknown breed\n");
        return;
    }

#ifdef DEBUG
    printf("DEBUG: HP IS: %d\n", hp);
    printf("DEBUG: ZERG TYPE IS: %d\n", zsp.zsp_ztype);
    printf("DEBUG: NAME LENGTH IS: %d\n", len - ZERG_STAT_LEN);
#endif
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

    return;
}

void z_gps_parse(FILE *fp, ZergHeader_t *zh)
{
    int len = 0;
    double longitude, latitude;
    int degrees, minutes;
    float seconds;
    ZergGpsPayload_t zgp;

    len = NTOH3(zh->zh_len);
    len -= ZERG_SIZE;
#ifdef DEBUG
    printf("DEBUG ZERG V 1 // TYPE 3\n");
    printf("DEBUG: PAYLOAD IS %d\n", len);
#endif

    (void) fread(&zgp, len, 1, fp);
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
    return;
}
