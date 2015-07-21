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
#ifndef PROCESS_STAT_H_
#define PROCESS_STAT_H_
#include <easy/easy.h>

#ifndef MAX_PROCESS_COUNT
#define MAX_PROCESS_COUNT 32
#endif

#define MAX_PID_SIZE            12
#define MAX_PROCESS_GROUP_COUNT 8
#define MAX_PATH_SIZE           2048
#define MAX_PROCESS_NAME_SIZE   32

typedef struct _single_process_stat_t {
    char pid[MAX_PID_SIZE];
    char state;
    unsigned long long virt; //虚拟内存的大小，单位KB
    unsigned long long res;  //常驻内存的大小，单位KB
} single_process_stat_t;

typedef struct _process_group_stat_t {
    char name[MAX_PROCESS_NAME_SIZE];                     //进程名称
    single_process_stat_t processes[MAX_PROCESS_COUNT];
    int count;
} process_group_stat_t;

typedef struct _processes_stat_t {
    process_group_stat_t process_groups[MAX_PROCESS_GROUP_COUNT]; 
    int process_group_count;
    int total_count;
} processes_stat_t;

/** 
 *        Name: processes_stat_get
 * Description: 获取进程状态信息
 *   Parameter: 
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int processes_stat_get(processes_stat_t *processes_stat, char **processes, int count);

/** 
 *        Name: processes_stat_json
 * Description: 将进程状态信息转换成json
 *   Parameter: 
 *             
 *      Return: 1 -> 成功
 *              o -> 失败
 */
int processes_stat_json(processes_stat_t *processes_stat, char *json_buffer, int size);

#endif
