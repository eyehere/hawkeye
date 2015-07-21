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
#ifndef MEM_STAT_H_
#define MEM_STAT_H_
#include <easy/easy.h>

#define PROC_MEMORY_STAT "/proc/meminfo"

/* Structure for memory and swap space utilization statistics */
typedef struct _memory_stat_t {
        unsigned long total;       //total amount of memory
        unsigned long free;        //free memory
        unsigned long buffered;    //buffered memory
        unsigned long cached;      //cached memory
        unsigned long active;      //Active memory
        unsigned long inactive;    //Inactive memory
        unsigned long slab;        //Slab memory
        unsigned long free_swap;   //free swap memory
        unsigned long total_swap;  //total amount of swap memory
        unsigned long swap_cached; //cached swap
        unsigned long commited;    //commited memory
} memory_stat_t;

/** 
 *        Name: memory_stat_get 
 * Description: 获取内存状态信息
 *   Parameter: memory_stat
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int memory_stat_get(memory_stat_t *memory_stat);

/** 
 *        Name: memory_stat_json
 * Description: 将内存状态信息转换成json
 *   Parameter: memory_stat
 *             
 *      Return: 1 -> 成功
 *              o -> 失败
 */
int memory_stat_json(memory_stat_t *memory_stat, char *json_buffer, int size);

/** 
 *        Name: memory_stat_dump
 * Description: 输出内存状态信息
 *   Parameter: memory_stat
 *   
 *      Return: 
 */
void memory_stat_dump(memory_stat_t *memory_stat);

#endif
