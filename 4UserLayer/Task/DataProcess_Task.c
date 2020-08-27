/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : DataProcess_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��5��15��
  ����޸�   :
  ��������   : ��ˢ��/QR/Զ���͹��������ݽ��д���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��5��15��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "DataProcess"
#include "elog.h"

#include "DataProcess_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "ini.h"
#include "malloc.h"
#include "tool.h"


static void test(void);

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *dataProcTaskName = "vDataProcTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;  
CMD_BUFF_STRU gCmd_buff = {0};


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
 
static void vTaskDataProcess(void *pvParameters);
//static READER_BUFF_STRU gtmpReaderMsg;



void CreateDataProcessTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskDataProcess,         
                (const char*    )dataProcTaskName,       
                (uint16_t       )DATAPROC_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DATAPROC_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDataProc);
}


static void vTaskDataProcess(void *pvParameters)
{    
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100); /* �������ȴ�ʱ��Ϊ100ms */ 
    int ret = 0;
    int len = 0;

    uint8_t openLeft[8] = { 0x02,0x00,0x07,0x01,0x06,0x4c,0x03,0x4D };    
    uint8_t openRight[8] = { 0x02,0x00,0x07,0x01,0x06,0x52,0x03,0x53 };
    uint8_t jsonbuff[256] = {0};

    
    CMD_BUFF_STRU *ptCmd = &gCmd_buff;
    READER_BUFF_STRU *ptMsg  = &gReaderRecvMsg;
    memset(&gReaderRecvMsg,0x00,sizeof(READER_BUFF_STRU));   
    memset(&gCmd_buff,0x00,sizeof(CMD_BUFF_STRU));       
    
    while (1)
    {
        xReturn = xQueueReceive( xCardIDQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE != xReturn)
        {  
            //û�н��յ�����
            //������
            continue;
        }

        //��Ϣ���ճɹ������ͽ��յ�����Ϣ                
        log_d("cardid %02x,%02x,%02x,%02x,devid = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID);

        ret = readHead(ptMsg->cardID, CARD_MODE);

        log_d("readHead = %d\r\n",ret);

        if(ret != NO_FIND_HEAD)
        {
            log_d("read card success\r\n");     

            ptCmd->cmd_len = 8;
            
            if(ptMsg->devID == 1)
            {
                memcpy(ptCmd->cmd,openLeft,ptCmd->cmd_len);                
            }
            else
            {
                memcpy(ptCmd->cmd,openRight,ptCmd->cmd_len); 
            } 

			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
			if(xQueueSend(xCmdQueue,             /* ��Ϣ���о�� */
						 (void *) &ptCmd,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						 (TickType_t)30) != pdPASS )
			{
                xQueueReset(xCmdQueue);
                DBG("send card2  queue is error!\r\n"); 
                //���Ϳ���ʧ�ܷ�������ʾ
                //�����Ƕ�����                
            }
            else
            {
                log_d("2 cardid %02x,%02x,%02x,%02x,devid = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID);
                //�������                
                packetCard(ptMsg->cardID, jsonbuff);

                //�������ݵ�MQTT������
                len = strlen((const char*)jsonbuff);

                len = mqttSendData(jsonbuff,len);
                
                log_d("send = %d\r\n",len);   

                
            }
        }
        else
        {
            log_d("read card error: not find card\r\n");
        }        
        
        
        /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4); 
        vTaskDelay(100); 

    }

}


