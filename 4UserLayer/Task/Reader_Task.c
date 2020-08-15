/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Reader_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 处理维根读卡器任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
 
#define LOG_TAG    "reader"
#include "elog.h"

#include "Reader_Task.h"
#include "CmdHandle.h"
#include "bsp_dipSwitch.h"
#include "tool.h"
#include "LocalData.h"





/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define READER_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

typedef union
{
	uint32_t id;        //卡号
	uint8_t sn[4];    //卡号按字符
}CARD_TYPE;


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *ReaderTaskName = "vReaderTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskReader = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskReader(void *pvParameters);

void CreateReaderTask(void)
{
    //跟android通讯串口数据解析
    xTaskCreate((TaskFunction_t )vTaskReader,     
                (const char*    )ReaderTaskName,   
                (uint16_t       )READER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )READER_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskReader);
}



static void vTaskReader(void *pvParameters)
{ 
    CARD_TYPE card1,card2;
    HEADINFO_STRU head;
    int ret = 0;

    while(1)
    {        
        card1.id = bsp_WeiGenScanf(0);
        card2.id = bsp_WeiGenScanf(1);

        if(card1.id != 0 || card2.id != 0)
        {
            log_d("card1 = %x,card2=%x\r\n",card1.id,card2.id);
            
            log_d("card1 %02x,%02x,%02x,%02x\r\n",card1.sn[3],card1.sn[2],card1.sn[1],card1.sn[0]);
            log_d("card2 %02x,%02x,%02x,%02x\r\n",card2.sn[3],card2.sn[2],card2.sn[1],card2.sn[0]);

            head.headData.id = card1.id;

            if(readHead(&head, CARD_MODE) != NO_FIND_HEAD)
            {
                log_d("read card success\r\n");
            }
            else
            {
                log_d("read card error: not find card\r\n");
            }
        }
        
    	/* 发送事件标志，表示任务正常运行 */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);       
        
        vTaskDelay(100);        
    }

}   



