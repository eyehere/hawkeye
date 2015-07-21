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
#ifndef NGINX_STAT_H_
#define NGINX_STAT_H_ 
#include <easy/easy.h>
#include <curl/curl.h>

typedef struct _nginx_stat_t {
    unsigned long f4v_bw;      //F4V文件实时出口带宽
    unsigned long mp4_bw;      //MP4文件实时出口带宽
    unsigned long ts_bw;       //TS文件实时出口带宽
    unsigned long m2ts_bw;     //M2TS文件实时出口带宽
    unsigned long live_bw;     //LIVE实时出口带宽

    int f4v_conn;              //F4V并发连接数
    int mp4_conn;              //MP4并发连接数
    int ts_conn;               //TS并发连接数
    int m2ts_conn;             //M2TS并发连接数
    int live_conn;             //LIVE并发连接数
} nginx_stat_t;

/** 
 *        Name: nginx_stat_get
 * Description: 获取nginx状态信息
 *   Parameter: nginx_stat
 *              req_url
 *              timeout -> http访问超时，单位毫秒
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
int nginx_stat_get(nginx_stat_t *nginx_stat, char *req_url, int timeout);

/** 
 *        Name: nginx_stat_json
 * Description: 将nginx的状态信息转换成json
 *   Parameter: nginx_stat
 *              json_buffer
 *              size
 *      Return: 1 -> 成功
 *              o -> 失败
 */
int nginx_stat_json(nginx_stat_t *nginx_stat, char *json_buffer, int size);

/** 
 *        Name: nginx_stat_dump
 * Description: 输出nginx状态信息
 *   Parameter: 
 *             
 *      Return: 1 -> 获取成功
 *              o -> 获取失败
 */
void nginx_stat_dump(nginx_stat_t *nginx_stat);

#endif
