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
#ifndef CPU_STAT_H_
#define CPU_STAT_H_
#include <easy/easy.h>

#define PROC_CPU_STAT "/proc/stat"

/*
 * Structure for CPU infomation.
 */
typedef struct _cpu_stat_t {
        unsigned long long user;
        unsigned long long nice;
        unsigned long long sys;
        unsigned long long idle;
        unsigned long long iowait;
        unsigned long long steal;
        unsigned long long hardirq;
        unsigned long long softirq;
        unsigned long long guest;

        unsigned long long total;
        unsigned long long busy;

        unsigned int       cpu_num;
} cpu_stat_t;

/** 
 *        Name: cpu_stat_get
 * Description: 获取CPU状态信息
 *   Parameter: 
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int cpu_stat_get(cpu_stat_t *cpu_stat);

/** 
 *        Name: cpu_stat_json
 * Description: 将cpu的状态转换成json字符串 
 *   Parameter: 
 *             
 *      Return: 1 -> 成功
 *              0 -> 失败 
 */
int cpu_stat_json(cpu_stat_t *stat1, cpu_stat_t *stat2, char *json_buffer, int size);

/** 
 *        Name: cpu_stat_dump
 * Description: 输出cpu状态信息
 *   Parameter: 
 *             
 *      Return:
 *              
 */
void cpu_stat_dump(cpu_stat_t *stat);

#endif
