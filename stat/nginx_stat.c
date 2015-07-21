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
#include "nginx_stat.h"

#define SERVICE_TYPE_UNKNOWN -1
#define SERVICE_TYPE_F4V      0
#define SERVICE_TYPE_TS       1
#define SERVICE_TYPE_M2TS     2
#define SERVICE_TYPE_MP4      3
#define SERVICE_TYPE_LIVE     4

typedef int (*nginx_stat_field_cb_t)(int i, char *field, int size, void *data);

static size_t nginx_stat_write(void *buffer, size_t size, size_t nmemb, void *userp);
static int nginx_stat_parse(buffer_t *buffer, nginx_stat_t *nginx_stat);
static int nginx_stat_line_parse(char *line, void *data, nginx_stat_field_cb_t cb);
static int nginx_stat_field_cb(int i, char *field, int size, void *data);

/* 得到nginx的状态：各个细分业务的带宽，并发连接数 */
int nginx_stat_get(nginx_stat_t *nginx_stat, char *req_url, int timeout)
{
    CURL           *curl         = NULL;
    mem_pool_t     *pool         = NULL;
    buffer_t       *buffer       = NULL;
    CURLcode        result       = -1;

    bzero(nginx_stat, sizeof(nginx_stat_t));
    curl = curl_easy_init();
    if (NULL == curl) {
        log_error("get curl handle failed.");
        return 0;
    }

    pool = mem_pool_create();
    if (NULL == pool) {
        log_error("mem_pool_create for http failed."); 
        curl_easy_cleanup(curl);
        return 0;
    }
    
    buffer = mem_pool_calloc(pool, sizeof(buffer_t));
    if (NULL == buffer) {
        log_error("mem_pool_calloc for nginx stat buffer failed.");
        curl_easy_cleanup(curl);
        mem_pool_destroy(&pool);
        return 0;
    }
    buffer->pool = pool; 

    log_debug("http request url: %s.", req_url);
    curl_easy_setopt(curl, CURLOPT_URL, req_url); 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nginx_stat_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout/1000);
    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        log_error("nginx_stat: %s.", curl_easy_strerror(result)); 
        curl_easy_cleanup(curl);
        mem_pool_destroy(&pool);
        return 0;
    }

    if (!nginx_stat_parse(buffer, nginx_stat)) {
        log_error("nginx_stat_parse failed.");
        curl_easy_cleanup(curl);
        mem_pool_destroy(&pool);
        return 0;
    }

    curl_easy_cleanup(curl);
    mem_pool_destroy(&pool);
    return 1;
}

int nginx_stat_json(nginx_stat_t *nginx_stat, char *json_buffer, int size)
{
    char *fmt_str = NULL;
    int len = 0;
    unsigned long total_bw = 0;
    int total_conn = 0;

    total_bw = nginx_stat->f4v_bw + 
               nginx_stat->mp4_bw +
               nginx_stat->ts_bw  +
               nginx_stat->m2ts_bw;
    total_conn = nginx_stat->f4v_conn +
                 nginx_stat->mp4_conn +
                 nginx_stat->ts_conn  +
                 nginx_stat->m2ts_conn;

    fmt_str = "nginx={\"bw\":{\"total\":%lu,\"f4v\":%lu,\"mp4\":%lu,\"ts\":%lu,\"m2ts\":%lu,\"live\":%lu},"
              "\"conn\":{\"total\":%d,\"f4v\":%d,\"mp4\":%d,\"ts\":%d,\"m2ts\":%d,\"live\":%d}}";
    len = snprintf(json_buffer, size, fmt_str, 
                   total_bw,
                   nginx_stat->f4v_bw,
                   nginx_stat->mp4_bw,
                   nginx_stat->ts_bw,
                   nginx_stat->m2ts_bw,
                   nginx_stat->live_bw,
                   total_conn,
                   nginx_stat->f4v_conn,
                   nginx_stat->mp4_conn,
                   nginx_stat->ts_conn,
                   nginx_stat->m2ts_conn,
                   nginx_stat->live_conn);
    if (len < size) {
        return 1;
    }
    else {
        log_error("nginx_stat_json: json_buffer is too small.");
        return 0;
    }
}

void nginx_stat_dump(nginx_stat_t *nginx_stat)
{
    log_info("nginx stat: ");
    log_info("f4v bandwidth:   %dK", nginx_stat->f4v_bw/1024);
    log_info("mp4 bandwidth:   %dK", nginx_stat->mp4_bw/1024);
    log_info("ts bandwidth:    %dK", nginx_stat->ts_bw/1024);
    log_info("m2ts bandwidth:  %dK", nginx_stat->m2ts_bw/1024);
    log_info("live bandwidth:  %dK", nginx_stat->live_bw/1024);
    log_info("f4v connection:  %d", nginx_stat->f4v_conn);
    log_info("mp4 connection:  %d", nginx_stat->mp4_conn);
    log_info("ts connection:   %d", nginx_stat->ts_conn);
    log_info("m2ts connection: %d", nginx_stat->m2ts_conn);
    log_info("live connection: %d", nginx_stat->live_conn);
}

static size_t nginx_stat_write(void *buffer, size_t size, size_t nmemb, void *userp)
{
    buffer_t *nginx_stat_buffer;

    log_debug("nginx_stat_write, recv data size: %d.", size * nmemb);
      
    nginx_stat_buffer = (buffer_t *)userp;
     
    if (-1 == buffer_write(nginx_stat_buffer, buffer, size * nmemb)) {
        log_error("buffer_write failed.");
        return 0;
    }
    return size * nmemb;
}

static int nginx_stat_parse(buffer_t *buffer, nginx_stat_t *nginx_stat)
{
    char line[1024];
    int  read_len = 0;
    int  title    = 1;

    while (!buffer_eof(buffer)) {
        read_len = buffer_read_line(buffer, line, sizeof(line)); 
        if (0 > read_len) {
            log_error("buffer_read_line failed.");
            return 0;
        }

        if (0 == read_len) {
            continue;
        }

        if (!title && !nginx_stat_line_parse(line, nginx_stat, nginx_stat_field_cb)) {
            log_error("nginx_stat_line_parse failed.");
            return 0;
        }
        title = 0;
    }

    return 1;
}

static int nginx_stat_line_parse(char *line, void *data, nginx_stat_field_cb_t cb)
{
    char *ptr   = line;
    char *start = line;
    int   count = 0;

    while (*ptr != 0) {
        if ((*ptr == ' ' || *ptr == '\t') && start == ptr) {
            ptr++;
            start++;
            continue;
        }

        if (*ptr == ' ' || *ptr == '\t') {
            *ptr = 0;
            if (!cb(count, start, ptr - start, data)) {
                log_error("nginx stat field parse cb failed.");
            }
            start = ptr + 1;
            count++;
        }

        ptr++;
    }
    
    if (ptr > start) {
        if (!cb(count, start, ptr - start, data)) {
            log_error("nginx stat field parse cb failed.");
        }
    }

    return 1;
}

static int service_type = -1;
static int nginx_stat_field_cb(int i, char *field, int size, void *data)
{
    nginx_stat_t *nginx_stat = (nginx_stat_t *)data; 

    //log_debug("field[%d]: %s.", i, field);
    //zone_name,key,max_active,max_bw,traffic,requests,active,bandwidth

    if (i == 0) {            /* zone_name, ignore */
    }
    else if (i == 1) {       /* key, for example: f4v */
        if (0 == strncasecmp(field, "f4v", size)) {
            service_type = SERVICE_TYPE_F4V; 
        }
        else if (0 == strncasecmp(field, "ts", size)) {
            service_type = SERVICE_TYPE_TS; 
        }
        else if (0 == strncasecmp(field, "m2ts", size)) {
            service_type = SERVICE_TYPE_M2TS;
        }
        else if (0 == strncasecmp(field, "mp4", size)) {
            service_type = SERVICE_TYPE_MP4;
        }
        else if (0 == strncasecmp(field, "live", size)) {
            service_type = SERVICE_TYPE_LIVE;
        }
        else {
            log_error("unknown service type: %s.", field);
            return 0;
        }
    }
    else if (i == 2) {       /* max_active */
    }
    else if (i == 3) {       /* max_bw */
    }
    else if (i == 4) {       /* traffic */
    }
    else if (i == 5) {       /* requests */
    }
    else if (i == 6) {       /* active */
        int active = atoi(field);
        if (service_type == SERVICE_TYPE_F4V) {
            nginx_stat->f4v_conn = active;
        }
        else if (service_type == SERVICE_TYPE_TS) {
            nginx_stat->ts_conn = active;
        }
        else if (service_type == SERVICE_TYPE_M2TS) {
            nginx_stat->m2ts_conn = active;
        }
        else if (service_type == SERVICE_TYPE_MP4) {
            nginx_stat->mp4_conn = active;
        }
        else if (service_type == SERVICE_TYPE_LIVE) {
            nginx_stat->live_conn = active;
        }
        else {
            log_error("active: unknown service type: %d.", service_type);
            return 0;
        }
    }
    else if (i == 7) {       /* bandwidth */
        unsigned long bandwidth = 0;

        if (field[size - 1] == 'K' || field[size - 1] == 'k') {
            field[size - 1] = 0;
            bandwidth = atol(field) * 1024;
        }
        else if (field[size - 1] == 'M' || field[size - 1] == 'm') {
            field[size - 1] = 0;
            bandwidth = atol(field) * 1024 * 1024;
        }
        else if (field[size - 1] == 'G' || field[size - 1] == 'g') {
            field[size - 1] = 0;
            bandwidth = atol(field) * 1024 * 1024 * 1024;
        }
        else {
            bandwidth = atol(field);
        }

        if (service_type == SERVICE_TYPE_F4V) {
            nginx_stat->f4v_bw = bandwidth; 
        }
        else if (service_type == SERVICE_TYPE_TS) {
            nginx_stat->ts_bw = bandwidth;
        }
        else if (service_type == SERVICE_TYPE_M2TS) {
            nginx_stat->m2ts_bw = bandwidth;
        }
        else if (service_type == SERVICE_TYPE_MP4) {
            nginx_stat->mp4_bw = bandwidth;
        }
        else if (service_type == SERVICE_TYPE_LIVE) {
            nginx_stat->live_bw = bandwidth;
        }
        else {
            log_error("bandwidth: unknown service type: %d.", service_type);
            return 0;
        }
    }
    else {
        log_error("unknown field[%d]: %s.", i, field);
        return 0;
    }

    return 1;
}

#ifdef _DEBUG
int main(void) 
{
    nginx_stat_t stat; 

    nginx_stat_get(&stat, "http://127.0.0.1/req-status", 2000);
    nginx_stat_dump(&stat);

    return 1;
}
#endif
