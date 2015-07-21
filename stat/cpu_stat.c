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
#include "cpu_stat.h"

typedef struct _cpu_usage_t {
    float user_usage;
    float nice_usage;
    float sys_usage;
    float iowait_usage;
    float steal_usage;
    float idle_usage;
    float usage;
} cpu_usage_t;

static int cpu_usage_calculate(cpu_stat_t *stat1, cpu_stat_t *stat2, cpu_usage_t *usage);

int cpu_stat_get(cpu_stat_t *cpu_stat)
{
    FILE *fp;
    char line[4096] = {0};

    bzero(cpu_stat, sizeof(cpu_stat_t));

    if ((fp = fopen(PROC_CPU_STAT, "r")) == NULL) {
        log_error("open %s failed, error: %s.", PROC_CPU_STAT, strerror(errno));
        return 0;
    }

    while (fgets(line, sizeof(line) - 1, fp) != NULL) {
        /* the first line */
        if (0 == strncmp(line, "cpu ", 4)) {
            sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &cpu_stat->user,
                    &cpu_stat->nice,
                    &cpu_stat->sys,
                    &cpu_stat->idle,
                    &cpu_stat->iowait,
                    &cpu_stat->hardirq,
                    &cpu_stat->softirq,
                    &cpu_stat->steal,
                    &cpu_stat->guest);

            cpu_stat->total = cpu_stat->user   +
                              cpu_stat->nice   +
                              cpu_stat->sys    +
                              cpu_stat->iowait +
                              cpu_stat->steal  +
                              cpu_stat->idle;

            cpu_stat->busy  = cpu_stat->user   +
                              cpu_stat->nice   +
                              cpu_stat->sys    +
                              cpu_stat->iowait +
                              cpu_stat->steal;
        }

        if (0 == strncmp(line, "cpu", 3)) {
            cpu_stat->cpu_num++;
        }

        bzero(line, sizeof(line));
    }

    cpu_stat->cpu_num--;
    fclose(fp);
    return 1;
}

int cpu_stat_json(cpu_stat_t *stat1, cpu_stat_t *stat2, char *json_buffer, int size)
{
    cpu_usage_t usage;
    int len = 0;

    cpu_usage_calculate(stat1, stat2, &usage);
    bzero(json_buffer, size);

    len = snprintf(json_buffer, size, "cpu={\"count\":%d,"
                                    "\"user\":%.3f,"
                                    "\"nice\":%.3f,"
                                    "\"sys\":%.3f,"
                                    "\"iowait\":%.3f,"
                                    "\"steal\":%.3f,"
                                    "\"idle\":%.3f,"
                                    "\"usage\":%.3f}",
                                    stat1->cpu_num,
                                    usage.user_usage,
                                    usage.nice_usage,
                                    usage.sys_usage,
                                    usage.iowait_usage,
                                    usage.steal_usage,
                                    usage.idle_usage,
                                    usage.usage); 
    if (len < size) {
        return 1;
    }
    else {
        log_error("cpu_stat_json: json_buffer is too small.");
        return 0;
    }
}

static int cpu_usage_calculate(cpu_stat_t *stat1, cpu_stat_t *stat2, cpu_usage_t *usage)
{
    long long total_delta;
    long long user_delta;
    long long nice_delta;
    long long sys_delta;
    long long idle_delta;
    long long iowait_delta;
    long long steal_delta;
    long long busy_delta;

    bzero(usage, sizeof(cpu_usage_t));

    total_delta = stat2->total - stat1->total;
    if (total_delta == 0) {
        log_error("cpu stat, total == 0.");
        return 0;
    }

    if (total_delta < 0) {
        log_error("cpu stat rotate, stat2->total: %llu, stat1->total: %llu.", 
                  stat2->total, stat1->total);
        return 0;
    }
      
    user_delta = stat2->user - stat1->user;
    if (user_delta < 0) {
        log_error("cpu stat rotate, stat2->user: %llu, stat1->user: %llu.",
                  stat2->user, stat1->user);
        user_delta = 0;
    }
    usage->user_usage = (float)user_delta/(float)total_delta;

    nice_delta = stat2->nice - stat1->nice;
    if (nice_delta < 0) {
        log_error("cpu stat rotate, stat2->nice: %llu, stat1->nice: %llu.",
                  stat2->nice, stat1->nice);
        nice_delta = 0;
    }
    usage->nice_usage = (float)nice_delta/(float)total_delta;

    sys_delta = stat2->sys - stat1->sys;
    if (sys_delta < 0) {
        log_error("cpu stat rotate, stat2->sys: %llu, stat1->sys: %llu.",
                  stat2->sys, stat1->sys);
        sys_delta = 0;
    }
    usage->sys_usage = (float)sys_delta/(float)total_delta;

    idle_delta = stat2->idle - stat1->idle;
    if (idle_delta < 0) {
        log_error("cpu stat rotate, stat2->idle: %llu, stat1->idle: %llu.",
                  stat2->idle, stat1->idle);
        idle_delta = 0;
    }
    usage->idle_usage = (float)idle_delta/(float)total_delta;

    iowait_delta = stat2->iowait - stat1->iowait; 
    if (iowait_delta < 0) {
        log_error("cpu stat rotate, stat2->iowait: %llu, stat1->iowait: %llu.",
                  stat2->iowait, stat1->iowait);
        iowait_delta = 0;
    }
    usage->iowait_usage = (float)iowait_delta/(float)total_delta;

    steal_delta = stat2->steal - stat1->steal;
    if (steal_delta < 0) {
        log_error("cpu stat rotate, stat2->steal: %llu, stat1->steal: %llu.",
                  stat2->steal, stat1->steal);
        steal_delta = 0;
    }
    usage->steal_usage = (float)steal_delta/(float)total_delta;

    busy_delta  = stat2->busy  - stat1->busy;
    if (busy_delta < 0) {
        log_error("cpu stat rotate, stat2->busy: %llu, stat1->busy: %llu.",
                  stat2->busy, stat1->busy);
        busy_delta = 0;
    }
    usage->usage = (float)busy_delta/(float)total_delta;

    return 1;
}

void cpu_stat_dump(cpu_stat_t *stat)
{
    log_info("cpu stat: ");
    log_info("cpu core num:  %d", stat->cpu_num);
    log_info("cpu user:      %d", stat->user);
    log_info("cpu nice:      %d", stat->nice);
    log_info("cpu sys:       %d", stat->sys);
    log_info("cpu idle:      %d", stat->idle);
    log_info("cpu total:     %d", stat->total);
    log_info("cpu busy:      %d", stat->busy);
}

#ifdef _DEBUG

int main(void) 
{
    char buffer[4096] = {0};
    cpu_stat_t stat1, stat2; 
    
    cpu_stat_get(&stat1);
    sleep(1);
    cpu_stat_get(&stat2);

    cpu_stat_json(&stat1, &stat2, buffer, sizeof(buffer));

    printf("%s\n");

    return 1;
}

#endif
