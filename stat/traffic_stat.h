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
#ifndef TRAFFIC_STAT_H_
#define TRAFFIC_STAT_H_
#include <easy/easy.h>

#define MAX_NIC_NAME_SIZE 32
#define MAX_NIC_COUNT     32
#define PROC_STAT_TRAFFIC "/proc/net/dev"

typedef struct _nic_bw_t {
    unsigned long long bw_in;
    unsigned long long bw_out;
} nic_bw_t;

typedef struct _traffic_bw_t {
    nic_bw_t bws[MAX_NIC_COUNT];
    int count;

    unsigned long long bw_in;
    unsigned long long bw_out;
} traffic_bw_t;

typedef struct _nic_stat_t {
    char     name[MAX_NIC_NAME_SIZE];
    unsigned long long byte_in;
    unsigned long long byte_out;
    unsigned long long pkt_in;
    unsigned long long pkt_out;
    struct timeval t;
} nic_stat_t;

/*
 * Structure for traffic infomation.
 */
typedef struct _traffic_stat_t {
    nic_stat_t nics[MAX_NIC_COUNT];
    int count;

    unsigned long long byte_in;
    unsigned long long byte_out;
} traffic_stat_t;

/** 
 *        Name: traffic_stat_get 
 * Description: 获取traffic状态信息
 *   Parameter: stat -> 输出参数，状态实例
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int traffic_stat_get(traffic_stat_t *stat);

/** 
 *        Name: traffic_stat_json
 * Description: 将带宽状态转换成json 
 *   Parameter: stat -> 输出参数，状态实例
 *             
 *      Return: 1 -> 成功
 *              o -> 失败
 */
int traffic_stat_json(traffic_stat_t *stat1, traffic_stat_t *stat2, char *json_buffer, int size);

#endif
