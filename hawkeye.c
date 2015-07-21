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
#include <easy/easy.h>
#include "hawkeye.h"
#include "hawkeye_config.h"

extern hawkeye_config_t *_config;

hawkeye_t *hawkeye_create()
{
    hawkeye_t *hawkeye = NULL;

    hawkeye = (hawkeye_t *)malloc(sizeof(hawkeye_t));
    if (NULL == hawkeye) {
        log_error("no enough memory for hawkeye.");
        return NULL;
    }
    bzero(hawkeye, sizeof(hawkeye_t));

    return hawkeye;
}

int hawkeye_do(hawkeye_t *hawkeye)
{
    cpu_stat_t       cpu_stat1, cpu_stat2;
    io_stat_t        io_stat1, io_stat2;
    load_stat_t      load_stat;
    memory_stat_t    memory_stat; 
    nginx_stat_t     nginx_stat;
    partition_stat_t partition_stat;
    traffic_stat_t   traffic_stat1, traffic_stat2;
    processes_stat_t processes_stat;

    bzero(hawkeye, sizeof(hawkeye_t));

    if (!cpu_stat_get(&cpu_stat1)) {
        log_error("cpu_stat_get failed.");
        return 0;
    }

    if (!io_stat_get(&io_stat1)) {
        log_error("io_stat_get failed.");
        return 0;
    }

    if (!traffic_stat_get(&traffic_stat1)) {
        log_error("traffic_stat_get failed.");
        return 0;
    }

    sleep(1);

    if (!cpu_stat_get(&cpu_stat2)) {
        log_error("cpu_stat_get failed.");
        return 0;
    }

    if (!io_stat_get(&io_stat2)) {
        log_error("io_stat_get failed.");
        return 0;
    }

    if (!load_stat_get(&load_stat)) {
        log_error("load_stat_get failed."); 
        return 0;
    }

    if (!memory_stat_get(&memory_stat)) {
        log_error("memory_stat_get failed.");
        return 0;
    }

    if (!nginx_stat_get(&nginx_stat, _config->nginx_status_req_url, _config->http_timeout)) {
        log_error("nginx_stat_get failed.");
        return 0;
    }

    if (!partition_stat_get(&partition_stat)) {
        log_error("partition_stat_get failed.");
        return 0;
    }

    if (!traffic_stat_get(&traffic_stat2)) {
        log_error("traffic_stat_get failed.");
        return 0;
    }

    if (!processes_stat_get(&processes_stat, _config->monitor_processes, _config->process_count)) {
        log_error("processes_stat_get failed.");
        return 0;
    }

    if (!cpu_stat_json(&cpu_stat1, &cpu_stat2, hawkeye->cpu_stat_json, sizeof(hawkeye->cpu_stat_json))) {
        log_error("cpu_stat_json failed.");
        return 0;
    }
    printf("cpu_stat_json: %s\n", hawkeye->cpu_stat_json);

    if (!io_stat_json(&io_stat1, &io_stat2, hawkeye->io_stat_json, sizeof(hawkeye->io_stat_json))) {
        log_error("io_stat_json failed.");
        return 0;
    }
    printf("io_stat_json: %s\n", hawkeye->io_stat_json);

    if (!load_stat_json(&load_stat, hawkeye->load_stat_json, sizeof(hawkeye->load_stat_json), cpu_stat1.cpu_num)) {
        log_error("load_stat_json failed.");
        return 0;
    }
    printf("load_stat_json: %s\n", hawkeye->load_stat_json);

    if (!memory_stat_json(&memory_stat, hawkeye->memory_stat_json, sizeof(hawkeye->memory_stat_json))) {
        log_error("memory_stat_json failed.");
        return 0;
    }
    printf("memory_stat_json: %s\n", hawkeye->memory_stat_json);

    if (!nginx_stat_json(&nginx_stat, hawkeye->nginx_stat_json, sizeof(hawkeye->nginx_stat_json))) {
        log_error("nginx_stat_json failed.");
        return 0;
    }
    printf("nginx_stat_json: %s\n", hawkeye->nginx_stat_json);

    if (!partition_stat_json(&partition_stat, hawkeye->partition_stat_json, sizeof(hawkeye->partition_stat_json))) {
        log_error("partition_stat_json failed.");
        return 0;
    }
    printf("partition_stat_json: %s\n", hawkeye->partition_stat_json);

    if (!traffic_stat_json(&traffic_stat1, &traffic_stat2, hawkeye->traffic_stat_json, sizeof(hawkeye->traffic_stat_json))) {
        log_error("traffic_stat_json failed.");
        return 0;
    }
    printf("traffic_stat_json: %s\n", hawkeye->traffic_stat_json);

    if (!processes_stat_json(&processes_stat, hawkeye->processes_stat_json, sizeof(hawkeye->processes_stat_json))) {
        log_error("processes_stat_json failed.");
        return 0;
    }
    printf("processes_stat_json: %s\n", hawkeye->processes_stat_json);

    return 1;
}

void hawkeye_destroy(hawkeye_t *hawkeye)
{
    if (NULL == hawkeye) {
        return;
    }

    free(hawkeye);
}

