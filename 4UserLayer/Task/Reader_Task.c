/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Reader_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��27��
  ����޸�   :
  ��������   : ����ά������������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��27��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
 
#define LOG_TAG    "reader"
#include "elog.h"

#include "Reader_Task.h"
#include "CmdHandle.h"
#include "bsp_dipSwitch.h"
#include "tool.h"
#include "LocalData.h"





/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define READER_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

typedef union
{
	uint32_t id;        //����
	uint8_t sn[4];    //���Ű��ַ�
}CARD_TYPE;


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *ReaderTaskName = "vReaderTask";  

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskReader = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskReader(void *pvParameters);

void CreateReaderTask(void)
{
    //��androidͨѶ�������ݽ���
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
        
    	/* �����¼���־����ʾ������������ */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);       
        
        vTaskDelay(100);        
    }

}   



