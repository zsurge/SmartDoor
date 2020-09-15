/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : jsonUtils.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月19日
  最近修改   :
  功能描述   : JSON数据处理C文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月19日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "jsonutils"
#include "elog.h"

#include "jsonUtils.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
//LOCAL_USER_STRU gLoalUserData;



/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : modifyJsonItem
 功能描述  : 修改或者新增JSON数据包中指令item的值
 输入参数  : const char *srcJson   json数据包
             const char *item    需要更新值的key 
             const char *value   需要更新的value 
             uint8_t isSubitem   =1 是 data内数据 
             char *descJson      更新后的json数据包
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
SYSERRORCODE_E modifyJsonItem(const char *srcJson,const char *item,const char *value,uint8_t isSubitem,char *descJson);
    修改内容   : 新生成函数

*****************************************************************************/
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson)
{
    cJSON *root ,*dataObj;
    char *tmpBuf;

    if(!srcJson)
    {
        cJSON_Delete(root);
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }    
    
    root = cJSON_Parse((char *)srcJson);    //解析数据包
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    if(isSubitem == 1)
    {
        //根据协议，默认所有的子项是data
        dataObj = cJSON_GetObjectItem ( root, "data" );         
        cJSON_AddStringToObject(dataObj,(const char*)item,(const char*)value);
    }
    else
    {
        cJSON_AddStringToObject(root,(const char*)item,(const char*)value);
    }  

    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        cJSON_Delete(root);
         my_free(tmpBuf);
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);


//    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
    
    return NO_ERR;

}



/*****************************************************************************
 函 数 名  : GetJsonItem
 功能描述  : 获取JSON字符串中指定项目的值
 输入参数  : const char *jsonBuff  json字符串
           const char *item      要读到的KEY
           uint8_t isSubitem     是否读DATA内的项目，=1 读data内项目；=0 读root下项目
 输出参数  : 无
 返 回 值  : char *

 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
 char *GetJsonItem(const char *jsonBuff,const char *item,uint8_t isSubitem)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem)
{
	static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
	cJSON* root,*json_item,*dataObj;
	

    log_d("<<<<%s>>>>\r\n",jsonBuff);

    if(strlen((const char*)jsonBuff) == 0 || strlen((const char*)jsonBuff) > JSON_ITEM_MAX_LEN )
    {
        cJSON_Delete(root);
        log_d ( "invalid data\r\n");       
		return NULL;
    }
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包

	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
		return NULL;
	}
	else
	{
        if(isSubitem == 1)
        {
            //根据协议，默认所有的子项是data
            dataObj = cJSON_GetObjectItem ( root, "data" );  
            json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, (const char*)item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//避免溢出
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}
			else
			{
			    strcpy ( (char*)value, json_item->valuestring );
			}
		}
		else if ( json_item->type == cJSON_Number )
		{
			sprintf ( (char*)value,"%d",json_item->valueint );
		}
//		else if( json_item->type == cJSON_Array )
//		{

//            //  2.日    期   : 2020年4月11日
//            //    作    者   :  
//            //    修改内容   : 添加对数组的支持，返回值还不完善    
//            tmpArrayNum = cJSON_GetArraySize(json_item);

//            for(int n=0;n<tmpArrayNum;n++)
//            {
//                arrayElement = cJSON_GetArrayItem(json_item, n);                 
//                strcpy ((char*)value, arrayElement->valuestring );            
//                log_d("cJSON_Array = %s\r\n",arrayElement->valuestring );
//            }
//		}
		else
		{
			log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
			return NULL;
		}

	}

    cJSON_Delete(root);

        
	return value;
}

SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson)
{
	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*newroot,*dataObj,*json_cmdid,*json_devcode,*identification;
    char *tmpBuf;
    char buf[8] = {0};
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
		return CJSON_PARSE_ERR;
	}
	else
	{
        json_cmdid = cJSON_GetObjectItem ( root, "commandCode" );
        json_devcode = cJSON_GetObjectItem ( root, "deviceCode" );
        identification = cJSON_GetObjectItem ( root, "data" );

        newroot = cJSON_CreateObject();
        dataObj = cJSON_CreateObject();
        if(!newroot && !dataObj)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
    		return CJSON_CREATE_ERR;
        }

        cJSON_AddStringToObject(newroot, "commandCode", json_cmdid->valuestring);
        cJSON_AddStringToObject(newroot, "deviceCode", json_devcode->valuestring);
        cJSON_AddStringToObject(newroot, "identification", identification->valuestring);

        cJSON_AddItemToObject(newroot, "data", dataObj);

        cJSON_AddStringToObject(dataObj, "version", (const char*)gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", (const char*)gDevinfo.Model);

        memset(buf,0x00,sizeof(buf));
        sprintf(buf,"%d",gRecordIndex.cardNoIndex);
        cJSON_AddStringToObject(dataObj, "regRersion", buf);
        cJSON_AddStringToObject(dataObj, "regface", " ");
        cJSON_AddStringToObject(dataObj, "ip", (const char*)gDevinfo.GetIP());
                
        tmpBuf = cJSON_PrintUnformatted(newroot); 

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");
            cJSON_Delete(root);
            cJSON_Delete(newroot);         
            my_free(tmpBuf);            
            return CJSON_FORMAT_ERR;
        }    

        strcpy((char *)descJson,tmpBuf);

	}

    cJSON_Delete(root);
    cJSON_Delete(newroot);

    my_free(tmpBuf);

    return result;
}

SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    char up_status[7] = {0};  
    uint8_t value[200] = {0};
    uint8_t tmp[300] = {0};
    uint8_t send[300] = {0};
    
    ef_get_env_blob ( "upData", value, sizeof ( value ), NULL );

    log_d("tmpBuf = %s\r\n",value);

    result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"commandCode",(const uint8_t *)"1017",0,tmp);    
    

    strcpy(up_status,(const char*)ef_get_env("up_status"));
    
    //升级失败
    if(memcmp(up_status,"101700",6) == 0)
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"2",1,send);

    }
    else if(memcmp(up_status,"101711",6) == 0) //升级成功
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101722",6) == 0) //升级成功
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101733",6) == 0) //禁止升级
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"3",1,send);

    }
    else
    {
        //无升级动作
    }    
 
    strcpy((char *)descBuf,(const char*)send);


    return result;

}

SYSERRORCODE_E saveUpgradeData(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    
    cJSON* root,*newroot,*tmpdataObj,*dataObj,*json_devCode,*productionModel,*id;
    cJSON* version,*softwareFirmware,*versionType;
    char *tmpBuf;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );        
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    }

    newroot = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();
    if(!newroot && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
        return CJSON_PARSE_ERR;
    }  
    
    json_devCode = cJSON_GetObjectItem ( root, "deviceCode" );    
    tmpdataObj = cJSON_GetObjectItem ( root, "data" );        
    productionModel = cJSON_GetObjectItem ( tmpdataObj, "productionModel" );
    id = cJSON_GetObjectItem ( tmpdataObj, "id" );
    version = cJSON_GetObjectItem ( tmpdataObj, "version" );
    softwareFirmware = cJSON_GetObjectItem ( tmpdataObj, "softwareFirmware" );
    versionType = cJSON_GetObjectItem ( tmpdataObj, "versionType" ); 

    if(json_devCode)
        cJSON_AddStringToObject(newroot, "deviceCode", json_devCode->valuestring);


    cJSON_AddItemToObject(newroot, "data", dataObj);
    
    if(productionModel)
        cJSON_AddStringToObject(dataObj, "productionModel", productionModel->valuestring);

    if(id)
        cJSON_AddNumberToObject(dataObj, "id", id->valueint);

    if(version)
        cJSON_AddStringToObject(dataObj, "version", version->valuestring);

    if(softwareFirmware)
        cJSON_AddNumberToObject(dataObj, "softwareFirmware", softwareFirmware->valueint);
        
    if(versionType)
        cJSON_AddNumberToObject(dataObj, "versionType", versionType->valueint);  
            
    tmpBuf = cJSON_PrintUnformatted(newroot); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");

        cJSON_Delete(root);
        cJSON_Delete(newroot);      
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    }

    ef_set_env_blob("upData",(const char*)tmpBuf,strlen ((const char*)tmpBuf));

    cJSON_Delete(root);

    cJSON_Delete(newroot);

    my_free(tmpBuf);
    
    return result;    
}


SYSERRORCODE_E getTimePacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON *root;  
    char *tmpBuf;
    
    root = cJSON_CreateObject();
    if (!root)  
    {          
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        cJSON_Delete(root);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    } 
    
    cJSON_AddStringToObject(root,"commandCode","3013");
    cJSON_AddStringToObject(root,"deviceCode",gDevBaseParam.deviceCode.deviceSn);
     
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        my_free(tmpBuf);
        cJSON_Delete(root);        
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descBuf,tmpBuf);


    log_d("getTimePacket = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);

    return result;
}



uint8_t* packetBaseJson(uint8_t *jsonBuff,char status)
{
    static uint8_t value[256] = {0};
    
	cJSON* root,*newroot,*json_cmdCode,*json_ownerId;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        
        cJSON_Delete(root);        
        cJSON_Delete(newroot);
        my_free(tmpBuf);
		return NULL;
	}
	else
	{
        json_cmdCode = cJSON_GetObjectItem ( root, "commandCode" );
        json_ownerId = cJSON_GetObjectItem ( root, "ownerId" );

        

        newroot = cJSON_CreateObject();
   
        if(!newroot)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
    		return NULL;
        }

        if(json_cmdCode)
            cJSON_AddStringToObject(newroot, "commandCode", json_cmdCode->valuestring);     
            
        if(json_ownerId)
            cJSON_AddNumberToObject(newroot, "ownerId", json_ownerId->valueint);
            
        
        cJSON_AddStringToObject(newroot, "deviceCode",gDevBaseParam.deviceCode.deviceSn);
        
        if(status == 1)
            cJSON_AddStringToObject(newroot, "status", "1");
        else
            cJSON_AddStringToObject(newroot, "status", "0");
            

        
        tmpBuf = cJSON_PrintUnformatted(newroot); 

//        log_d("packetBaseJson = %s\r\n",tmpBuf);

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");

            cJSON_Delete(root);
            cJSON_Delete(newroot);      
            my_free(tmpBuf);
            return NULL;
        }    

        strcpy((char *)value,tmpBuf);

	}
	
    my_free(tmpBuf);
	cJSON_Delete(root);
    cJSON_Delete(newroot);
    
    return value;    
}



uint8_t** GetCardArray ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t *num)
{
    static uint8_t** result; 
    cJSON* root,*json_item;
    cJSON* arrayElement;
    int tmpArrayNum = 0;
    int i = 0;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
    
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        return NULL;
    }
    else
    {
        //根据协议，默认所有的子项是data
        json_item = cJSON_GetObjectItem ( root, "cardNo" );  
        
        if( json_item->type == cJSON_Array )
        {
            tmpArrayNum = cJSON_GetArraySize(json_item);
            
            log_d("cardArrayNum = %d\r\n",tmpArrayNum);
            
            //每个人最多20张卡
            if(tmpArrayNum > 20)
            {
                tmpArrayNum = 20;
            }

            result = (uint8_t **)my_malloc(tmpArrayNum*sizeof(uint8_t *));

            if(result == NULL)
            {
                 *num = 0;
                log_d("create array error\r\n");
                cJSON_Delete(root);
                return NULL;                
            }

            *num = tmpArrayNum;
            
            for (i = 0; i < tmpArrayNum; i++)
            {
                result[i] = (uint8_t *)my_malloc(8 * sizeof(uint8_t));
            }            

            for(i=0;i<tmpArrayNum;i++)
            {
                arrayElement = cJSON_GetArrayItem(json_item, i);                 
                strcpy ((char*)result[i], arrayElement->valuestring ); 
                log_d("result :%d = %s\r\n",i,result[i]); 
            }

        }
        else if( json_item->type == cJSON_String )
        {
            //一般走到这里，卡号就是空的
            if(strlen((const char*)json_item->valuestring) == 0)
            {
                 *num = 0;
                log_d("card no is empty \r\n");
                cJSON_Delete(root);
                return NULL;    
            }
        
            tmpArrayNum = 1;
            *num = tmpArrayNum;

            result[0] = (uint8_t *)my_malloc(8 * sizeof(uint8_t)); 
            
			if ( strlen ( json_item->valuestring ) > 8 )
			{
				memcpy ( result[0], json_item->valuestring,8 );
			}
			else
			{
			    strcpy ( (char*)result[0], json_item->valuestring ); 
			}

			log_d ( "json_item =  %s\r\n",json_item->valuestring );

            
            
        }
        else
        {
            *num = 0;
            log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
            return NULL;
        }
        
    }        
    
    cJSON_Delete(root);
    return result;
}


uint8_t parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo)
{   
    uint8_t isFind = 0;
    #if 0
    cJSON *root ,*tmpArray;
    uint8_t buf[300] = {0};
    
    if(!jsonBuff || !qrCodeInfo)
    {
        cJSON_Delete(root);
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }    
    
    root = cJSON_Parse((char *)jsonBuff);    //解析数据包
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 


    tmpArray = cJSON_GetObjectItem(root, "qS");
    strcpy((char *)qrCodeInfo->startTime,tmpArray->valuestring);
    log_d("qrCodeInfo->startTime= %s\r\n",qrCodeInfo->startTime); 
    
    tmpArray = cJSON_GetObjectItem(root, "qE");
    strcpy((char *)qrCodeInfo->endTime,tmpArray->valuestring);
    log_d("qrCodeInfo->endTime= %s\r\n",qrCodeInfo->endTime); 
    
    tmpArray = cJSON_GetObjectItem(root, "qI");
    sprintf((char*)qrCodeInfo->userId,"%d",tmpArray->valueint);
    log_d("qrCodeInfo->qrID= %s\r\n",qrCodeInfo->userId); 

    tmpArray = cJSON_GetObjectItem(root, "t");
    qrCodeInfo->platformType = tmpArray->valueint;
    log_d("qrCodeInfo->type= %d\r\n",qrCodeInfo->platformType); 
    
    tmpArray = cJSON_GetObjectItem(root, "f1");
    memcpy(qrCodeInfo->accessFloor,parseAccessFloor(tmpArray->valuestring),FLOOR_ARRAY_LENGTH);


    qrCodeInfo->defaultFloor = qrCodeInfo->accessFloor[0];
    log_d("qrCodeInfo->defaultFloor = %d\r\n",qrCodeInfo->defaultFloor);
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d1");
    strcpy((char *)buf,tmpArray->valuestring);  
    if(strlen((const char*)buf) > 0)
    {
        log_d("d1 = %s\r\n",buf); 
        if(findDev(buf,1) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }

    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d2");
    strcpy((char *)buf,tmpArray->valuestring);    
    if(strlen((const char*)buf) > 0)
    {
        log_d("d2 = %s\r\n",buf); 
        if(findDev(buf,2) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d3");
    strcpy((char *)buf,tmpArray->valuestring);    
    if(strlen((const char*)buf) > 0)
    {
        log_d("d3= %s\r\n",buf); 
        if(findDev(buf,3) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }

    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d4");
    strcpy((char *)buf,tmpArray->valuestring);
    if(strlen((const char*)buf) > 0)
    {
        log_d("d4 = %s\r\n",buf);    
        if(findDev(buf,4) == 1)
        {
            isFind =1;
            goto QR_END;
        }    
    }


    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d5");
    strcpy((char *)buf,tmpArray->valuestring);
    if(strlen((const char*)buf) > 0)
    {
        log_d("d5 = %s\r\n",buf);    
        if(findDev(buf,5) == 1)
        {
           isFind =1;
        }    
    }
  
QR_END:    
    cJSON_Delete(root);
    
    #endif
    
    return isFind;

}


uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson)
{ 
    
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*dataObj;
    char *tmpBuf;
    char tmpTime[32] = {0};

    root = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();

    if(!root && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
		return CJSON_CREATE_ERR;
    }

    cJSON_AddStringToObject(root, "deviceCode", gDevBaseParam.deviceCode.deviceSn);
    log_d("deviceCode = %s\r\n",gDevBaseParam.deviceCode.deviceSn);
    
    cJSON_AddItemToObject(root, "data", dataObj);

    if(localUserData->platformType == 4 )
    {    
        cJSON_AddStringToObject(root, "commandCode","3007");
        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
        cJSON_AddNumberToObject(dataObj, "callType",localUserData->authMode); 
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE); 
        strcpy(tmpTime,(const char*)bsp_ds1302_readtime());  
        cJSON_AddStringToObject(dataObj, "callElevatorTime",tmpTime);        

        if(localUserData->authMode == 7)
        {
            cJSON_AddNumberToObject(dataObj, "type",localUserData->platformType);
            cJSON_AddNumberToObject(dataObj, "callState",CALL_OK);
//            cJSON_AddStringToObject(dataObj, "qrId", (const char*)localUserData->qrID);
        }
        else
        {
            cJSON_AddNumberToObject(dataObj, "type",CALL_OK);            
        }
    }
    else
    {
        cJSON_AddStringToObject(root, "commandCode","4002");
        cJSON_AddNumberToObject(dataObj, "enterType",  4);//这里设置为4是QRCODE在开发者平台上的数据，梯控上的数据为7        
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);   
        cJSON_AddStringToObject(dataObj, "enterTime",(const char*)bsp_ds1302_readtime());                
        cJSON_AddNumberToObject(dataObj, "faceCompare",CALL_OK);//：1、成功 2失败，
        cJSON_AddNumberToObject(dataObj, "direction", DIRECTION_IN);//1进，2出        
        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
    }
    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(root);
        my_free(tmpBuf);            
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);    

    cJSON_Delete(root);
    my_free(tmpBuf);
    return result;

}

uint8_t packetCard(uint8_t *cardID,uint8_t *descJson)
{ 
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root;
    char *tmpBuf;
    char tmpCardID[9] = {0};

    root = cJSON_CreateObject();
    
    if(!root)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
		return CJSON_CREATE_ERR;
    }

    log_d("3 cardid %02x,%02x,%02x,%02x\r\n",cardID[0],cardID[1],cardID[2],cardID[3]);
    

    cJSON_AddStringToObject(root, "deviceCode", gDevBaseParam.deviceCode.deviceSn);
    log_d("deviceCode = %s\r\n",gDevBaseParam.deviceCode.deviceSn);    

    cJSON_AddStringToObject(root, "commandCode","1051");
    cJSON_AddNumberToObject(root, "status", ON_LINE);   
    cJSON_AddStringToObject(root, "currentTime",(const char*)bsp_ds1302_readtime());
    
    log_d("4 cardid %02x,%02x,%02x,%02x\r\n",cardID[0],cardID[1],cardID[2],cardID[3]);

    memset(tmpCardID,0x00,sizeof(tmpCardID));
    
    bcd2asc((uint8_t *)tmpCardID,cardID, 8, 1);

    log_d("CARD NO. = %s\r\n",tmpCardID);
    
    cJSON_AddStringToObject(root, "cardNo", (const char*)tmpCardID);

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(root);
        my_free(tmpBuf);            
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);    

    cJSON_Delete(root);
    my_free(tmpBuf);
    return result;

}






