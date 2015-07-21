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
#include "report.h"

extern hawkeye_config_t *_config;

static char sn[128]   = {0};
static int  sn_getted = 0;
static int get_sn(char *sn, int size);

int report_do(hawkeye_t *hawkeye, int timeout)
{
    CURL           *curl                  = NULL;
    CURLcode        result                = -1;
    char            post_buffer[8 * 1024] = {0};
    int             len                   = 0;

    if (!sn_getted) {
        if (!get_sn(sn, sizeof(sn))) {
            log_error("get_sn failed.");
            return 0;
        }
        sn_getted = 1; 
    }

    curl = curl_easy_init();
    if (NULL == curl) {
        log_error("get curl handle failed.");
        return 0;
    }

    len = snprintf(post_buffer, sizeof(post_buffer), "t=%lu&sn=%s&%s&%s&%s&%s&%s&%s&%s&%s",
                  time(0),
                  sn, 
                  hawkeye->cpu_stat_json,
                  hawkeye->io_stat_json,
                  hawkeye->load_stat_json,
                  hawkeye->memory_stat_json,
                  hawkeye->nginx_stat_json,
                  hawkeye->partition_stat_json,
                  hawkeye->traffic_stat_json,
                  hawkeye->processes_stat_json);
    if (len >= (int)sizeof(post_buffer)) {
        log_error("post_buffer is too small.");
        curl_easy_cleanup(curl);
        return 0;
    }

    //printf("-------------post: len[%d], buffer: %s\n", len, post_buffer);
    if (_config->proxy_server) {
        /*char proxy_server[1024] = {0}; 
        snprintf(proxy_server, sizeof(proxy_server) - 1, "%s:%d", _config->proxy_server, _config->proxy_port);
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_server);*/

        curl_easy_setopt(curl, CURLOPT_PROXY, _config->proxy_server);
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, _config->proxy_port);
    }

    if (_config->proxy_user) {
        char proxy_user[1024] = {0};
        snprintf(proxy_user, sizeof(proxy_user) - 1, "%s:%s", _config->proxy_user, _config->proxy_password);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_user);
    }

    curl_easy_setopt(curl, CURLOPT_URL, _config->status_report_url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout/1000);
    curl_easy_setopt(curl, CURLOPT_POST, 8);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_buffer);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        log_error("report to: %s failed, %s.", _config->status_report_url, curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_cleanup(curl);
    return 1;
}

static int get_sn(char *sn, int size)
{
    FILE *fp = NULL;
    char line[1024];

    fp = popen("dmidecode -t1", "r");
    if (NULL == fp) {
        log_error("popen dmidecode -t1 failed.");
        return 0;
    }

    bzero(line, sizeof(line));
    while (NULL != fgets(line, sizeof(line) - 1, fp)) {
        char *str = NULL;
        if (NULL != (str = strstr(line, "Serial Number: "))) {
            strncpy(sn, str + sizeof("Serial Number:"), size - 1);

            str = sn;
            while (*str) {
                if (*str == '\n' || *str == '\r') {
                    *str = 0;
                    break;
                }
                str++;
            }

            pclose(fp);
            return 1;
        }

        bzero(line, sizeof(line));
    }

    pclose(fp);
    return 0;
}
