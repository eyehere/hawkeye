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
#ifndef GLANCE_CONFIG_H_
#define GLANCE_CONFIG_H_

#define LOG_DST_CONSOLE 0
#define LOG_DST_FILE    1
#define LOG_DST_SYSLOG  2

#define MAX_PROCESS_COUNT 32

typedef struct _hawkeye_config_t {
    int    daemon;                               //是否以daemon进程方式运行
    int    log_level;                            //日志级别
    int    log_dst;                              //日志输出方式，支持控制台，文件，syslog三种方式
    char  *log_file;                             //日志文件
    int    hawkeye_period;                        //采集数据的周期
    int    http_timeout;                         //HTTP超时时间
    char  *nginx_status_req_url;                 //nginx状态请求URL
    char  *status_report_url;                    //CDN状态汇报URL
    char  *monitor_processes[MAX_PROCESS_COUNT]; //监控进程组
    int   process_count;
    char  *proxy_server;                         //代理服务器IP
    int   proxy_port;                            //代理服务器端口
    char  *proxy_user;                           //代理服务器账号
    char  *proxy_password;                       //代理服务器密码

    int    bw_out_ceiling;                       //网卡出口带宽上限
    double nic_bw_usage_ceiling;                 //网卡带宽利用率上限
} hawkeye_config_t;

/** 
 *        Name: hawkeye_config_load
 * Description: load hawkeye config
 *   Parameter: path -> hawkeye config file path
 *      Return: the instance of hawkeye config 
 *              load failedly, return NULL 
 */
hawkeye_config_t *hawkeye_config_load(char *path);

/** 
 *        Name: hawkeye_config_free
 * Description: release hawkeye config 
 *   Parameter: config -> the instance of hawkeye config
 *      Return:  
 *               
 */
void hawkeye_config_free(hawkeye_config_t *config);

/** 
 *        Name: hawkeye_config_dump
 * Description: dump hawkeye config information
 *   Parameter: config -> the instance of hawkeye config
 *      Return:  
 *               
 */
void hawkeye_config_dump(hawkeye_config_t *config);

#endif
