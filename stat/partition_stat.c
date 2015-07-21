/*
  +----------------------------------------------------------------------+
  | hawkeye                                                              |
  +----------------------------------------------------------------------+
  | this source file is subject to version 2.0 of the apache license,    |
  | that is bundled with this package in the file license, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/license-2.0.html                      |
  | if you did not receive a copy of the apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | yiming_6weijun@163.com so we can mail you a copy immediately.        |
  +----------------------------------------------------------------------+
  | author: weijun lu  <yiming_6weijun@163.com>                          |
  +----------------------------------------------------------------------+
*/
#include "partition_stat.h"

#define K ((long long)1024)
#define M ((long long)1024 * (long long)1024)
#define G ((long long)1024 * (long long)1024 * (long long)1024)
#define T ((long long)1024 * (long long)1024 * (long long)1024 * (long long)1024)

static int digital_buffer(char *buffer, int size, unsigned long long digital);
static int disk_status(char *mnt_dir);

int partition_stat_get(partition_stat_t *stat)
{
    FILE *mnt_file     = NULL;
    struct mntent *mnt = NULL;
    struct statfs fsbuf;

    bzero(stat, sizeof(partition_stat_t));

    mnt_file = setmntent(MTAB_FILE, "r");
    if (NULL == mnt_file) {
        log_error("setmntext failed.");
        return 0;
    }

    while (NULL != (mnt = getmntent(mnt_file))) {
        if (stat->count >= MAX_PARTITIOn_COUNT) {
            log_warn("partition count >= %d.", MAX_PARTITIOn_COUNT);
            break;
        }

        if(strncmp(mnt->mnt_fsname, "/", 1)) {
            continue;
        }

        strncpy(stat->partitions[stat->count].fs_name, mnt->mnt_fsname + 5, MAX_FS_NAME_SIZE - 1);
        strncpy(stat->partitions[stat->count].mnt_dir, mnt->mnt_dir, MAX_MNT_DIR_SIZE - 1);

        if (!statfs(mnt->mnt_dir, &fsbuf)) {
            stat->partitions[stat->count].type   = fsbuf.f_type;
            stat->partitions[stat->count].bsize  = fsbuf.f_bsize;
            stat->partitions[stat->count].blocks = fsbuf.f_blocks;
            stat->partitions[stat->count].bfree  = fsbuf.f_bfree;
            stat->partitions[stat->count].bavail = fsbuf.f_bavail;
            stat->partitions[stat->count].status = disk_status(mnt->mnt_dir);
        }
        else {
            log_error("statfs for %s failed, %s.", mnt->mnt_dir); 
            stat->partitions[stat->count].status = PARTITION_STATUS_ERROR;
        }

        stat->partitions[stat->count].total_size         = (fsbuf.f_bsize/1024) * (fsbuf.f_blocks/1024); 
        stat->partitions[stat->count].nonroot_total_size = 
            (fsbuf.f_bsize/1024) * ((fsbuf.f_blocks - fsbuf.f_bfree + fsbuf.f_bavail)/1024);
        stat->partitions[stat->count].used_size          = (fsbuf.f_bsize/1024) * ((fsbuf.f_blocks - fsbuf.f_bfree)/1024);
        stat->partitions[stat->count].avail_size         = (fsbuf.f_bsize/1024) * (fsbuf.f_bavail/1024);

        if (stat->partitions[stat->count].nonroot_total_size != 0) {
            stat->partitions[stat->count].usage = 
                (float)stat->partitions[stat->count].used_size/(float)stat->partitions[stat->count].nonroot_total_size;
        }
        else {
            stat->partitions[stat->count].usage = 0;
        }

        stat->nonroot_total_size += stat->partitions[stat->count].nonroot_total_size;
        stat->total_size         += stat->partitions[stat->count].total_size;
        stat->used_size          += stat->partitions[stat->count].used_size;
        stat->avail_size         += stat->partitions[stat->count].avail_size;

        stat->count++;
    }

    stat->usage = (float)stat->used_size/(float)stat->nonroot_total_size;
    endmntent(mnt_file);

    return 1;
}

int partition_stat_json(partition_stat_t *stat, char *json_buffer, int size)
{
    int i = 0; 
    int offset = 0;
    int else_size = size;
    char tmp_buffer[1024] = {0};
    char *fmt_str = NULL;
    int len = 0;

    bzero(json_buffer, size);
    for (i = 0; i < stat->count; i++) {
        bzero(tmp_buffer, sizeof(tmp_buffer));
        if (i == 0) {
            fmt_str = "disk={\"%d\":{\"name\":\"%s\",\"mnt_dir\":\"%s\",\"total\":%lu,\"used\":%lu,\"avail\":%lu,"
                      "\"usage\":%.3f,\"status\":%d}";
        }
        else {
            fmt_str = ",\"%d\":{\"name\":\"%s\",\"mnt_dir\":\"%s\",\"total\":%lu,\"used\":%lu,\"avail\":%lu,"
                      "\"usage\":%.3f,\"status\":%d}";
        }
        
        len = snprintf(tmp_buffer, sizeof(tmp_buffer) - 1, fmt_str,
                       i + 1, 
                       stat->partitions[i].fs_name,
                       stat->partitions[i].mnt_dir,
                       stat->partitions[i].total_size,
                       stat->partitions[i].used_size,
                       stat->partitions[i].avail_size,
                       stat->partitions[i].usage,
                       stat->partitions[i].status);
        if (len >= else_size) {
            log_error("partition_stat_json: json_buffer is too small.");
            return 0;
        }

        {
            char total_buffer[128] = {0};
            char used_buffer[128]  = {0};
            char avail_buffer[128] = {0};

            digital_buffer(total_buffer, sizeof(total_buffer), stat->partitions[i].total_size);
            digital_buffer(used_buffer,  sizeof(used_buffer),  stat->partitions[i].used_size);
            digital_buffer(avail_buffer, sizeof(avail_buffer), stat->partitions[i].avail_size);

            //dev size used avail usage  mnt_dir
            log_debug("%-10s %-10s %-10s %-10s %.1f%% %-10s", 
                stat->partitions[i].fs_name,
                total_buffer, used_buffer, avail_buffer, 
                stat->partitions[i].usage * 100,
                stat->partitions[i].mnt_dir);
        } 

        memcpy(json_buffer + offset, tmp_buffer, len);
        offset    += len;
        else_size -= len;
    }

    bzero(tmp_buffer, sizeof(tmp_buffer)); 
    fmt_str = ",\"total\":%lu,\"used\":%lu,\"avail\":%lu,\"usage\":%.3f}"; 

    len = snprintf(tmp_buffer, sizeof(tmp_buffer) - 1, fmt_str, 
                  stat->total_size,
                  stat->used_size,
                  stat->avail_size,
                  stat->usage);
    if (len >= else_size) {
        log_error("partition_stat_json: json_buffer is too small.");
        return 0;
    }
    memcpy(json_buffer + offset, tmp_buffer, len);
    offset    += len;
    else_size -= len;

    return 1;
}

static int digital_buffer(char *buffer, int size, unsigned long long digital)
{
    char *unit = NULL; 
    double number = 0;

    if (digital * M >= T) {
        unit = "T";
        number = (double)digital/(double)M;
    }
    else if (digital * M < T && digital * M >= G) {
        unit = "G";
        number = (double)digital/(double)K;
    }
    else if (digital * M < G && digital * M >= M) {
        unit = "M";
        number = (double)digital;
    }
    else {
        unit = "";
        number = (double)digital;
    }

    snprintf(buffer, size - 1, "%.1f%s", number, unit);
    return 1;
}

static int disk_status(char *mnt_dir)
{
    FILE *fp = NULL;

    fp = fopen(mnt_dir, "r");
    if (fp == NULL) {
        log_error("fopen failed, error: ", strerror(errno));
        return PARTITION_STATUS_ERROR;
    }

    fclose(fp);
    return PARTITION_STATUS_OK;
}

#ifdef _DEBUG

int main(void) 
{
    int i = 0;
    partition_stat_t stat; 
    char total_buffer[128];
    char used_buffer[128];
    char avail_buffer[128];
    float  used_usage;

    partition_stat_get(&stat);

    printf("%-10s %-10s %-10s %-10s %-10s %-10s\n", 
                "fs_name", 
                "mnt_dir", 
                "total", "used", "avail", "usage");

    for (i = 0; i < stat.count; i++) {
        bzero(total_buffer, sizeof(total_buffer));
        bzero(used_buffer, sizeof(used_buffer));
        bzero(avail_buffer, sizeof(avail_buffer));

        digital_buffer(total_buffer, sizeof(total_buffer), stat.partitions[i].total_size);
        digital_buffer(used_buffer, sizeof(used_buffer), stat.partitions[i].used_size);
        digital_buffer(avail_buffer, sizeof(avail_buffer), stat.partitions[i].avail_size);

        printf("%-10s %-10s %-10s %-10s %-10s %.1f%\n", 
                stat.partitions[i].fs_name, 
                stat.partitions[i].mnt_dir, 
                total_buffer, used_buffer, avail_buffer, 
                stat.partitions[i].usage);
    }
}

#endif
