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
#ifndef IO_STAT_H_
#define IO_STAT_H_
#include <easy/easy.h>

#define MAX_DEV_NAME_SIZE  128
#define MAX_MOUNT_DIR_SIZE 256
#define MAX_IO_COUNT       100

#define DISK_STAT_FILE "/proc/diskstats"

typedef struct _io_t {
    char               dev_name[MAX_DEV_NAME_SIZE];
    unsigned int       major;       /* Device major number */
    unsigned int       minor;       /* Device minor number */
    unsigned long long rd_ios;      /* Read I/O operations */
    unsigned long long rd_merges;   /* Reads merged */
    unsigned long long rd_sectors;  /* Sectors read */
    unsigned long long rd_ticks;    /* Time in queue + service for read */
    unsigned long long wr_ios;      /* Write I/O operations */
    unsigned long long wr_merges;   /* Writes merged */
    unsigned long long wr_sectors;  /* Sectors written */
    unsigned long long wr_ticks;    /* Time in queue + service for write */
    unsigned long long ticks;       /* Time of requests in queue */
    unsigned long long aveq;        /* Average queue length */ 
    struct timeval     t;
} io_t;

typedef struct _io_usage_t {
    char  dev_name[MAX_IO_COUNT][MAX_DEV_NAME_SIZE];
    float usage[MAX_IO_COUNT];
    int   count;
} io_usage_t;

typedef struct _io_stat_t {
    io_t ios[MAX_IO_COUNT]; 
    int count; 
} io_stat_t;

/** 
 *        Name: io_stat_get
 * Description: 得到IO状态信息
 *   Parameter: stat
 *
 *      Return: 1 -> 获取IO状态成功。
 *              0 -> 获取IO状态失败。
 */
int io_stat_get(io_stat_t *io_stat);

/** 
 *        Name: io_stat_json
 * Description: 将IO利用率转换成json字符串
 *   Parameter: stat1
 *              stat2
 *              json_buffer -> 输出结果
 *              size -> json_buffer的大小
 *
 *      Return: 1 -> 成功
 *              0 -> 失败。
 */
int io_stat_json(io_stat_t *stat1, io_stat_t *stat2, char *json_buffer, int size);

/** 
 *        Name: io_stat_usage_dump
 * Description: 输出IO状态信息 
 *   Parameter: usage
 *
 *      Return: 出口带宽
 */
void io_stat_usage_dump(io_usage_t *usage);

#endif

