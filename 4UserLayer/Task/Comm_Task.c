/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Comm_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��28��
  ����޸�   :
  ��������   : ������ͨѶ���������ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
//#include "bsp_usart6.h"
#include "malloc.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37
#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55



#define STEP1   0
#define STEP2   10
#define STEP3   20
#define STEP4   30

typedef struct FROMHOST
{
    uint8_t rxStatus;                   //����״̬
    uint8_t rxCRC;                      //У��ֵ
    uint8_t rxBuff[16];                 //�����ֽ���
    uint16_t rxCnt;                     //�����ֽ���    
}FROMHOST_STRU;

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)
//static uint32_t totalCnt = 0;
//static uint32_t validCnt = 0;
//static uint32_t sendCnt = 0;

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);
//static uint8_t deal_Serial_Parse(void);

//static FROMHOST_STRU rxFromHost;


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
    while(1)
    { 
        vTaskDelay(300);
    }  
}




//static uint8_t deal_Serial_Parse(void)
//{
//    uint8_t ch = 0;
//    
//    while(RS485_Recv(COM6,&ch,1))
//    {
//       switch (rxFromHost.rxStatus)
//        {                
//            case STEP1:
//                if(0x5A == ch) /*���հ�ͷ*/
//                {
//                    rxFromHost.rxBuff[0] = ch;
//                    rxFromHost.rxCRC = ch;
//                    rxFromHost.rxCnt = 1;
//                    rxFromHost.rxStatus = STEP2;
//                }

//                break;
//           case STEP2:
//                if(0x01 == ch) //�ж��ڶ����ֽ��Ƿ�����Ҫ���ֽڣ�����������ʱ���ȡ���뿪�ص�ֵ
//                {
//                    rxFromHost.rxBuff[1] = ch;
//                    rxFromHost.rxCRC ^= ch;
//                    rxFromHost.rxCnt = 2;
//                    rxFromHost.rxStatus = STEP3;                
//                }
//                else
//                {                
//                   memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));                   
//                }
//                break;           
//            default:      /* �����������ݰ� */
//            
//                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
//                rxFromHost.rxCRC ^= ch;
//                
//                if(rxFromHost.rxCnt >= 5)
//                {
//                
//                    if(rxFromHost.rxCRC == 0)
//                    { 
//                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
//                        return FINISHED;                         
//                    }  
//                    memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
//                } 
//             
//                break;
//         }
//         
//    } 
//    return UNFINISHED;
//}


