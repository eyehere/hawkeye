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
#ifndef LOAD_STAT_H_
#define LOAD_STAT_H_
#include <easy/easy.h>

#define PROC_LOAD_STAT "/proc/loadavg"

/* Structure for queue and load statistics */
typedef struct _load_stat_t {
	float  avg_1;
	float  avg_5;
	float  avg_15;
	unsigned int  nr_threads;
	unsigned long nr_running;
} load_stat_t;

/** 
 *        Name: load_stat_get
 * Description: 获取load状态信息
 *   Parameter: 
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int load_stat_get(load_stat_t *load_stat);

/** 
 *        Name: load_stat_json
 * Description: 将load状态转换成json
 *   Parameter: 
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int load_stat_json(load_stat_t *load_stat, char *json_buffer, int size, int cpu_num);

/** 
 *        Name: load_stat_dump
 * Description: 输出load状态信息
 *   Parameter: 
 *             
 *      Return: 
 *              
 */
void load_stat_dump(load_stat_t *load_stat);

#endif
