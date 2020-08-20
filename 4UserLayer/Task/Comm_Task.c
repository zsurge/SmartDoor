/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Comm_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : 跟电梯通讯的任务处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "malloc.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);



void CreateCommTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskComm,         
                (const char*    )CommTaskName,       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);
}


static void vTaskComm(void *pvParameters)
{ 
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100); /* 设置最大等待时间为100ms */ 
    
    CMD_BUFF_STRU *ptCmd = &gCmd_buff;
    
    memset(&gCmd_buff,0x00,sizeof(CMD_BUFF_STRU));   
    

    while(1)
    { 

        xReturn = xQueueReceive( xCmdQueue,    /* 消息队列的句柄 */
                                 (void *)&ptCmd,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE == xReturn)
        {  
            //发送指令
            log_d("ptCmd = %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n",
            ptCmd->cmd[0],ptCmd->cmd[1],ptCmd->cmd[2],ptCmd->cmd[3],
            ptCmd->cmd[4],ptCmd->cmd[5],ptCmd->cmd[6],ptCmd->cmd[7]);
            
        }
        else
        {
            //发送查询
//            log_d("send query cmd\r\n");
        }

        
         vTaskDelay(100); 
    }  
}


