/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : FloorDataProc.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 电梯控制器的指令处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "FloorData"
#include "elog.h"
#include "FloorDataProc.h"




/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define AUTO_REG            1
#define MANUAL_REG          2

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void calcFloor(uint8_t layer,uint8_t regMode,uint8_t *src,uint8_t *outFloor);
static SYSERRORCODE_E authReader(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData);




void packetDefaultSendBuf(uint8_t *buf)
{
    uint8_t sendBuf[64] = {0};

    sendBuf[0] = CMD_STX;
    sendBuf[1] = 0x01;//bsp_dipswitch_read();
    sendBuf[2] = 0x01;//bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
}


void packetSendBuf(READER_BUFF_STRU *pQueue)
{
    uint8_t jsonBuf[512] = {0};
    uint8_t sendBuf[64] = {0};
    uint16_t len = 0;
    uint16_t ret = 0;
    int tagFloor = 0;
    USERDATA_STRU *localUserData = &gUserDataStru;
    memset(localUserData,0x00,sizeof(USERDATA_STRU));
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);
    log_d("card or QR data = %s\r\n",pQueue->data);

    switch(pQueue->authMode)
    {
        case AUTH_MODE_CARD:
        case AUTH_MODE_QR:
            log_d("card or QR auth,pQueue->authMode = %d\r\n",pQueue->authMode);
            ret = authReader(pQueue,localUserData);  
            
            if(ret != NO_ERR)
            {
                log_d("reject access\r\n");
                return ;  //无权限
            }
            
            //2.发给服务器
            packetPayload(localUserData,jsonBuf); 

            len = strlen((const char*)jsonBuf);

            len = mqttSendData(jsonBuf,len);
            log_d("send = %d\r\n",len);            
            break;
        case AUTH_MODE_REMOTE:

            break;
        case AUTH_MODE_UNBIND:
            //直接发送停用设备指令
            xQueueReset(xDataProcessQueue); 
            log_d("send AUTH_MODE_UNBIND floor\r\n");
            break;
        case AUTH_MODE_BIND:
            //直接发送启动设置指令
            xQueueReset(xDataProcessQueue); 
            log_d("send AUTH_MODE_BIND floor\r\n");
            break;
        default:
            log_d("invalid authMode\r\n");
            break;    
   }

}

SYSERRORCODE_E authReader(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t key[CARD_NO_BCD_LEN+1] = {0};  
    uint8_t isFind = 0;  
    
    memset(key,0x00,sizeof(key)); 
    log_d("card or QR data = %s,mode = %d\r\n",pQueue->data,pQueue->authMode);
    
    if(pQueue->authMode == AUTH_MODE_QR) 
    {
        //二维码
        log_d("pQueue->data = %s\r\n",pQueue->data);

        localUserData->authMode = pQueue->authMode; 
        isFind = parseQrCode(pQueue->data,localUserData);

        log_d("isfind = %d\r\n",isFind);      

        if(isFind != NO_ERR)
        {
            //未找到记录，无权限
            log_d("not find record\r\n");
            return NO_AUTHARITY_ERR;
        }         
       
    }
    else
    {
        //读卡 CARD 230000000089E1E35D,23         
        memcpy(key,pQueue->data,CARD_NO_BCD_LEN);
        log_d("key = %02x,%02x,%02x,%02x\r\n",key[0],key[1],key[2],key[3]);     
        
        isFind = readUserData(key,CARD_MODE,localUserData);   

        log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData->cardState);

        if(localUserData->cardState != CARD_VALID || isFind != 0)
        {
            //未找到记录，无权限
            log_e("not find record\r\n");
            return NO_AUTHARITY_ERR;
        } 
        
        localUserData->platformType = 4;
        localUserData->authMode = pQueue->authMode; 
    }

    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);  
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);
    log_d("localUserData->platformType = %s\r\n",localUserData->platformType);

    return result;
}




SYSERRORCODE_E authRemote(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    char value[128] = {0};
    int val_len = 0;
    char *buf[6] = {0}; //存放分割后的子字符串 
    int num = 0;
    uint8_t key[8+1] = {0};    

    memset(key,0x00,sizeof(key));   
    
    memset(value,0x00,sizeof(value));

    val_len = ef_get_env_blob((const char*)key, value, sizeof(value) , NULL);
   

    log_d("get env = %s,val_len = %d\r\n",value,val_len);

    if(val_len <= 0)
    {
        //未找到记录，无权限
        log_e("not find record\r\n");
        return NO_AUTHARITY_ERR;
    }

    split(value,";",buf,&num); //调用函数进行分割 
    log_d("num = %d\r\n",num);

    if(num != 5)
    {
        log_e("read record error\r\n");
        return READ_RECORD_ERR;       
    }

    localUserData->authMode = pQueue->authMode;    
    
    if(AUTH_MODE_QR == pQueue->authMode)
    {        
        strcpy((char*)localUserData->cardNo,buf[0]);        
    }
    else
    {
        memcpy(localUserData->cardNo,key,CARD_NO_LEN);

        log_d("buf[0] = %s\r\n",buf[0]);     
    }   


    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);    
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    return result;

}

static void calcFloor(uint8_t layer,uint8_t regMode,uint8_t *src,uint8_t *outFloor)
{
    uint8_t div = 0;
    uint8_t remainder = 0;
    uint8_t floor = layer + 3; //这里是因为有地下三层
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    uint8_t tmpFloor = 0;
    uint8_t index = 0;
    
    memcpy(sendBuf,src,MAX_SEND_LEN);

//    dbh("before", sendBuf, MAX_SEND_LEN);
        
    div = floor / 8;
    remainder = floor % 8;

    if(regMode == AUTO_REG)
    {
        index = div + 8;
    }
    else
    {
        index = div;
    }

    log_d("div = %d,remain = %d\r\n",div,remainder);
    

    if(div != 0 && remainder == 0)// 8,16,24
    {       
        sendBuf[index-1] = setbit(sendBuf[index-1],8-1);
    } 
    else //1~7层和非8层的倍数
    {
        sendBuf[index] = setbit(sendBuf[index],remainder-1);
    }

    memcpy(outFloor,sendBuf,MAX_SEND_LEN);

//    dbh("after", sendBuf, MAX_SEND_LEN);
}





