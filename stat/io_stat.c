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
#include "io_stat.h"

static int io_stat_usage_calculate(io_stat_t *stat1, io_stat_t *stat2, io_usage_t *usage);

int io_stat_get(io_stat_t *io_stat)
{
    FILE *fp        = NULL;
    char line[4096] = {0};
    char *scan_fmt  = NULL;
    int  items      = 0;

    scan_fmt = "%4d %4d %s %u %u %llu %u %u %u %llu %u %*u %u %u";
    bzero(io_stat, sizeof(io_stat_t));

    fp = fopen(DISK_STAT_FILE, "r");
    if (NULL == fp) {
        log_error("fopen %s failed, error: %s.", DISK_STAT_FILE, strerror(errno));
        return 0;
    }

    while (fgets(line, sizeof(line) - 1, fp) != NULL) {
        io_t io;

        if (io_stat->count >= MAX_IO_COUNT) {
            log_error("io_stat->count >= %d.", MAX_IO_COUNT);
            break;
        }

        bzero(&io, sizeof(io_t));
        items = sscanf(line, scan_fmt,
                &io.major,  &io.minor,     &io.dev_name,
                &io.rd_ios, &io.rd_merges, &io.rd_sectors, &io.rd_ticks,
                &io.wr_ios, &io.wr_merges, &io.wr_sectors, &io.wr_ticks,
                &io.ticks,  &io.aveq);

        if (io.rd_ios == 0 && io.rd_merges == 0 && io.rd_sectors == 0 && io.rd_ticks == 0 &&
            io.wr_ios == 0 && io.wr_merges == 0 && io.wr_sectors == 0 && io.wr_ticks == 0) {
            continue;
        }

        /*if (0 != strncmp(io.dev_name, "sd", 2)) {
            continue;
        }*/

        /*if (isdigit(io.dev_name[3])) {
            continue;
        }*/

        /*   
         * Unfortunately, we can report only transfer rates
         * for partitions in 2.6 kernels, all other I/O
         * statistics are unavailable.
         */
        if (items == 7) {
            io.rd_sectors = io.rd_merges;
            io.wr_sectors = io.rd_ticks;
            io.rd_ios = 0;
            io.rd_merges = 0;
            io.rd_ticks = 0;
            io.wr_ios = 0;
            io.wr_merges = 0;
            io.wr_ticks = 0;
            io.ticks = 0;
            io.aveq = 0;
            items = 13; 
            log_warn("can't get 12 items for diststas.");
        }

        if (13 != items) {
            bzero(line, sizeof(line));
            continue;
        }

        gettimeofday(&io.t, NULL);
        memcpy(&io_stat->ios[io_stat->count], &io, sizeof(io_t));
        io_stat->count++;
        bzero(line, sizeof(line));
    }
    
    fclose(fp);
    return 1;
}

int io_stat_json(io_stat_t *stat1, io_stat_t *stat2, char *json_buffer, int size)
{
    int i         = 0;
    int offset    = 0;
    int else_size = size;
    io_usage_t usage;

    bzero(json_buffer, size);
    io_stat_usage_calculate(stat1, stat2, &usage);

    for (i = 0; i < usage.count; i++) {
        char tmp_buffer[1024] = {0};
        char *fmt_str = NULL;
        int  len = 0;

        if (i == 0) {
            fmt_str = "io={\"%d\":{\"name\":\"%s\",\"usage\":%.3f}";
        }
        else if(i == usage.count - 1) {
            fmt_str = ",\"%d\":{\"name\":\"%s\",\"usage\":%.3f}}";
        }
        else {
            fmt_str = ",\"%d\":{\"name\":\"%s\",\"usage\":%.3f}";
        }

        len = snprintf(tmp_buffer, sizeof(tmp_buffer) - 1, fmt_str, i + 1, usage.dev_name[i], usage.usage[i]);
        if (len >= else_size) {
            log_error("io_stat_json json_buffer is too small."); 
            return 0;
        }
        memcpy(json_buffer + offset, tmp_buffer, len);
        offset    += len;
        else_size -= len;
    }

    return 1;
}

static int io_stat_usage_calculate(io_stat_t *stat1, io_stat_t *stat2, io_usage_t *usage)
{
    int i = 0, j = 0; 
    io_t *io1, *io2;

    bzero(usage, sizeof(io_usage_t)); 

    for (i = 0; i < stat1->count; i++) {
        io1 = &stat1->ios[i];
        io2 = NULL;

        if (usage->count >= MAX_IO_COUNT) {
            log_error("usage->count >= %d.", MAX_IO_COUNT); 
            return 1;
        }
          
        for (j = 0; j < stat2->count; j++) {
            if (0 == strcmp(io1->dev_name, stat2->ios[j].dev_name)) {
                io2 = &stat2->ios[j];
                break;
            }
        }

        if (io2) {
            long long tick_delta = 0;

            int time_delta = (io2->t.tv_sec  - io1->t.tv_sec) * 1000 + 
                             (io2->t.tv_usec - io1->t.tv_usec)/1000;
            if (time_delta <= 0) {
                log_error("io_stat1 and io_stat2 has the same time.");
                return 0;
            }

            tick_delta = io2->ticks - io1->ticks;
            if (tick_delta < 0) {
                log_error("ticks rotate, io2->ticks: %llu, io1->ticks: %llu.",
                          io2->ticks, io1->ticks);
                tick_delta = 0;
            }

            strcpy(usage->dev_name[i], io1->dev_name); 
            usage->usage[i] = (float)tick_delta/(float)time_delta;
            if (usage->usage[i] > 1.0) {
                usage->usage[i] = 1.0;
            }
            usage->count++;
        }
    }

    return 1;
}

void io_stat_usage_dump(io_usage_t *usage)
{
    int i = 0; 
    float avg_io = 0;

    for (i = 0; i < usage->count; i++) {
        printf("%s usage: %.2f%%\n", usage->dev_name[i], usage->usage[i] * 100);
        avg_io += usage->usage[i];
    }

    printf("avg usage: %.2f%%\n", (avg_io/usage->count) * 100);
}

#ifdef _DEBUG
int main(void) 
{
    io_stat_t stat1, stat2; 
    io_usage_t usage;
    int i = 0;

    bzero(&stat1, sizeof(io_stat_t));
    bzero(&stat2, sizeof(io_stat_t));

    io_stat_get(&stat1);
    sleep(1);
    io_stat_get(&stat2);

    io_stat_usage_calculate(&stat1, &stat2, &usage); 
    io_stat_usage_dump(&usage); 

    return 1;
}
#endif
