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
#ifndef GLANCE_H_
#define GLANCE_H_
#include "stat/cpu_stat.h"
#include "stat/load_stat.h"
#include "stat/memory_stat.h"
#include "stat/nginx_stat.h"
#include "stat/traffic_stat.h"
#include "stat/io_stat.h"
#include "stat/partition_stat.h"
#include "stat/processes_stat.h"

typedef struct _hawkeye_t {
    char sn[128];
    char cpu_stat_json[1024];      
    char io_stat_json[4096];
    char load_stat_json[256];
    char memory_stat_json[1024];
    char nginx_stat_json[1024];
    char partition_stat_json[4096];
    char traffic_stat_json[2048];
    char processes_stat_json[4096];
} hawkeye_t;

/** 
 *        Name: hawkeye_create
 * Description: 创建hawkeye。
 *   Parameter: 
 *             
 *      Return: 返回hawkeye
 */
 hawkeye_t *hawkeye_create();

/** 
 *        Name: hawkeye_do
 * Description: 获取服务器的状态。
 *   Parameter: hawkeye -> 输出参数，服务器状态。
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int hawkeye_do(hawkeye_t *hawkeye);

/** 
 *        Name: hawkeye_destroy
 * Description: 释放服务器的状态。
 *   Parameter: hawkeye -> hawkeye实例
 *             
 *      Return: 
 */
void hawkeye_destroy(hawkeye_t *hawkeye);

#endif
