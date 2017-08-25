#include "zerg.h"

void z_status_parse(FILE *fp, ZergHeader_t *zh, ZergBlock_t *zb)
{
    int len = 0;
    ZergStatPayload_t zsp;

    len = NTOH3(zh->zh_len);
    len -= ZERG_SIZE;

    (void) fread(&zsp, sizeof(zsp), 1, fp);
    if (zsp.zsp_ztype > 15) {
        fprintf(stderr, "Unknown breed\n");
        return;
    }

    (void) fseek(fp, len - ZERG_STAT_LEN, SEEK_CUR);

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

    (void) fread(&zgp, len, 1, fp);

    /* Fill ZergBlock here */
    zb->z_id = zh->zh_src;
    zb->z_long = zgp.zgp_long;
    zb->z_lat = zgp.zgp_lat;
    zb->z_alt = zgp.zgp_alt;

    return;
}
