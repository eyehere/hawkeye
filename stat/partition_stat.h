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
#ifndef PARTITION_STAT_H_
#define PARTITION_STAT_H_
#include <easy/easy.h>
#include <mntent.h>
#include <sys/vfs.h>

#define MTAB_FILE "/etc/mtab"

#define MAX_FS_NAME_SIZE  256
#define MAX_MNT_DIR_SIZE  256

#define MAX_PARTITIOn_COUNT 100

#define PARTITION_STATUS_ERROR 0
#define PARTITION_STATUS_OK    1

typedef struct _partition_t {
    long    type;     /*  type of filesystem (see below) */
    long    bsize;    /*  optimal transfer block size */
    long    blocks;   /*  total data blocks in file system */
    long    bfree;    /*  free blocks in fs */
    long    bavail;   /*  free blocks avail to non-superuser */
    char fs_name[MAX_FS_NAME_SIZE];
    char mnt_dir[MAX_MNT_DIR_SIZE];

    long long nonroot_total_size; /* 非root用户可使用的总大小 */ 
    long long total_size; /* 总大小，以M为单位 */
    long long used_size;  /* 已用空间，以M为单位 */
    long long avail_size; /* 空闲空间，以M为单位 */
    float     usage;      /* 使用率 */

    int status;
} partition_t;

typedef struct _partition_stat_t {
    partition_t partitions[MAX_PARTITIOn_COUNT]; 
    int         count;

    long long  nonroot_total_size;
    long long  total_size;
    long long  used_size;
    long long  avail_size;
    float usage;
} partition_stat_t;

/** 
 *        Name: partition_stat_get
 * Description: 获取分区表信息
 *   Parameter: stat
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int partition_stat_get(partition_stat_t *stat);

/** 
 *        Name: partition_stat_json
 * Description: 将分区信息转换成json
 *   Parameter: stat
 *             
 *      Return: 1 -> 成功
 *              o -> 失败
 */
int partition_stat_json(partition_stat_t *stat, char *json_buffer, int size);

#endif
