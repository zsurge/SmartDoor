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
#include "bsp_Wiegand.h"






/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define READER_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

#define READER1         1           
#define READER2         2


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
static void reverseArray(uint8_t *array);

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

void reverseArray(uint8_t *array) 
{
    int i,temp;
    int size = sizeof(array)/sizeof(array[0]);

    
    for(i=0; i<size/2; i++)
    {
        temp = array[i];
        array[i] = array[size-i-1];
        array[size-i-1] = temp;
    }   

}



static void vTaskReader(void *pvParameters)
{ 
    CARD_TYPE cardDev1,cardDev2;    
    READER_BUFF_STRU *ptReaderBuf = &gReaderMsg;     

    while(1)
    {        
        /* ���� */
        ptReaderBuf->devID = 0; 
        memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
    
        cardDev1.id = bsp_WeiGenScanf(0);
        cardDev2.id = bsp_WeiGenScanf(1);

        if(cardDev1.id != 0)
        {
            reverseArray(cardDev1.sn);
            
            ptReaderBuf->devID = READER1; 
            memcpy(ptReaderBuf->cardID,cardDev1.sn,sizeof(cardDev1.sn));   


			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
			if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
						 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						 (TickType_t)10) != pdPASS )
			{
                xQueueReset(xCardIDQueue);
                DBG("send card1  queue is error!\r\n"); 
                //���Ϳ���ʧ�ܷ�������ʾ
                //�����Ƕ�����                
            }            

        }
        
        /* ���� */
        ptReaderBuf->devID = 0; 
        memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  

        if(cardDev2.id != 0)
        {
            reverseArray(cardDev2.sn);
            
            ptReaderBuf->devID = READER2; 
            memcpy(ptReaderBuf->cardID,cardDev2.sn,sizeof(cardDev2.sn));   

			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
			if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
						 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						 (TickType_t)10) != pdPASS )
			{
                xQueueReset(xCardIDQueue);
                DBG("send card2  queue is error!\r\n"); 
                //���Ϳ���ʧ�ܷ�������ʾ
                //�����Ƕ�����                
            } 

        }        

        

//        if(cardDev1.id != 0 || cardDev2.id != 0)
//        {
//            log_d("cardDev1 = %x,cardDev2=%x\r\n",cardDev1.id,cardDev2.id);
//            
//            log_d("cardDev1 %02x,%02x,%02x,%02x\r\n",cardDev1.sn[0],cardDev1.sn[1],cardDev1.sn[2],cardDev1.sn[3]);
//            log_d("cardDev2 %02x,%02x,%02x,%02x\r\n",cardDev2.sn[0],cardDev2.sn[1],cardDev2.sn[2],cardDev2.sn[3]);

//            reverseArray(cardDev1.sn);
//            
//            log_d("reverseArray %02x,%02x,%02x,%02x\r\n",cardDev1.sn[0],cardDev1.sn[1],cardDev1.sn[2],cardDev1.sn[3]);

//            ret = readHead(cardDev1.sn, CARD_MODE);

//            log_d("readHead = %d\r\n",ret);

//            if(ret != NO_FIND_HEAD)
//            {
//                log_d("read card success\r\n");
//            }
//            else
//            {
//                log_d("read card error: not find card\r\n");
//            }
//        }        
        
    	/* �����¼���־����ʾ������������ */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);       
        
        vTaskDelay(100);        
    }

}   



