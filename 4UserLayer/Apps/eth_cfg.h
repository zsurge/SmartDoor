/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : eth_cfg.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2020年1月6日
  最近修改   :
  功能描述   : 网络相关参数
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月6日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __ETH_CFG__H
#define __ETH_CFG__H


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "tool.h"
#include "easyflash.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//#define   HOST_NAME       "192.168.110.79"     //服务器IP地址 测试专用 
//#define   HOST_NAME       "192.168.110.109"     //服务器IP地址 线下 
//#define   HOST_NAME         "mqtt.bsgoal.net.cn"
//#define   HOST_NAME         "120.25.169.59"
//#define HOST_NAME "120.78.247.221"
#define HOST_NAME "mqtt.bsgoal.net.cn"
//#ifdef LWIP_DNS
//#define HOST_NAME "mqtt.bsgoal.net.cn"
//#else
//#define HOST_NAME "120.78.247.221"
//#endif


#define   HOST_PORT     1883    //由于是TCP连接，端口必须是1883

#define DEVICE_PUBLISH		"/smartCloud/server/msg/device"	
#define DEVICE_SUBSCRIBE	"/smartCloud/terminal/msg/"  

#define DEV_FACTORY_PUBLISH		"/smartCloud/production/msg/device"	
#define DEV_FACTORY_SUBSCRIBE	"/smartCloud/production/msg/"    


extern int gConnectStatus;



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/



#endif


