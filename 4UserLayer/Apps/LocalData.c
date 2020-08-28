/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : LocalData.c
  版 本 号   : 初稿
  作    者   :
  生成日期   : 2020年3月21日
  最近修改   :
  功能描述   : 管理本地数据，对卡号，用户ID为索引的数-
                   据进行增删改查
  函数列表   :
  修改历史   :
  1.日    期   : 2020年3月21日
    作    者   :
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
// *----------------------------------------------*/
//volatile uint16_t gCurCardHeaderIndex = 0;
//volatile uint16_t gCurUserHeaderIndex = 0;
//volatile uint16_t gCurRecordIndex = 0;
//volatile uint16_t gDelCardHeaderIndex = 0;    //已删除卡号索引
//volatile uint16_t gDelUserHeaderIndex = 0;    //已删除用户ID索引

USERDATA_STRU gUserDataStru = {0};

HEADINFO_STRU gSectorBuff[512] = {0};

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace ( uint8_t mode );
static void eraseUserDataIndex ( void );
static ISFIND_ENUM findIndex ( uint8_t* header,uint32_t address,uint16_t curIndex,uint16_t* index );
static uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex );

//static uint8_t delSourceHeader ( uint16_t index,uint8_t mode );


static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target);

static int Bin_Search_Head(HEADINFO_STRU *num,int numsSize,int target);





uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex )
{
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t temp[HEAD_lEN*2+1] = {0};
	uint32_t addr = 0;
	uint32_t value = 0;

	memset ( readBuff,0x00,sizeof ( readBuff ) );
	//写入已删除的空间中,根据已删除索引，获取到当前索引下卡的索引

	if ( mode == CARD_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR + curIndex * HEAD_lEN;
	}

	FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );

	bcd2asc ( temp, readBuff,CARD_NO_LEN_ASC, 1 );

	value = atoi ( ( const char* ) temp );


	return value;

}


void eraseUserDataAll ( void )
{
	int32_t iTime1, iTime2;
	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
	eraseHeadSector();
//	eraseDataSector();
    eraseUserDataIndex();
	clearTemplateFRAM();
    initTemplateParam();	
	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "eraseUserDataAll成功，耗时: %dms\r\n",iTime2 - iTime1 );
}

static void eraseUserDataIndex ( void )
{
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,WRITE_PRARM);
}


void eraseHeadSector ( void )
{
	FRAM_Erase ( FM24V10_1,0,122880 );	
}

void eraseDataSector ( void )
{
	uint16_t i = 0;

	for ( i=0; i<DATA_SECTOR_NUM; i++ )
	{
		bsp_sf_EraseSector ( CARD_NO_DATA_ADDR+i*SECTOR_SIZE );
		bsp_sf_EraseSector ( USER_ID_DATA_ADDR+i*SECTOR_SIZE );
	}
}

#if 0
uint8_t writeUserData ( USERDATA_STRU *userData,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	uint32_t index = 0;
    uint8_t head[8] = {0};

	int32_t iTime1, iTime2;

	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */

	//检查存储空间是否已满；
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		return 1; //提示已经满了
	}

	if ( mode == CARD_MODE )
	{
	    asc2bcd ( head, userData->cardNo,CARD_NO_LEN_ASC, 1 );	
	}


    //写表头
	addHead(head,mode);

    //读当前FLASH的索引
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	//获取地址
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + (gRecordIndex.cardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = USER_ID_DATA_ADDR + (gRecordIndex.delCardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU )-1 );


	//这里应该打印一下WBUFF，看数据是否正确

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU )-1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//再读出来，对比是否一致
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );
		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("写交易记录表失败!\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_e ( "writeUserData成功，耗时: %dms\r\n",iTime2 - iTime1 );

//    dbh("writeUserData", wBuff, sizeof(USERDATA_STRU));

	return 0;
}

uint8_t readUserData ( uint8_t* header,uint8_t mode,USERDATA_STRU* userData )
{
	uint8_t rBuff[255] = {0};
	uint8_t crc = 0;
	uint8_t ret = 0;
	int index = 0;
	uint32_t addr = 0;

    int32_t iTime1, iTime2;

	if ( header == NULL )
	{
		//log_d("invalid head\r\n");
		return 1; //提示错误的参数
	}	
	
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */

	index = readHead(header,mode);
	
    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("readUserData成功，耗时: %dms\r\n",iTime2 - iTime1);	

	if(index == NO_FIND_HEAD)
	{
        log_d("no find head\r\n");
        return 2;
	}

	return 0;

}


#endif

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



uint8_t modifyUserData ( USERDATA_STRU *userData,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t header[CARD_USER_LEN] = {0};
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	int index = 0;
    uint8_t head[4] = {0};

	int32_t iTime1, iTime2;
	//log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */


	//检查存储空间是否已满；
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		//log_d("not enough speac storage the data\r\n");
		return 1; //提示已经满了
	}

	if ( mode == CARD_MODE )
	{
        asc2bcd (head, userData->cardNo,CARD_NO_LEN_ASC, 1 );
	
	}



    index = readHead(head,mode);

	//log_d("searchHeaderIndex ret = %d",ret);

	if ( index ==  NO_FIND_HEAD)
	{
		//log_d("can't find the head index\r\n");
		return 3;//提示未找到索引
	}

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}


	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU ) );

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU ) - 1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//再读出来，对比是否一致
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("modify record is error\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "修改记录成功，耗时: %dms\r\n",iTime2 - iTime1 );

	return 0;
}


uint8_t writeZeaoHead (uint8_t multiple,uint16_t remainder,HEADINFO_STRU *card)
{

	uint8_t readBuff[HEAD_lEN] = {0};
	uint8_t ret = 0;
	uint32_t addr = 0;
	HEADINFO_STRU temp[1] = {0};
	
	addr = CARD_NO_HEAD_ADDR + (multiple * HEAD_NUM_SECTOR + remainder)* sizeof(HEADINFO_STRU);

    memcpy(temp,card,1);
	
    log_d("del id = %x,flash addr = %d\r\n",temp[0].headData.id,temp[0].flashAddr);
    
	dbh("del sn", (char *)temp[0].headData.sn, 4);
	
	ret = FRAM_Write ( FM24V10_1, addr, temp,sizeof(HEADINFO_STRU) );


	if ( ret != 0 )
	{
		return 3;
	}

	return 0;
}






void TestFlash ( uint8_t mode )
{
	char buff[156] = {0};
    HEADINFO_STRU tmp;
	uint32_t addr = 0;
	uint32_t data_addr = 0;
	uint16_t i = 0;
	uint32_t num = 0;

	if ( buff == NULL )
	{
		//log_d("my_malloc error\r\n");
		return ;
	}
	
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    


	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		data_addr = CARD_NO_DATA_ADDR;
		num = gRecordIndex.cardNoIndex;
	}

	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		num = gRecordIndex.delCardNoIndex;
	}


	for ( i=0; i<num; i++ )
	{
        memset ( &tmp,0x00,sizeof ( HEADINFO_STRU ) );
		memset ( buff,0x00,sizeof ( buff ) );
		
		FRAM_Read ( FM24V10_1, addr+i*sizeof ( HEADINFO_STRU ), &tmp, sizeof ( HEADINFO_STRU ) );
		bcd2asc ( ( uint8_t* ) buff, tmp.headData.sn, CARD_NO_LEN_ASC, 0 );
		printf ( "the %d ==========> header = %s,flash addr =%d %s\r\n",i,buff,tmp.flashAddr);

	}

//	for ( i=0; i<num; i++ )
//	{
//		memset ( buff,0x00,sizeof ( buff ) );
//		bsp_sf_ReadBuffer ( ( uint8_t* ) buff, data_addr+i * ( sizeof ( USERDATA_STRU ) ), sizeof ( USERDATA_STRU ) );
//		printf ( "the %d data ====================== \r\n",i );
//		dbh ( "data", buff, ( sizeof ( USERDATA_STRU ) ) );

//	}

}

#if 0

//ret success 当前ID在FLASH中的索引;fail NO_FIND_HEAD
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

    //512个头，1页的空间
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

    iTime1 = xTaskGetTickCount();   /* 记下开始时间 */

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

    //1.读取单页或者多页最后一页的地址
    if(multiple > 0)
    {
        address += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    
    
    //2.读取最后一页第一个卡号和最后一个卡号；
    ret = FRAM_Read (FM24V10_1, address, sectorBuff, (remainder)* HEAD_lEN);

    
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
        address += i * HEAD_NUM_SECTOR  * HEAD_lEN;
        
        //2.读取第一个卡号和最后一个卡号；
        ret = FRAM_Read (FM24V10_1, address, sectorBuff, HEAD_NUM_SECTOR * HEAD_lEN);
        
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

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "read Head success，use %d ms\r\n",iTime2 - iTime1 );    

    myfree(SRAMIN,sectorBuff);
    return NO_FIND_HEAD;

}
#endif

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

	if ( headBuff == NULL )
	{
		return NO_FIND_HEAD;
	}	

	if(headBuff[0] == 0x01)
	{
	    log_d("card status:del\r\n");
	    return NO_FIND_HEAD; //已删除卡
	}

    iTime1 = xTaskGetTickCount();   /* 记下开始时间 */

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

    //1.读取单页或者多页最后一页的地址
    if(multiple > 0)
    {
        address += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));
    
    //2.读取最后一页第一个卡号和最后一个卡号；
    ret = FRAM_Read (FM24V10_1, address, gSectorBuff, (remainder)* HEAD_lEN);

    
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
        address += i * HEAD_NUM_SECTOR  * HEAD_lEN;
        
        //2.读取第一个卡号和最后一个卡号；
        ret = FRAM_Read (FM24V10_1, address, gSectorBuff, HEAD_NUM_SECTOR * HEAD_lEN);
        
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

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "read Head success，use %d ms\r\n",iTime2 - iTime1 );    

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
        /* 找到数组中第一个无序的数，保存为tmp */
        if (head[i].headData.id < head[i - 1].headData.id)
        {
            tmp = head[i];
        }
        else
        {
            continue;
        }
		/* 找到数组中第一个无序的数，保存为tmp */

		/* 二分查询开始 */
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
		/* 二分查询结束,此时a[left]>=a[i],记录下left的值 */

		/* 将有序数组中比要插入的数大的数右移 */
        for (int j = i; j > left; j--)
        {
            head[j] = head[j - 1];
        }
		/* 将有序数组中比要插入的数大的数右移 */

		// 将left位置赋值为要插入的数
        head[left] = tmp;
    }
}

//return:<0,未找到，>=0在FLASH中的索引值
static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target)
{
	int left = 0,right = numsSize-1,mid;
	
	while(left <= right)
	{
		mid = (left + right) / 2;//确定中间元素	
		if(num[mid].headData.id > target)
		{
			right = mid-1; //mid已经交换过了,right往前移一位
		}
		else if(num[mid].headData.id < target)
		{
			left = mid+1;//mid已经交换过了,left往后移一位
		}	
		else //判断是否相等
		{		    
            return num[mid].flashAddr;
		}
	}        
    return NO_FIND_HEAD;
}

//返回查找到的卡号的索引
static int Bin_Search_Head(HEADINFO_STRU *num,int numsSize,int target)
{
	int left = 0,right = numsSize-1,mid;
	
	while(left <= right)
	{
		mid = (left + right) / 2;//确定中间元素	
		if(num[mid].headData.id > target)
		{
			right = mid-1; //mid已经交换过了,right往前移一位
		}
		else if(num[mid].headData.id < target)
		{
			left = mid+1;//mid已经交换过了,left往后移一位
		}	
		else //判断是否相等
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
	uint8_t i = 0;

    int32_t iTime1, iTime2;

    log_d("head %02x,%02x,%02x,%02x\r\n",head[0],head[1],head[2],head[3]);   

    
   iTime1 = xTaskGetTickCount();   /* 记下开始时间 */
   //1.先判定当前有多少个卡号;
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

    //索引要从1开始    
    if(multiple==0 && remainder==0)
    {
        //第一条记录
        gSectorBuff[0].flashAddr = curIndex;
        memcpy(gSectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);  

        log_d("<<<<<write first recond>>>>>\r\n");
//        log_d("add = %x,sectorBuff[0].headData.sn = %02x,%02x,%02x,%02x,addr = %d\r\n",addr,gSectorBuff[0].headData.sn[0],gSectorBuff[0].headData.sn[1],gSectorBuff[0].headData.sn[2],gSectorBuff[0].headData.sn[3],gSectorBuff[0].flashAddr);
        
        //写入到存储区域
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,1 * sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ret;
        }        
    }
    else 
    {
        //2.追加到最后一页；
        if(multiple > 0)
        {
            addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);  
        }

        //3.读取最后一页
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
        
        //4.赋值,追加需要添加的卡号到最后一个位置
        gSectorBuff[remainder].flashAddr = curIndex;
        memcpy(gSectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);
        
        log_d("add head = %x,sn = %02x %02x %02x %02x", gSectorBuff[remainder].headData.id, gSectorBuff[remainder].headData.sn[0],gSectorBuff[remainder].headData.sn[1],gSectorBuff[remainder].headData.sn[2],gSectorBuff[remainder].headData.sn[3]);  

        
        //5.排序
        sortHead(gSectorBuff,remainder+1);

//        log_d("<<<<<<<<<< >>>>>>>>>>\r\n");
        
//        for(i=0;i<remainder+1;i++)
//        {
//            log_d("add = %d,id =%x,sn =%02x,%02x,%02x,%02x,gSectorBuff.flashAddr = %d\r\n",addr,gSectorBuff[i].headData.id,gSectorBuff[i].headData.sn[0],gSectorBuff[i].headData.sn[1],gSectorBuff[i].headData.sn[2],gSectorBuff[i].headData.sn[3],gSectorBuff[i].flashAddr);
//        }

        //6.写入到存储区域
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

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "add head成功，耗时: %dms\r\n",iTime2 - iTime1 );


    return 1;
  
}


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
    
    
   iTime1 = xTaskGetTickCount();   /* 记下开始时间 */
   //1.先判定当前有多少个卡号;
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


    //1.读取单页或者多页最后一页的地址
    if(multiple > 0)
    {
        addr += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",addr,multiple,remainder);
    
    memset(gSectorBuff,0x00,sizeof(gSectorBuff));
    
    //2.读取最后一页第一个卡号和最后一个卡号；
    ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* HEAD_lEN);    

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
        ret = Bin_Search_Head(gSectorBuff,remainder,head.headData.id);
        if(ret != NO_FIND_HEAD)
        {
            log_d("s del card success,the index = %d,value = %x\r\n",ret,gSectorBuff[ret].headData.id);

            gSectorBuff[ret].headData.sn[0] = 0x01;
            
            //找到，要修改该页该索引的值，然后排序 
//            writeZeaoHead(multiple,ret,&gSectorBuff[ret]);
            
            sortHead(gSectorBuff,remainder); 

            ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,remainder * sizeof(HEADINFO_STRU));
            
            if(ret == 0)
            {
                log_d("write fram error\r\n");
                return NO_FIND_HEAD;
            }        
            
            //删除索引++，并记录当前页及索引，当前页是会变的
            //这个暂时不做
            log_d("del card success\r\n");
            return ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        addr += i * HEAD_NUM_SECTOR  * HEAD_lEN;

        memset(gSectorBuff,0x00,sizeof(gSectorBuff));
        
        //2.读取第一个卡号和最后一个卡号；
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, HEAD_NUM_SECTOR * HEAD_lEN);
        
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
                //找到，要修改该页该索引的值，然后排序
                gSectorBuff[ret].headData.sn[0] = 0x01;
                log_d("m del card success,the index = %d,value = %x\r\n",ret,gSectorBuff[ret].headData.id);
                
                //找到，要修改该页该索引的值，然后排序 
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





