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
#include "hawkeye_config.h"

static int hawkeye_config_item_handler(char *key, char *value, void *userp);
static char *log_dst_strs[] = {"console", "file", "syslog"};

hawkeye_config_t *hawkeye_config_load(char *path)
{
    hawkeye_config_t *config = NULL;
    struct stat stat_buf;

    bzero(&stat_buf, sizeof(struct stat));
    if (-1 == stat((const char *)path, &stat_buf)) {
        log_error("config file: %s is not existed.", path);
        return NULL;
    }

    config = (hawkeye_config_t *)malloc(sizeof(hawkeye_config_t)); 
    if (NULL == config) {
        log_error("no enough memory for hawkeye_config_t.");
        return NULL;
    }
    bzero(config, sizeof(hawkeye_config_t));

    if (!property_read(path, hawkeye_config_item_handler, config)) {
        log_error("parse hawkeye config file: %s failed.", path);
        hawkeye_config_free(config);
        return NULL;
    }

    return config;
}

void hawkeye_config_free(hawkeye_config_t *config)
{
    int i = 0;

    if (NULL == config) {
        return;
    }

    if (config->log_file) {
        free(config->log_file);
    }

    if (config->nginx_status_req_url) {
        free(config->nginx_status_req_url);
    }
    
    if (config->status_report_url) {
        free(config->status_report_url);
    }

    for (i = 0; i < config->process_count; i++) {
        free(config->monitor_processes[i]);
    }

    free(config);
}

static int hawkeye_config_item_handler(char *key, char *value, void *userp)
{
    hawkeye_config_t *config = (hawkeye_config_t *)userp;

    log_debug("hawkeye_config_item_handler, %s: %s.", key, value);
    if (0 == strcasecmp(key, "daemon")) {
        if (0 == strcasecmp(value, "yes")) {
            config->daemon = 1;
        }
        else if(0 == strcasecmp(value, "no")) {
            config->daemon = 0;
        }
        else {
            log_error("unknown dameon value: %s.", value);
            return 0;
        }
    }
    else if (0 == strcasecmp(key, "log_level")) {
        config->log_level = log_level_int(value); 

        if (-1 == config->log_level) {
            log_error("unknown log level: %s.", value);
            return 0;
        }
    }
    else if (0 == strcasecmp(key, "log_dst")) {
        if (0 == strcasecmp(value, "console")) {
            config->log_dst = LOG_DST_CONSOLE;
        }
        else if (0 == strcasecmp(value, "file")) {
            config->log_dst = LOG_DST_FILE;
        }
        else {
            log_error("unknown log_dst: %s.", value);
            return 0;
        }
    }
    else if (0 == strcasecmp(key, "log_file")) {
        config->log_file = (char *)malloc(strlen(value) + 1);     
        if (NULL == config->log_file) {
            log_error("no enough memory for config->log_file.");
            return 0;
        }
        bzero(config->log_file, strlen(value) + 1);
        strcpy(config->log_file, value);
    }
    else if (0 == strcasecmp(key, "hawkeye_period")) {
        config->hawkeye_period = atoi(value);

        if (config->hawkeye_period <= 0) {
            log_error("hawkeye_period < 0, %s.", value);
            return 0;
        }
    }
    else if (0 == strcasecmp(key, "http_timeout")) {
        config->http_timeout = atoi(value);

        if (config->http_timeout <= 0) {
            log_error("http_timeout <= 0, %s.", value);
            return 0;
        }
    }
    else if (0 == strcasecmp(key, "nginx_status_req_url")) {
        config->nginx_status_req_url = (char *)malloc(strlen(value) + 1);
        if (NULL == config->nginx_status_req_url) {
            log_error("no enough memory for config->nginx_status_req_url.");
            return 0;
        }
        bzero(config->nginx_status_req_url, strlen(value) + 1);
        strcpy(config->nginx_status_req_url, value);
    }
    else if (0 == strcasecmp(key, "status_report_url")) {
        config->status_report_url = (char *)malloc(strlen(value) + 1);
        if (NULL == config->status_report_url) {
            log_error("no enough memory for config->status_report_url.");
            return 0;
        }
        bzero(config->status_report_url, strlen(value) + 1);
        strcpy(config->status_report_url, value);
    }
    else if (0 == strcasecmp(key, "monitor_processes")) {
        char *p = NULL; 

        p = strtok(value, "|");
        
        while (p) {
            if (config->process_count >= MAX_PROCESS_COUNT) {
                log_error("monitor process count > %d.", MAX_PROCESS_COUNT);
                return 0;
            }

            config->monitor_processes[config->process_count] = (char *)malloc(strlen(p) + 1);
            if (NULL == config->monitor_processes[config->process_count]) {
                log_error("malloc for monitor_processes failed.");
                return 0;
            }
            bzero(config->monitor_processes[config->process_count], strlen(p) + 1);
            strcpy(config->monitor_processes[config->process_count], p);

            config->process_count++;
            p = strtok(NULL, "|");
        }
    }
    else if (0 == strcasecmp(key, "proxy_server")) {
        config->proxy_server = (char *)malloc(strlen(value) + 1);
        if (NULL == config->proxy_server) {
            log_error("no enough memory for config->proxy_server.");
            return 0;
        }
        bzero(config->proxy_server, strlen(value) + 1);
        strcpy(config->proxy_server, value);
    }
    else if (0 == strcasecmp(key, "proxy_user")) {
        config->proxy_user = (char *)malloc(strlen(value) + 1);
        if (NULL == config->proxy_user) {
            log_error("no enough memory for config->proxy_user.");
            return 0;
        }
        bzero(config->proxy_user, strlen(value) + 1);
        strcpy(config->proxy_user, value);
    }
    else if (0 == strcasecmp(key, "proxy_password")) {
        config->proxy_password = (char *)malloc(strlen(value) + 1);
        if (NULL == config->proxy_password) {
            log_error("no enough memory for config->proxy_password.");
            return 0;
        }
        bzero(config->proxy_password, strlen(value) + 1);
        strcpy(config->proxy_password, value);
    }
    else if (0 == strcasecmp(key, "proxy_port")) {
        config->proxy_port = atoi(value);
    }
    else if (0 == strcasecmp(key, "bw_out_ceiling")) {
        config->bw_out_ceiling = atoi(value);
    }
    else if (0 == strcasecmp(key, "nic_bw_usage_ceiling")) {
        config->nic_bw_usage_ceiling = atof(value);
    }
    else{
        log_error("unknown config, %s: %s.", key, value);
        return 0;
    }

    return 1;
}

void hawkeye_config_dump(hawkeye_config_t *config)
{
    int i = 0;
    printf("========================= GLANCE CONFIG ===================\n");
    printf("%-30s%s\n",   "daemon: ",                 config->daemon ? "yes":"no");
    printf("%-30s%s\n",   "log_level: ",              log_level_str(config->log_level));
    printf("%-30s%s\n",   "log_dst: ",                log_dst_strs[config->log_dst]);
    printf("%-30s%s\n",   "log_file: ",               config->log_file);
    printf("%-30s%d\n",   "hawkeye_period: ",          config->hawkeye_period);
    printf("%-30s%d\n",   "http_timeout: ",           config->http_timeout);
    printf("%-30s%s\n",   "nginx_status_req_url: ",   config->nginx_status_req_url);
    printf("%-30s%s\n",   "status_report_url: ",      config->status_report_url);
    printf("%-30s\n",     "monitor_processes: ");
    for (i = 0; i < config->process_count; i++) {
        printf("%-30s%s\n", " ", config->monitor_processes[i]);
    }
    printf("%-30s%s\n",   "proxy_server: ",           config->proxy_server);
    printf("%-30s%d\n",   "proxy_port: ",             config->proxy_port);
    printf("%-30s%s\n",   "proxy_user: ",             config->proxy_user);
    printf("%-30s%s\n",   "proxy_password: ",         config->proxy_password);
    printf("%-30s%dM\n",  "bw_out_ceiling: ",         config->bw_out_ceiling);
    printf("%-30s%f\n",   "nic_bw_usage_ceiling: ",   config->nic_bw_usage_ceiling);
    printf("===========================================================\n");
}
