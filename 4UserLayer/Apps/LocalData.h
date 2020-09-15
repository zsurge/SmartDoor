/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : LocalData.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��3��21��
  ����޸�   :
  ��������   : LocalData.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��3��21��
    ��    ��   :  
    �޸�����   : �����ļ�

  FLASH����
  0-0x200000����������
  0x300000    ���bin�ļ�
  0x400000    ����Ѵ洢���ţ���ɾ�����ţ��Ѵ洢�û�ID����ɾ���û�ID��
  0x500000    ��ſ�������
  0x900000    ����û�����
  0x1300000   Ԥ��

FLASH����˼·��
����0.�ȶ�ȡ��ɾ������������Ϊ�㣬�����ɾ�����һ������ֵ��ֵ������������д���ţ�
      ��ɾ������ֵ�Լ�������ִ�в���2��
    1.��ɾ������Ϊ�㣬�Կ���Ϊ�������洢��FLASH�У���Ϊ������ÿ����һ�����ţ�����������
    2.���Ź�������Ϊ�̶����ݳ��ȣ��洢ʱ��д�������ݵ�ַ+ƫ����(����*�̶�����)
ɾ��1.���ҿ��ţ����У����¼�ÿ�����ֵ���洢����ɾ���ռ��ڣ�����ɾ������ֵ����,
      ���ޣ��򷵻�δ�ҵ���
�ģ�1.���ҿ��ţ�ȷ��λ�ã��޸���Ӧ��ֵ����д�뵽FLASH
�飺1.���ҿ��ţ�ȷ��λ�ã���������Ӧ��ֵ  

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 
#include "ini.h"


#define CARD_USER_LEN              (8)
#define FLOOR_ARRAY_LENGTH         (64) //ÿ����ͨ�û����64��Ȩ��
#define TIME_LENGTH                (10)
#define TIMESTAMP_LENGTH           (10)
#define RESERVE_LENGTH             (4) //Ԥ���ռ� Ϊ�˶��룬����һ�����������������ֽ���





////���ÿ�״̬Ϊ0��ɾ����
#define CARD_DEL                    0
#define CARD_VALID                  1
#define USER_DEL                    CARD_DEL
#define USER_VALID                  CARD_VALID
#define TABLE_HEAD                  0xAA




/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//extern volatile uint16_t gCurCardHeaderIndex;    //��������
//extern volatile uint16_t gCurUserHeaderIndex;    //�û�ID����
//extern volatile uint16_t gDelCardHeaderIndex;    //��ɾ����������
//extern volatile uint16_t gDelUserHeaderIndex;    //��ɾ���û�ID����
//extern volatile uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

typedef enum 
{
  ISFIND_NO = 0,
  ISFIND_YES 
}ISFIND_ENUM;





#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //����ͷ
    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t cardState;                              //��״̬ ��Ч/��ɾ��/������/��ʱ��    
    uint8_t platformType;                           //ƽ̨���ͣ�������ƽ̨����˼��ƽ̨��
    uint8_t cardNo[CARD_USER_LEN+1];                  //����
    uint8_t brushCardTime[TIME_LENGTH+1];               //ˢ��ʱ��
//    uint8_t startTime[TIME_LENGTH+1];                 //�˻���Чʱ��
//    uint8_t endTime[TIME_LENGTH+1];                   //�˻�����ʱ��    
//    uint8_t timeStamp[TIME_LENGTH+1];                 //��ά��ʱ���
    uint8_t reserve[RESERVE_LENGTH+1];                //Ԥ���ռ� 
    uint8_t crc;                                    //У��ֵ head~reseve
}USERDATA_STRU;
#pragma pack()

extern USERDATA_STRU gUserDataStru;




#if 0
uint8_t writeUserData(USERDATA_STRU *userData,uint8_t mode);

uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);

uint8_t modifyUserData(USERDATA_STRU *userData,uint8_t mode);

uint8_t writeZeaoHead (uint8_t multiple,uint16_t remainder,HEADINFO_STRU *card);

#endif





//add 2020.07.14
//��ȡ/���ҿ���
int readHead(uint8_t *headBuff,uint8_t mode); 

//�Ի������Ŀ��Ž�������
void sortHead(HEADINFO_STRU *head,int length);

//��ӿ���
uint8_t addHead(uint8_t *head,uint8_t mode);

//ɾ������
int delHead(uint8_t *headBuff,uint8_t mode);

//���ɾ�����Ŷ�Ӧ������λ��(�뷨�����⣬�ݲ�ʵ��)
uint8_t addDelHead(int index);

//uint8_t addHeadID(uint8_t *head,uint8_t mode);



#endif /* __LOCALDATA_H__ */
