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
#ifndef REPORT_H_
#define REPORT_H_
#include <easy/easy.h>
#include "hawkeye_config.h"
#include "hawkeye.h"

/** 
 *        Name: report_do
 * Description: 汇报状态信息
 *   Parameter: hawkeye
 *              timeout -> 汇报超时时间, 单位毫秒
 *             
 *      Return: 1 -> 汇报成功
 *              0 -> 汇报失败
 */
int report_do(hawkeye_t *hawkeye, int timeout);

#endif
