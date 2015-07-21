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
#include "traffic_stat.h"

static int traffic_bw_calculate(traffic_stat_t *pre_stat, traffic_stat_t *cur_stat, traffic_bw_t *bws);
static int nic_get_ip(char *name, char *ip, int ip_size);

/*
 * collect traffic infomation
 */
int traffic_stat_get(traffic_stat_t *stat)
{
	FILE *fp;
	char line[4096] = {0};
	char *p = NULL;

    bzero(stat, sizeof(traffic_stat_t));

	if ((fp = fopen(PROC_STAT_TRAFFIC, "r")) == NULL) {
        log_error("open %s failed, error: .", PROC_STAT_TRAFFIC, strerror(errno));
		return 0;
	}

    while (fgets(line, sizeof(line) - 1, fp) != NULL) {
        char *str = line; 

        while (*str == ' ' || *str == '\t') {
            str++; 
        }

        if (*str == 0) {
            continue;
        }

        if (strstr(str, "eth")) {
            nic_stat_t *nic_stat = &stat->nics[stat->count];

            bzero(nic_stat, sizeof(nic_stat_t));
            p = strchr(str, ':');
            *p = 0;
            strncpy(nic_stat->name, str, MAX_NIC_NAME_SIZE);
             
            sscanf(p + 1, "%llu %llu %*u %*u %*u %*u %*u %*u " 
                          "%llu %llu %*u %*u %*u %*u %*u %*u",
                    &nic_stat->byte_in,
                    &nic_stat->pkt_in,
                    &nic_stat->byte_out,
                    &nic_stat->pkt_out);
            if (nic_stat->byte_in == 0 && nic_stat->byte_out == 0) {
                continue;
            }

            stat->byte_in  += nic_stat->byte_in;
            stat->byte_out += nic_stat->byte_out;

            gettimeofday(&nic_stat->t, NULL);
            stat->count++;
        }

        bzero(line, sizeof(line));
    }
	
	fclose(fp);
    return 1;
}

static int traffic_bw_calculate(traffic_stat_t *pre_stat, traffic_stat_t *cur_stat, traffic_bw_t *bws)
{
    int i = 0;
    
    bzero(bws, sizeof(traffic_bw_t));
    for (i = 0; i < pre_stat->count; i++) {
        nic_stat_t *ns1, *ns2; 
        long long bw_delta   = 0;
        long long time_delta = 0;

        ns1 = &pre_stat->nics[i];
        ns2 = &cur_stat->nics[i];

        time_delta = (ns2->t.tv_sec  - ns1->t.tv_sec) * 1000 +
                     (ns2->t.tv_usec - ns1->t.tv_usec)/1000;

        bw_delta = ns2->byte_in - ns1->byte_in;
        if (bw_delta < 0) {
            log_error("byte_in rotate, ns2->byte_in: %llu, ns1->byte_in: %llu.", ns2->byte_in, ns1->byte_in);
            bw_delta = bw_delta + 0xffffffffffffffff;
        }
        bws->bws[i].bw_in = (bw_delta * 8) * 1000/time_delta;

        //加一些异常处理的日志
        if (bws->bws[i].bw_in/(1024 * 1024 * 8) > 150) {
            log_error("byte_in rotate, ns2->byte_in: %llu, ns1->byte_in: %llu.", ns2->byte_in, ns1->byte_in);
        }

        bw_delta = ns2->byte_out - ns1->byte_out;
        if (bw_delta < 0) {
            log_error("byte_out rotate, ns2->byte_out: %llu, ns1->byte_out: %llu.", ns2->byte_out, ns1->byte_out);
            bw_delta = bw_delta + 0xffffffffffffffff;
        }
        bws->bws[i].bw_out = (bw_delta * 8) * 1000/time_delta;

        //加一些异常处理的日志
        if (bws->bws[i].bw_out/(1024 * 1024 * 8) > 150) {
            log_error("byte_out rotate, ns2->byte_out: %llu, ns1->byte_out: %llu.", ns2->byte_out, ns1->byte_out);
        }

        log_debug("traffic stat: %s %llu %llu", ns1->name, 
            bws->bws[i].bw_in/(1024 * 1024 * 8), bws->bws[i].bw_out/(1024 * 1024 * 8));
        
        bws->bw_in  += bws->bws[i].bw_in;
        bws->bw_out += bws->bws[i].bw_out;

        bws->count++;
    }

    return 1;
}

int traffic_stat_json(traffic_stat_t *stat1, traffic_stat_t *stat2, char *json_buffer, int size)
{
    int i      = 0;
    int offset = 0;
    int else_size = size;
    traffic_bw_t bw;
    char tmp_buffer[1024];
    char *fmt_str = NULL;
    int len = 0;

    bzero(json_buffer, size);
    traffic_bw_calculate(stat1, stat2, &bw);

    for (i = 0; i < bw.count; i++) {
        char ip[32] = {0};

        bzero(tmp_buffer, sizeof(tmp_buffer));

        if (i == 0) {
            fmt_str = "traffic={\"%d\":{\"name\":\"%s\",\"ip\":\"%s\",\"bw_in\":%llu,\"bw_out\":%llu}"; 
        }
        else {
            fmt_str = ",\"%d\":{\"name\":\"%s\",\"ip\":\"%s\",\"bw_in\":%llu,\"bw_out\":%llu}"; 
        }

        nic_get_ip(stat1->nics[i].name, ip, sizeof(ip));

        len = snprintf(tmp_buffer, sizeof(tmp_buffer) - 1, fmt_str, 
            i + 1,
            stat1->nics[i].name, 
            ip,
            bw.bws[i].bw_in,
            bw.bws[i].bw_out);

        if (len >= else_size) {
            log_error("traffic_stat_json: json_buffer is too small.");
            return 0;
        }

        memcpy(json_buffer + offset, tmp_buffer, len);
        offset    += len;
        else_size -= len;
    }

    bzero(tmp_buffer, sizeof(tmp_buffer));
    fmt_str = ",\"bw_in\":%llu,\"bw_out\":%llu}";
    len = snprintf(tmp_buffer, sizeof(tmp_buffer) - 1, fmt_str, bw.bw_in, bw.bw_out);
    if (len >= else_size) {
        log_error("traffic_stat_json: json_buffer is too small.");
        return 0;
    }
    memcpy(json_buffer + offset, tmp_buffer, len);
    else_size -= len;
    offset    += len;

    return 1;
}

static int nic_get_ip(char *name, char *ip, int ip_size)
{
    int fd = -1; 
    struct ifreq ifr;
    char *str;

    bzero(&ifr, sizeof(struct ifreq));
    bzero(ip, ip_size);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        log_error("nic_get_addr: create socket failed.");
        return 0;
    }

    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name , name);

    ioctl(fd, SIOCGIFADDR, &ifr);
    str = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    strncpy(ip, str, ip_size - 1);   
    
    close(fd);

    return 1;
}

#ifdef _DEBUG
int main(void)
{
    traffic_stat_t stat1, stat2;
    traffic_bw_t traffic_bw;
    int i = 0;

    traffic_stat_get(&stat1); 
    sleep(1);
    traffic_stat_get(&stat2); 

    traffic_bw_calculate(&stat1, &stat2, &traffic_bw);

    for (i = 0; i < traffic_bw.count; i++) {
        printf("%s: bw_in[%lluM], bw_out[%lluM]\n", 
                stat1.nics[i].name,
                traffic_bw.bws[i].bw_in/(8 * 1024 * 1024),
                traffic_bw.bws[i].bw_out/(8 * 1024 * 1024));
    }
    
    return 1; 
}
#endif
