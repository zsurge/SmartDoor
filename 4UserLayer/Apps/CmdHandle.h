/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ����ͨѶЭ�����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __CMDHANDLE_H
#define __CMDHANDLE_H

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "pub_options.h"
#include "errorcode.h"




/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MQTT_MAX_LEN 1024*2
#define MQTT_TEMP_LEN 300


#define AUTH_MODE_RELIEVECONTROL        1
//IC��
#define AUTH_MODE_CARD      2
//Զ��
#define AUTH_MODE_REMOTE    3
//���
#define AUTH_MODE_UNBIND    4
//��
#define AUTH_MODE_BIND      5
//��ά��
#define AUTH_MODE_QR        7

#define QUEUE_BUF_LEN   512


#define CARD_ID_LEN     4



#pragma pack(1)
typedef struct
{
    uint8_t cardID[CARD_ID_LEN];           //��Ҫ���͸�������������
                                           //���һ���ֽ� =0 DISABLE ��ֹ����; = 1 ENABLE ������
    uint8_t devID;                          // = 1 1�Ŷ�������=2 2�Ŷ�����
}READER_BUFF_STRU;
#pragma pack()




extern READER_BUFF_STRU gReaderMsg;


extern int gConnectStatus;
extern int gMySock;
extern uint8_t gUpdateDevSn; 
extern uint32_t gCurTick;


//������Ϣ��������
int mqttSendData(uint8_t *payload_out,uint16_t payload_out_len); 

SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf );

void Proscess(void* data);

#endif

