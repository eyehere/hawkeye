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
#include "load_stat.h"

int load_stat_get(load_stat_t *load_stat)
{
    FILE *fp;
    int load_tmp[3];
    unsigned int avg_1  = 0;
    unsigned int avg_5  = 0;
    unsigned int avg_15 = 0;

    bzero(load_stat, sizeof(load_stat_t));

    if (NULL == (fp = fopen(PROC_LOAD_STAT, "r"))) {
        log_error("open %s failed, error: %s.", PROC_LOAD_STAT, strerror(errno));
        return 0;
    }

    /* Read load averages and queue length */
    fscanf(fp, "%d.%d %d.%d %d.%d %ld/%d %*d\n",
            &load_tmp[0], &avg_1,
            &load_tmp[1], &avg_5,
            &load_tmp[2], &avg_15,
            &load_stat->nr_running,
            &load_stat->nr_threads);

    load_stat->avg_1  = (float)(load_tmp[0] * 100 + avg_1)/100;
    load_stat->avg_5  = (float)(load_tmp[1] * 100 + avg_5)/100;
    load_stat->avg_15 = (float)(load_tmp[2] * 100 + avg_15)/100;

    if (load_stat->nr_running) {
        /* Do not take current process into account */
        load_stat->nr_running--;
    }
    
    fclose(fp);
    return 1;
}

int load_stat_json(load_stat_t *load_stat, char *json_buffer, int size, int cpu_num)
{
    int len = 0;
    char *fmt_str = "load={\"avg_1\":%.2f,\"avg_5\":%.2f,\"avg_15\":%.2f}";

    bzero(json_buffer, size);
    len = snprintf(json_buffer, size, fmt_str, 
                   load_stat->avg_1/cpu_num, load_stat->avg_5/cpu_num, load_stat->avg_15/cpu_num); 
    if (len < size) {
        return 1;
    }
    else {
        log_error("load_stat_json: json_buffer is too small.");
        return 0;
    }
}

void load_stat_dump(load_stat_t *load_stat)
{
    log_info("load avg stat: %2.2f %2.2f %2.2f", load_stat->avg_1, load_stat->avg_5, load_stat->avg_15);
}

#ifdef _DEBUG
int main(void) 
{
    load_stat_t stat; 

    load_stat_get(&stat);

    printf("%2.2f %2.2f %2.2f\n", stat.avg_1, stat.avg_5, stat.avg_15);
    return 1;
}
#endif
