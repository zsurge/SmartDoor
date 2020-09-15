/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : LocalData.c
  �� �� ��   : ����
  ��    ��   :
  ��������   : 2020��3��21��
  ����޸�   :
  ��������   : ���������ݣ��Կ��ţ��û�IDΪ��������-
                   �ݽ�����ɾ�Ĳ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��3��21��
    ��    ��   :
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "LocalData.h"
#include "bsp_spi_flash.h"
#include "easyflash.h"
#include "stdio.h"
#include "tool.h"
#include "malloc.h"
#include "bsp_MB85RC128.h"
#include "deviceInfo.h"



#define LOG_TAG    "localData"
#include "elog.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
// *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace ( uint8_t mode );

static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target);



static uint8_t checkFlashSpace ( uint8_t mode )
{
	if ( mode == CARD_MODE )
	{
		if ( gRecordIndex.cardNoIndex > MAX_HEAD_RECORD-5 )
		{
			//log_d("card flash is full\r\n");

			return 1;
		}
	}
	else
	{
		if ( gRecordIndex.delCardNoIndex > MAX_HEAD_DEL_CARDNO-5 )
		{
			//log_d("card flash is full\r\n");
			return 1;
		}
	}


	return 0;
}


#if 0

//ret success ��ǰID��FLASH�е�����;fail NO_FIND_HEAD
int readHead(uint8_t *headBuff,uint8_t mode)
{
	uint8_t i = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint16_t loop = 0;
	uint32_t address = 0;
	uint32_t curIndex = 0;
	
	int32_t iTime1, iTime2;
	 
	int ret = 0;
    HEADINFO_STRU head;

    //512��ͷ��1ҳ�Ŀռ�
    HEADINFO_STRU * sectorBuff = (HEADINFO_STRU*)mymalloc(SRAMIN,sizeof(HEADINFO_STRU)*HEAD_NUM_SECTOR);    

    
    if (sectorBuff == NULL)
    {
       myfree(SRAMIN,sectorBuff);
       log_d("my_malloc error\r\n");
       return NO_FIND_HEAD;
    }

	if ( headBuff == NULL )
	{
	    myfree(SRAMIN,sectorBuff);
		return NO_FIND_HEAD;
	}	

    iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */

	memcpy(head.headData.sn,headBuff,sizeof(head.headData.sn));

    log_d("wang find head.headData.id = %x,sn = %02x,%02x,%02x,%02x\r\n",head.headData.id,head.headData.sn[0],head.headData.sn[1],head.headData.sn[2],head.headData.sn[3]);
	

    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    

	if ( mode == CARD_MODE )
	{
		address = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		address = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	
	multiple = curIndex / HEAD_NUM_SECTOR;
	remainder = curIndex % HEAD_NUM_SECTOR;

    //1.��ȡ��ҳ���߶�ҳ���һҳ�ĵ�ַ
    if(multiple > 0)
    {
        address += multiple * HEAD_NUM_SECTOR  * CARD_USER_LEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    
    
    //2.��ȡ���һҳ��һ�����ź����һ�����ţ�
    ret = FRAM_Read (FM24V10_1, address, sectorBuff, (remainder)* CARD_USER_LEN);

    
    log_d("FRAM_Read SUCCESS addr = %x,remainder = %d\r\n",address,remainder);
    
//    for(i=0;i<remainder;i++)
//    {
//        log_d("add = %x,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",address,sectorBuff[i].headData.id,sectorBuff[i].headData.sn[0],sectorBuff[i].headData.sn[1],sectorBuff[i].headData.sn[2],sectorBuff[i].headData.sn[3],sectorBuff[i].flashAddr);
//    }    
    
    log_d("head = %x,last page %x,%x\r\n",head.headData.id,sectorBuff[0].headData.id,sectorBuff[remainder-1].headData.id);
    
    if(ret == 0)
    {
        myfree(SRAMIN,sectorBuff);
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;       

    }   
    
    log_d("head = %x,last page %x,%x\r\n",head.headData.id,sectorBuff[0].headData.id,sectorBuff[remainder-1].headData.id);

    
    if((head.headData.id >= sectorBuff[0].headData.id) && (head.headData.id <= sectorBuff[remainder-1].headData.id))
    {
    
        ret = Bin_Search(sectorBuff,remainder,head.headData.id);

        log_d("1.Bin_Search = %d\r\n",ret);
        
        if(ret != NO_FIND_HEAD)
        {
            log_d("find it\r\n");
            myfree(SRAMIN,sectorBuff);
            return ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        address += i * HEAD_NUM_SECTOR  * CARD_USER_LEN;
        
        //2.��ȡ��һ�����ź����һ�����ţ�
        ret = FRAM_Read (FM24V10_1, address, sectorBuff, HEAD_NUM_SECTOR * CARD_USER_LEN);
        
        if(ret == 0)
        {
            myfree(SRAMIN,sectorBuff);
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
        
        if(head.headData.id >= sectorBuff[0].headData.id && head.headData.id <= sectorBuff[HEAD_NUM_SECTOR-1].headData.id)
        {
            ret = Bin_Search(sectorBuff,HEAD_NUM_SECTOR,head.headData.id);
            if(ret != NO_FIND_HEAD)
            {
                myfree(SRAMIN,sectorBuff);
                return ret;
            }
        }
    
    }

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "read Head success��use %d ms\r\n",iTime2 - iTime1 );    

    myfree(SRAMIN,sectorBuff);
    return NO_FIND_HEAD;

}
#endif

int readHead(uint8_t *headBuff,uint8_t mode)
{
	uint8_t i = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t address = 0;
	uint32_t curIndex = 0;
	
	int32_t iTime1, iTime2;
	 
	int ret = 0;
    HEADINFO_STRU head;

	if ( headBuff == NULL )
	{
		return NO_FIND_HEAD;
	}	

	if(headBuff[0] == 0x01)
	{
	    log_d("card status:del\r\n");
	    return NO_FIND_HEAD; //��ɾ����
	}

    iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */

	memcpy(head.headData.sn,headBuff,sizeof(head.headData.sn));

    log_d("wang find head.headData.id = %x,sn = %02x,%02x,%02x,%02x\r\n",head.headData.id,head.headData.sn[0],head.headData.sn[1],head.headData.sn[2],head.headData.sn[3]);
	

    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    

	if ( mode == CARD_MODE )
	{
		address = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		address = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	
	multiple = curIndex / HEAD_NUM_SECTOR;
	remainder = curIndex % HEAD_NUM_SECTOR;

    //1.��ȡ��ҳ���߶�ҳ���һҳ�ĵ�ַ
    if(multiple > 0)
    {
        address += multiple * HEAD_NUM_SECTOR  * CARD_USER_LEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));
    
    //2.��ȡ���һҳ��һ�����ź����һ�����ţ�
    ret = FRAM_Read (FM24V10_1, address, gSectorBuff, (remainder)* CARD_USER_LEN);

    
    log_d("FRAM_Read SUCCESS addr = %x,remainder = %d\r\n",address,remainder);
    
//    for(i=0;i<remainder;i++)
//    {
//        log_d("add = %x,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",address,gSectorBuff[i].headData.id,sectorBuff[i].headData.sn[0],sectorBuff[i].headData.sn[1],sectorBuff[i].headData.sn[2],sectorBuff[i].headData.sn[3],sectorBuff[i].flashAddr);
//    }    
    
    log_d("head = %x,last page %x,%x\r\n",head.headData.id,gSectorBuff[0].headData.id,gSectorBuff[remainder-1].headData.id);
    
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;       

    }   
    
    log_d("head = %x,last page %x,%x\r\n",head.headData.id,gSectorBuff[0].headData.id,gSectorBuff[remainder-1].headData.id);

    
    if((head.headData.id >= gSectorBuff[0].headData.id) && (head.headData.id <= gSectorBuff[remainder-1].headData.id))
    {
    
        ret = Bin_Search(gSectorBuff,remainder,head.headData.id);

        log_d("1.Bin_Search flash index = %d\r\n",ret);
        
        if(ret != NO_FIND_HEAD)
        {
            log_d("find it\r\n");
            return ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        address += i * HEAD_NUM_SECTOR  * CARD_USER_LEN;
        
        //2.��ȡ��һ�����ź����һ�����ţ�
        ret = FRAM_Read (FM24V10_1, address, gSectorBuff, HEAD_NUM_SECTOR * CARD_USER_LEN);
        
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
        
        if(head.headData.id >= gSectorBuff[0].headData.id && head.headData.id <= gSectorBuff[HEAD_NUM_SECTOR-1].headData.id)
        {
            ret = Bin_Search(gSectorBuff,HEAD_NUM_SECTOR,head.headData.id);
            if(ret != NO_FIND_HEAD)
            {
                return ret;
            }
        }
    
    }

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "read Head success��use %d ms\r\n",iTime2 - iTime1 );    

    return NO_FIND_HEAD;

}



void sortHead(HEADINFO_STRU *head,int length)
{
    int left, right, mid;
    HEADINFO_STRU tmp;

    memset(&tmp,0x00,sizeof(tmp));

    log_d("sortHead length = %d\r\n",length);
    
    for (int i = 1; i < length; i++)
    {
        /* �ҵ������е�һ���������������Ϊtmp */
        if (head[i].headData.id < head[i - 1].headData.id)
        {
            tmp = head[i];
        }
        else
        {
            continue;
        }
		/* �ҵ������е�һ���������������Ϊtmp */

		/* ���ֲ�ѯ��ʼ */
        left = 0;
        right = i - 1;
        while (left <= right)
        {
            mid = (left + right) / 2;
            if (head[mid].headData.id > tmp.headData.id)
            {
                right = mid - 1;
            }
            else
            {
                left = mid + 1;
            }
        }
		/* ���ֲ�ѯ����,��ʱa[left]>=a[i],��¼��left��ֵ */

		/* �����������б�Ҫ���������������� */
        for (int j = i; j > left; j--)
        {
            head[j] = head[j - 1];
        }
		/* �����������б�Ҫ���������������� */

		// ��leftλ�ø�ֵΪҪ�������
        head[left] = tmp;
    }
}

//return:<0,δ�ҵ���>=0//���ز��ҵ��Ŀ��ŵ�����
static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target)
{
	int left = 0,right = numsSize-1,mid;
	
	while(left <= right)
	{
		mid = (left + right) / 2;//ȷ���м�Ԫ��	
		if(num[mid].headData.id > target)
		{
			right = mid-1; //mid�Ѿ���������,right��ǰ��һλ
		}
		else if(num[mid].headData.id < target)
		{
			left = mid+1;//mid�Ѿ���������,left������һλ
		}	
		else //�ж��Ƿ����
		{		    
            return mid;
		}
	}        
    return NO_FIND_HEAD;
}







uint8_t addHead(uint8_t *head,uint8_t mode)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t addr = 0;
	uint8_t ret = 0;
	uint32_t curIndex = 0;

    int32_t iTime1, iTime2;

    log_d("head %02x,%02x,%02x,%02x\r\n",head[0],head[1],head[2],head[3]);   

    
   iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
   //1.���ж���ǰ�ж��ٸ�����;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
   
    
    multiple = curIndex / HEAD_NUM_SECTOR;
    remainder = curIndex % HEAD_NUM_SECTOR;

//    log_d("mode = %d,addr = %x,multiple = %d,remainder=%d\r\n",mode,addr,multiple,remainder);

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));

    //����Ҫ��1��ʼ    
    if(multiple==0 && remainder==0)
    {
        //��һ����¼
        memcpy(gSectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);  

        log_d("<<<<<write first recond>>>>>\r\n");
        
        //д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,1 * sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ret;
        }        
    }
    else 
    {
        //2.׷�ӵ����һҳ��
        if(multiple > 0)
        {
            addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);  
        }

        //3.��ȡ���һҳ
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return ret;
        }

        log_d("FRAM_Read SUCCESS addr = %x,remainder = %d\r\n",addr,remainder);
       
//        for(i=0;i<remainder;i++)
//        {
//            log_d("add = %x,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//        }        
        
        //4.��ֵ,׷����Ҫ��ӵĿ��ŵ����һ��λ��
        memcpy(gSectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);
        
        log_d("add head = %x,sn = %02x %02x %02x %02x", gSectorBuff[remainder].headData.id, gSectorBuff[remainder].headData.sn[0],gSectorBuff[remainder].headData.sn[1],gSectorBuff[remainder].headData.sn[2],gSectorBuff[remainder].headData.sn[3]);  

        
        //5.����
        sortHead(gSectorBuff,remainder+1);

//        log_d("<<<<<<<<<< >>>>>>>>>>\r\n");
        
//        for(i=0;i<remainder+1;i++)
//        {
//            log_d("add = %d,id =%x,sn =%02x,%02x,%02x,%02x,gSectorBuff.flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//        }

        //6.д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,(remainder+1)* sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ret;
        }        
    }


	if ( mode == CARD_MODE )
	{
        gRecordIndex.cardNoIndex++;
	}
	else if ( mode == CARD_DEL_MODE )
	{
        gRecordIndex.delCardNoIndex++;		
	}


//    log_d("cardNoIndex = %d,userIdIndex = %d\r\n",gRecordIndex.cardNoIndex,gRecordIndex.userIdIndex);	
    optRecordIndex(&gRecordIndex,WRITE_PRARM);

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "add head�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );


    return 1;
  
}

#if 0
uint8_t addHeadID(uint8_t *head,uint8_t mode)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t addr = 0;
	uint8_t ret = 0;
	uint32_t curIndex = 0;
	uint8_t i = 0;

    int32_t iTime1, iTime2;

    log_d("head %02x,%02x,%02x,%02x\r\n",head[0],head[1],head[2],head[3]);   

    
   iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
   //1.���ж���ǰ�ж��ٸ�����;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
   
    
    multiple = curIndex / HEAD_NUM_SECTOR;
    remainder = curIndex % HEAD_NUM_SECTOR;

//    log_d("mode = %d,addr = %x,multiple = %d,remainder=%d\r\n",mode,addr,multiple,remainder);

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));

    //����Ҫ��1��ʼ    
    if(multiple==0 && remainder==0)
    {
        //��һ����¼
        gSectorBuff[0].flashAddr = curIndex;
        memcpy(gSectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);  

        log_d("<<<<<write first recond>>>>>\r\n");
//        log_d("add = %x,sectorBuff[0].headData.sn = %02x,%02x,%02x,%02x,addr = %d\r\n",addr,gSectorBuff[0].headData.sn[0],gSectorBuff[0].headData.sn[1],gSectorBuff[0].headData.sn[2],gSectorBuff[0].headData.sn[3],gSectorBuff[0].flashAddr);
        
        //д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,1 * sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ret;
        }        
    }
    else 
    {
        //2.׷�ӵ����һҳ��
        if(multiple > 0)
        {
            addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);  
        }

        //3.��ȡ���һҳ
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return ret;
        }

        log_d("FRAM_Read SUCCESS addr = %x,remainder = %d\r\n",addr,remainder);
       
//        for(i=0;i<remainder;i++)
//        {
//            log_d("add = %x,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//        }        
        
        //4.��ֵ,׷����Ҫ��ӵĿ��ŵ����һ��λ��
        gSectorBuff[remainder].flashAddr = curIndex;
        memcpy(gSectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);
        
        log_d("add head = %x,sn = %02x %02x %02x %02x", gSectorBuff[remainder].headData.id, gSectorBuff[remainder].headData.sn[0],gSectorBuff[remainder].headData.sn[1],gSectorBuff[remainder].headData.sn[2],gSectorBuff[remainder].headData.sn[3]);  

        
        //5.����
        sortHead(gSectorBuff,remainder+1);

//        log_d("<<<<<<<<<< >>>>>>>>>>\r\n");
        
//        for(i=0;i<remainder+1;i++)
//        {
//            log_d("add = %d,id =%x,sn =%02x,%02x,%02x,%02x,gSectorBuff.flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//        }

        //6.д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,(remainder+1)* sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ret;
        }        
    }


	if ( mode == CARD_MODE )
	{
        gRecordIndex.cardNoIndex++;
	}
	else if ( mode == CARD_DEL_MODE )
	{
        gRecordIndex.delCardNoIndex++;		
	}


//    log_d("cardNoIndex = %d,userIdIndex = %d\r\n",gRecordIndex.cardNoIndex,gRecordIndex.userIdIndex);	
    optRecordIndex(&gRecordIndex,WRITE_PRARM);

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "add head�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );


    return 1;
  
}
#endif


int delHead(uint8_t *headBuff,uint8_t mode)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t addr = 0;
	int ret = 0;
	uint32_t curIndex = 0;
	uint8_t i = 0;
	
    HEADINFO_STRU head;
    
    int32_t iTime1, iTime2;
	
   memcpy(head.headData.sn,headBuff,sizeof(head.headData.sn));
   log_d("source id = %02x,%02x,%02x,%02x\r\n",headBuff[0],headBuff[1],headBuff[2],headBuff[3]);
   log_d("head.headData.id = %x,sn = %02x,%02x,%02x,%02x\r\n",head.headData.id,head.headData.sn[0],head.headData.sn[1],head.headData.sn[2],head.headData.sn[3]);
    
    
   iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
   //1.���ж���ǰ�ж��ٸ�����;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
   
    
    multiple = curIndex / HEAD_NUM_SECTOR;
    remainder = curIndex % HEAD_NUM_SECTOR;


    //1.��ȡ��ҳ���߶�ҳ���һҳ�ĵ�ַ
    if(multiple > 0)
    {
        addr += multiple * HEAD_NUM_SECTOR  * CARD_USER_LEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",addr,multiple,remainder);
    
    memset(gSectorBuff,0x00,sizeof(gSectorBuff));
    
    //2.��ȡ���һҳ��һ�����ź����һ�����ţ�
    ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* CARD_USER_LEN);    

    log_d("FRAM_Read SUCCESS addr = %x,remainder = %d\r\n",addr,remainder);
    
//    for(i=0;i<remainder;i++)
//    {
//        log_d("add = %x,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//    }    
    
    log_d("head = %x,last page %x,%x\r\n",head.headData.id,gSectorBuff[0].headData.id,gSectorBuff[remainder-1].headData.id);

    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;        
    }      

    
    if((head.headData.id >= gSectorBuff[0].headData.id) && (head.headData.id <= gSectorBuff[remainder-1].headData.id))
    {
        ret = Bin_Search(gSectorBuff,remainder,head.headData.id);
        if(ret != NO_FIND_HEAD)
        {
            log_d("s del card success,the index = %d,value = %x\r\n",ret,gSectorBuff[ret].headData.id);

            gSectorBuff[ret].headData.sn[0] = 0x01;
            
            //�ҵ���Ҫ�޸ĸ�ҳ��������ֵ��Ȼ������ 
//            writeZeaoHead(multiple,ret,&gSectorBuff[ret]);
            
            sortHead(gSectorBuff,remainder); 

            ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,remainder * sizeof(HEADINFO_STRU));
            
            if(ret == 0)
            {
                log_d("write fram error\r\n");
                return NO_FIND_HEAD;
            }        
            
            //ɾ������++������¼��ǰҳ����������ǰҳ�ǻ���
            //�����ʱ����
            log_d("del card success\r\n");
            return ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        addr += i * HEAD_NUM_SECTOR  * CARD_USER_LEN;

        memset(gSectorBuff,0x00,sizeof(gSectorBuff));
        
        //2.��ȡ��һ�����ź����һ�����ţ�
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, HEAD_NUM_SECTOR * CARD_USER_LEN);
        
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
        
        if(head.headData.id >= gSectorBuff[0].headData.id && head.headData.id <= gSectorBuff[HEAD_NUM_SECTOR-1].headData.id)
        {
            ret = Bin_Search(gSectorBuff,HEAD_NUM_SECTOR,head.headData.id);
            if(ret != NO_FIND_HEAD)
            {
                //�ҵ���Ҫ�޸ĸ�ҳ��������ֵ��Ȼ������
                gSectorBuff[ret].headData.sn[0] = 0x01;
                log_d("m del card success,the index = %d,value = %x\r\n",ret,gSectorBuff[ret].headData.id);
                
                //�ҵ���Ҫ�޸ĸ�ҳ��������ֵ��Ȼ������ 
//                writeZeaoHead(multiple,ret,&gSectorBuff[ret]);

                
                sortHead(gSectorBuff,HEAD_NUM_SECTOR);     
                
                ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,HEAD_NUM_SECTOR * sizeof(HEADINFO_STRU));
                
                if(ret == 0)
                {
                    log_d("write fram error\r\n");
                    return NO_FIND_HEAD;
                }        
                
                return ret;           
            }
        }
    
    }    
    
    return NO_FIND_HEAD;
  
}







