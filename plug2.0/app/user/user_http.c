/*
 * user_http.c
 *
 *  Created on: 2018��11��11��
 *      Author: lenovo
 */

#include "user_common.h"
#include "esp_common.h"

const CHAR* HttpStatus_Ok                     = "OK";
const CHAR* HttpStatus_Created                = "Created";
const CHAR* HttpStatus_Found                  = "Found";
const CHAR* HttpStatus_BadRequest             = "BadRequest";
const CHAR* HttpStatus_NotFound               = "NotFound";
const CHAR* HttpStatus_InternalServerError    = "InternalServerError";

const CHAR* STRING_SPACE = " ";
const CHAR* STRING_ENTER = "\n";

HTTP_ROUTER_MAP_S stHttpRouterMap[HTTP_ROUTER_MAP_MAX];


const CHAR szHttpCodeMap[][5] =
{
	"101",
    "200",
    "201",
	"204",
    "302",
    "400",
    "404",
    "500",

    ""
};

const CHAR szHttpStatusMap[][20] =
{
	"Switching Protocols",
    "OK",
    "Created",
	"No Content",
    "Found",
    "BadRequest",
    "NotFound",
    "InternalServerError",

    ""
};

const CHAR szHttpMethodStr[][10] =
{
    "GET",
    "POST",
    "HEAD",
    "PUT",
    "DELETE",
    ""
};

const CHAR szHttpUserAgentStringmap[][10] =
{
    "Windows",
    "Android",
    "curl",
    "Wget",
    ""
};

const CHAR szHttpContentTypeStr[][25] =
{
    "text/html, charset=utf-8",
    "application/x-javascript",
    "text/css",
    "application/json",
    "image/x-icon",
    "image/png",
    "image/gif",

    "application/octet-stream",
    ""
};

const CHAR szHttpEncodingStr[][10] =
{
    "gzip",

    ""
};

const CHAR szHttpCacheControlStr[][20] =
{
    "no-cache",
    "max-age=3600",
    "max-age=86400",
    "max-age=604800",
    "max-age=2592000",
    "max-age=31536000",

    ""
};

const CHAR aGzipSuffix[HTTP_ENCODING_Buff+1][5] = {".gz",""};

UINT HTTP_RequestInit( HTTP_CTX *pstCtx );


INT32 HTTP_ParsingHttpHead( HTTP_CTX *pstCtx, CHAR * pcData, UINT32 uiLen )
{
    CHAR *pcCurPos = NULL;
    CHAR *pcTmpPos = NULL;
    CHAR *pcPos = NULL;
    INT32 iLoop = 0;
    int i = 0;

    if ( NULL == pcData || NULL == pstCtx)
    {
        LOG_OUT( LOGOUT_ERROR, "pcData:%p, pstCtx:%p", pcData, pstCtx);
        return FAIL;
    }

    //header�Ѿ�������ɣ����ڽ���body��
    if ( pstCtx->stReq.eProcess == RES_Process_GetBody ||
         pstCtx->stReq.eProcess == RES_Process_GotHeader )
    {
        //LOG_OUT(LOGOUT_DEBUG, "Getting Body...");
        pstCtx->stReq.uiRecvCurLen = uiLen;
        pstCtx->stReq.pcResqBody = pcData;
        pstCtx->stReq.uiRecvLen += uiLen;

        if ( pstCtx->stReq.eProtocol == HTTP_websocket )
        {
        	//LOG_OUT(LOGOUT_DEBUG, "Getting Body...");
        	return OK;
        }

        if ( pstCtx->stReq.eProcess == RES_Process_GotHeader )
        {
            pstCtx->stReq.eProcess = RES_Process_GetBody;
        }

        if ( pstCtx->stReq.uiRecvLen >= pstCtx->stReq.uiRecvTotalLen )
        {
            pstCtx->stReq.eProcess = RES_Process_Finished;
            //LOG_OUT(LOGOUT_DEBUG, "RES_Process_Finished.");
            return OK;
        }
        //LOG_OUT(LOGOUT_DEBUG, "RES_Process_Finished.");
        return OK;
    }
    else if ( pstCtx->stReq.eProcess != RES_Process_None )
    {
    	LOG_OUT(LOGOUT_ERROR, "eProcess = %d.", pstCtx->stReq.eProcess);
        HTTP_RequestInit( pstCtx );
        return FAIL;
    }

    pcData[uiLen] = '\r';
    pcData[uiLen+1] = '\n';
    pcData[uiLen+2] = 0;
    pcPos = pcCurPos = pcData;

    //LOG_OUT(LOGOUT_DEBUG, "\n%s\n", pcData);
    HTTP_RequestInit( pstCtx );

    /* ���н��� */
    while ( pcData != NULL )
    {
        //LOG_OUT(LOGOUT_DEBUG, "pcData��[%s]", pcData);
        pcCurPos = strsep(&pcData, STRING_ENTER);
        if ( NULL == pcCurPos )
        {
            //LOG_OUT(LOGOUT_DEBUG, "continue");
            continue;
        }
        //LOG_OUT(LOGOUT_DEBUG, "pcCurPos��[%s]", pcCurPos);

        //header����
        if ( strlen(pcCurPos) <= 1 && pstCtx->stReq.uiRecvTotalLen != 0 &&
           ( pstCtx->stReq.eMethod == HTTP_METHOD_POST ||
             pstCtx->stReq.eMethod == HTTP_METHOD_PUT ))
        {
            pcCurPos = pcCurPos + 2;
            pstCtx->stReq.pcResqBody = pcCurPos;
            pstCtx->stReq.uiRecvCurLen = uiLen - (pcCurPos - pcPos);
            pstCtx->stReq.uiRecvLen += pstCtx->stReq.uiRecvCurLen;

            if ( pstCtx->stReq.uiRecvCurLen == 0 )
            {
                //LOG_OUT(LOGOUT_DEBUG, "RES_Process_GotHeader.");
                pstCtx->stReq.eProcess = RES_Process_GotHeader;
                return OK;
            }

            if ( pstCtx->stReq.uiRecvLen >= pstCtx->stReq.uiRecvTotalLen )
            {
                //LOG_OUT(LOGOUT_DEBUG, "RES_Process_Finished.");
                pstCtx->stReq.eProcess = RES_Process_Finished;
                return OK;
            }
            pstCtx->stReq.eProcess = RES_Process_GetBody;
            //LOG_OUT(LOGOUT_DEBUG, "RES_Process_GetBody.");
            return OK;
        }

        /* GET / HTTP/1.1 : eMethod, URL*/
        if ( HTTP_METHOD_BUFF == pstCtx->stReq.eMethod )
        {
            //����eMethod
            //LOG_OUT(LOGOUT_INFO, "eMethod:[%s]", pcCurPos);
        	for ( ; isspace(*pcCurPos); pcCurPos++){}
        	pcTmpPos = strstr( pcCurPos, " ");
        	if ( pcTmpPos == 0 )
        	{
                LOG_OUT( LOGOUT_ERROR, "Parsing eMethod failed, [%s]", pcCurPos);
                return FAIL;
        	}
        	*pcTmpPos = 0;
        	strupr(pcCurPos);

            for ( iLoop = HTTP_METHOD_GET; iLoop < HTTP_METHOD_BUFF; iLoop++ )
            {
                if ( strstr( pcCurPos, szHttpMethodStr[iLoop]) )
                {
                    pstCtx->stReq.eMethod = iLoop;
                    //LOG_OUT(LOGOUT_DEBUG, "eMethod��[%d]", pstCtx->stReq.eMethod);
                    pcCurPos = pcTmpPos + 1;
                    break;
                }
            }

            /* ����URL */
            if ( pstCtx->stReq.eMethod != HTTP_METHOD_BUFF && NULL != (pcTmpPos = strstr( pcCurPos, "/" )))
            {
                for ( ; !isspace(*pcTmpPos); pcTmpPos++){}
                *pcTmpPos = 0;
                //LOG_OUT(LOGOUT_DEBUG, "pcTmpPos:[%s]", strstr( pcCurPos, "/" ));
                strncpy( pstCtx->stReq.szURL, strstr( pcCurPos, "/" ), sizeof(pstCtx->stReq.szURL));
            }

            if ( pstCtx->stReq.eMethod == HTTP_METHOD_BUFF || pstCtx->stReq.szURL[0] == 0 )
            {
                LOG_OUT( LOGOUT_ERROR, "get eMethod or URL failed.");
                HTTP_RequestInit( pstCtx );
                return FAIL;
            }
        }
        //curl����ȹ��� ���͵��ֶ����ݳ����ֶ��Ǵ�д��ĸ��ͷ"Content-Length", postman ���͵��ֶ����ݳ����ֶ���Сд��ĸ"content-length"
        else if ( NULL != (pcTmpPos = strstr( pcCurPos, "ontent-Length: " )))
        {
            pcTmpPos += 15;
            //LOG_OUT(LOGOUT_DEBUG, "Content-Length:[%s]", pcTmpPos);
            pstCtx->stReq.uiRecvTotalLen = atoi(pcTmpPos);
        }
        else if ( NULL != (pcTmpPos = strstr( pcCurPos, ":" )))
        {
            // �����հ��ַ�
        	for ( ; isspace(*pcCurPos); pcCurPos++){}
            *pcTmpPos = 0;
            pcTmpPos++;
            // �����հ��ַ�
            for ( ; isspace(*pcTmpPos); pcTmpPos++ ){}
            HTTP_SetReqHeader( pstCtx, pcCurPos, pcTmpPos );
            for ( ; !isspace(*pcTmpPos); pcTmpPos++ ){}
            *pcTmpPos = 0;
        }
    }
    //LOG_OUT(LOGOUT_DEBUG, "RES_Process_GotHeader.");

    pstCtx->stReq.eProcess = RES_Process_GotHeader;
    return OK;
}

UINT HTTP_SetReqHeader( HTTP_CTX *pstCtx, CHAR* pcKey, CHAR*pcValue )
{
    UINT i = 0;

    if ( pstCtx == NULL ||  pcKey == NULL || pcValue == NULL)
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx, pcKey or pcValue is NULL, pstCtx:%p pcKey:%p pcValue:%p",
                pstCtx, pcKey, pcValue);
        return FAIL;
    }

    for ( i = 0; i < sizeof(pstCtx->stReq.stHeader)/sizeof(pstCtx->stReq.stHeader[0]); i++ )
    {
        if ( pstCtx->stReq.stHeader[i].pcKey == NULL )
        {
            pstCtx->stReq.stHeader[i].pcKey = pcKey;
            pstCtx->stReq.stHeader[i].pcValue = pcValue;

            //LOG_OUT(LOGOUT_DEBUG, "pcKey:%s, pcValue:%s", pcKey, pcValue);
            return OK;
        }
    }

    LOG_OUT(LOGOUT_ERROR, "stHeader num is full %d, pcKey:%s pcValue:%s",
            sizeof(pstCtx->stReq.stHeader)/sizeof(pstCtx->stReq.stHeader[0]), pcKey, pcValue);
    return FAIL;
}

CHAR* HTTP_GetReqHeader( HTTP_CTX *pstCtx, const CHAR* pcKey )
{
    UINT i = 0;

    if ( pstCtx == NULL ||  pcKey == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx, pcKey is NULL, pstCtx:%p pcKey:%p",pstCtx, pcKey);
        return NULL;
    }

    for ( i = 0; i < sizeof(pstCtx->stReq.stHeader)/sizeof(pstCtx->stReq.stHeader[0]); i++ )
    {
        if ( pstCtx->stReq.stHeader[i].pcKey == NULL )
		{
        	break;
		}

        if ( strcmp(pstCtx->stReq.stHeader[i].pcKey, pcKey) == 0 )
        {
            return pstCtx->stReq.stHeader[i].pcValue;
        }
    }

    return NULL;
}

VOID HTTP_RouterMapInit( VOID )
{
    UINT uiLoop = 0;

    for ( uiLoop = 0; uiLoop < HTTP_ROUTER_MAP_MAX; uiLoop++ )
    {
        stHttpRouterMap[uiLoop].eMethod = HTTP_METHOD_BUFF;
        stHttpRouterMap[uiLoop].pfHttpHandler = NULL;
        memset(stHttpRouterMap[uiLoop].szURL, 0, HTTP_URL_MAX_LEN);
    }
}

UINT HTTP_RouterRegiste( UINT uiMethod, CHAR* pcUrl, VOID* pfFunc, CHAR* pcFunStr)
{
    UINT uiLoop = 0;

    if ( uiMethod >= HTTP_METHOD_BUFF )
    {
        LOG_OUT(LOGOUT_ERROR, "HTTP_RouterRegiste uiMethod:%d", uiMethod);
        return FAIL;
    }

    if ( pcUrl == NULL ||  pfFunc == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "HTTP_RouterRegiste pcUrl or pfFunc is NULL, pcUrl:%d,pfFunc:%d", pcUrl, pfFunc);
        return FAIL;
    }

    //�ж��Ƿ�ע�������ע������и���
    for ( uiLoop = 0; uiLoop < HTTP_ROUTER_MAP_MAX; uiLoop++ )
    {
        if ( stHttpRouterMap[uiLoop].eMethod == uiMethod &&
             strcmp(stHttpRouterMap[uiLoop].szURL, pcUrl ) == 0 )
        {
            stHttpRouterMap[uiLoop].pfHttpHandler = pfFunc;
            return OK;
        }
    }

    for ( uiLoop = 0; uiLoop < HTTP_ROUTER_MAP_MAX; uiLoop++ )
    {
        if ( stHttpRouterMap[uiLoop].eMethod == HTTP_METHOD_BUFF )
        {
            stHttpRouterMap[uiLoop].eMethod = uiMethod;
            stHttpRouterMap[uiLoop].pfHttpHandler = pfFunc;
            strncpy(stHttpRouterMap[uiLoop].szURL, pcUrl, HTTP_URL_MAX_LEN);
            break;
        }
    }

    if ( uiLoop >= HTTP_ROUTER_MAP_MAX )
    {
        LOG_OUT(LOGOUT_ERROR, "HTTP_RouterRegiste stHttpRouterMap is full, num:%d", HTTP_ROUTER_MAP_MAX);
        return FAIL;
    }

    return OK;
}


VOID HTTP_RouterInit( VOID )
{
    HTTP_RouterMapInit();

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/",                 HTTP_GetHome,       "HTTP_GetHome");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/health",           HTTP_GetHealth,     "HTTP_GetHealth");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/info",             HTTP_GetInfo,       "HTTP_GetInfo");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/timer/:timer",     HTTP_GetTimerData,  "HTTP_GetTimerData");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/delay/:delay",     HTTP_GetDelayData,  "HTTP_GetDelayData");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/infrared/:infrared",
                                                              HTTP_GetInfraredData,"HTTP_GetInfraredData");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/infrared/:infrared/switch/:switch",
                                                              HTTP_GetInfraredValue,"HTTP_GetInfraredValue");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/system",           HTTP_GetSystemData,   "HTTP_GetSystemData");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/cloudplatform",    HTTP_GetCloudPlatformData, "HTTP_GetCloudPlatformData");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/temperature",      HTTP_GetTemperature, "HTTP_GetTemperature");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/html/header",      HTTP_GetHtmlHeader,  "HTTP_GetHtmlHeader");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/html/header",      HTTP_PostHtmlHeader, "HTTP_PostHtmlHeader");
    HTTP_RouterRegiste(HTTP_METHOD_PUT,  "/html/:html",       HTTP_PutHtml,        "HTTP_PutHtml");

    HTTP_RouterRegiste(HTTP_METHOD_POST, "/timer",            HTTP_PostTimerData,  "HTTP_PostTimerData");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/delay",            HTTP_PostDelayData,  "HTTP_PostDelayData");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/infrared",         HTTP_PostInfraredData, "HTTP_PostInfraredData");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/system",           HTTP_PostSystemData, "HTTP_PostSystemData");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/cloudplatform",    HTTP_PostCloudPlatformData, "HTTP_PostCloudPlatformData");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/webset",           HTTP_PostWebSet,      "HTTP_PostWebSet");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/webset",           HTTP_GetWebSet,       "HTTP_GetWebSet");

    HTTP_RouterRegiste(HTTP_METHOD_POST, "/control",          HTTP_PostDeviceControl, "HTTP_PostDeviceControl");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/scanwifi",         HTTP_GetScanWifi,     "HTTP_GetScanWifi");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/relaystatus",      HTTP_GetRelayStatus,  "HTTP_GetRelayStatus");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/relaystatus",      HTTP_PostRelayStatus, "HTTP_PostRelayStatus");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/date",             HTTP_GetDate,         "HTTP_GetDate");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/date",             HTTP_PostDate,        "HTTP_PostDate");

    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/meter",            HTTP_GetMeter,        "HTTP_GetMeter");
    HTTP_RouterRegiste(HTTP_METHOD_POST, "/meter",            HTTP_PostMeter,       "HTTP_PostMeter");

    HTTP_RouterRegiste(HTTP_METHOD_PUT,  "/upgrade",          HTTP_PutUpgrade,      "HTTP_PutUpgrade");
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/upload",           HTTP_GetUploadHtml,   "HTTP_GetUploadHtml");

    // websocket
    HTTP_RouterRegiste(HTTP_METHOD_GET,  "/console",          HTTP_GetConsole,     "HTTP_GetConsole");

    HTTP_FileListRegiste();
}

BOOL HTTP_RouterIsMatch( const CHAR* pcRouter, const CHAR* pcUrl)
{
    CHAR pcRBuf[HTTP_URL_MAX_LEN] = { 0 };
    CHAR pcUBuf[HTTP_URL_MAX_LEN] = { 0 };
    CHAR *pcRData, *pcUData = NULL;
    CHAR *pcRtmp, *pcUtmp = NULL;

    if (pcRouter == NULL || pcUrl == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pcRouter:%p, pcUrl:%p", pcRouter, pcUrl);
        return FALSE;
    }

    if ( strcmp(pcRouter, pcUrl) == 0 )
    {
        return TRUE;
    }

    strncpy(pcRBuf, pcRouter,    HTTP_URL_MAX_LEN);
    strncpy(pcUBuf, pcUrl,       HTTP_URL_MAX_LEN);

    pcRData = pcRBuf;
    pcUData = pcUBuf;

    while( pcRData != NULL && pcUData != NULL)
    {
        pcRtmp = strsep(&pcRData, "/");
        pcUtmp = strsep(&pcUData, "/");

        if (pcRtmp == NULL || pcUtmp == NULL )
        {
            return FALSE;
        }

        if ( NULL != strstr(pcRtmp, ":") )
        {
            continue;
        }

        // URL�а���ͨ���*���ж��߼�
        if ( strstr(pcRtmp, "*") )
        {
            while ( *pcRtmp )
            {
            	// ͨ���������������ж�
            	if ( *pcRtmp == '*' )
            	{
            		return TRUE;
            	}

            	if ( *pcRtmp != *pcUtmp )
            	{
            		return FALSE;
            	}

            	pcRtmp++;
    			pcUtmp++;
            }
        }
        else
        {
        	// û��ͨ�����Ҫ����ƥ��
			if ( strcmp(pcRtmp, pcUtmp) != 0 )
			{
				return FALSE;
			}
        }
    }

    if (pcRData == NULL && pcUData == NULL )
    {
        return TRUE;
    }

    return FALSE;
}

UINT HTTP_GetRouterPara( HTTP_CTX *pstCtx, const CHAR* pcKey, CHAR* pcValue )
{
    CHAR pcRBuf[HTTP_URL_MAX_LEN];
    CHAR pcUBuf[HTTP_URL_MAX_LEN];
    CHAR *pcRData, *pcUData = NULL;
    CHAR *pcRtmp, *pcUtmp = NULL;

    if ( pstCtx == NULL || pcKey == NULL || pcValue == NULL ||
         pstCtx->stReq.pcRouter == NULL || pstCtx->stReq.szURL == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx:%p, pcKey:%p, pcValue:%p, pcRouter:%p, szURL:%p",
                pstCtx, pcKey, pcValue, pstCtx->stReq.pcRouter, pstCtx->stReq.szURL);
        return FAIL;
    }

    strncpy(pcRBuf, pstCtx->stReq.pcRouter,    HTTP_URL_MAX_LEN);
    strncpy(pcUBuf, pstCtx->stReq.szURL,     HTTP_URL_MAX_LEN);

    pcRData = pcRBuf;
    pcUData = pcUBuf;

    while( pcRData != NULL && pcUData != NULL)
    {
        pcRtmp = strsep(&pcRData, "/");
        pcUtmp = strsep(&pcUData, "/");

        if ( NULL != strstr(pcRtmp, ":") )
        {
            if ( strcmp(pcRtmp+1, pcKey) == 0 )
            {
                strcpy(pcValue, pcUtmp);
                return OK;
            }
        }
    }
    return FAIL;
}

UINT HTTP_RequestInit( HTTP_CTX *pstCtx )
{
    if ( pstCtx == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx:%p", pstCtx);
        return FAIL;
    }

    memset(&pstCtx->stReq, 0, sizeof(HTTP_REQ_S));
    pstCtx->stReq.eMethod  = HTTP_METHOD_BUFF;

    return OK;
}

UINT HTTP_ResponInit( HTTP_CTX *pstCtx )
{
    if ( pstCtx == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx:%p", pstCtx);
        return FAIL;
    }

    FREE_MEM(pstCtx->stResp.pcResponBody);
    memset(&pstCtx->stResp, 0, sizeof(pstCtx->stResp));
    pstCtx->stResp.eHttpCode = HTTP_CODE_Buff;

    return OK;
}

UINT HTTP_RouterHandle( HTTP_CTX *pstCtx )
{
	UINT uiRet = 0;
    UINT uiLoop = 0;
    CHAR *pcValue = NULL;
    UINT i = 0;

    if ( NULL == pstCtx )
    {
        LOG_OUT( LOGOUT_ERROR, "pstCtx:%p", pstCtx);
        uiRet = FAIL;
        goto end;
    }

    if ( pstCtx->stReq.eProcess == RES_Process_None ||
         pstCtx->stReq.eProcess == RES_Process_Invalid )
    {
        LOG_OUT(LOGOUT_ERROR, "eProcess:%d", pstCtx->stReq.eProcess);
        uiRet = FAIL;
        goto end;
    }

    if ( (pstCtx->stReq.eMethod == HTTP_METHOD_POST ||
          pstCtx->stReq.eMethod == HTTP_METHOD_PUT ) &&
          pstCtx->stReq.eProcess == RES_Process_GotHeader )
    {
        uiRet = OK;
        goto end;
    }

    if ( pstCtx->stReq.szURL[0] == 0 )
    {
        LOG_OUT(LOGOUT_INFO, "szURL is NULL");
        uiRet = FAIL;
        goto end;
    }

    if ( pstCtx->stReq.pfHandler != NULL )
    {
    	//LOG_OUT( LOGOUT_DEBUG, "handle...");
    	uiRet = pstCtx->stReq.pfHandler(pstCtx);
        goto end;
    }

    // websocket Э������⴦��
	CHAR* pcHeader = NULL;
    pcHeader = HTTP_GetReqHeader(pstCtx, "Upgrade");
    if ( pcHeader != NULL && strcmp(pcHeader, "websocket") == 0 )
    {
    	//3600s�����ݽ�����Ͽ�����
    	pstCtx->uiTimeOut = 3600;
    	pstCtx->stReq.eProtocol = HTTP_websocket;
    }
    else
    {
    	pstCtx->stReq.eProtocol = HTTP_1_1;
    }

    HTTP_ResponInit(pstCtx);

    pcValue = HTTP_GetReqHeader(pstCtx, "Host");

    if ( pcValue != NULL &&
    	 sscanf(pcValue,"%d.%d.%d.%d",&i,&i,&i,&i) != 4 &&
		 strstr(pstCtx->stReq.szURL, "favicon") == 0 &&
    	 strstr(pstCtx->stReq.szURL, "system") == 0 &&
		 strstr(pstCtx->stReq.szURL, "upload") == 0 &&
		 strstr(pstCtx->stReq.szURL, "html/") == 0 )
    {
    	//HTTP_GetredirectHtml(pstCtx);
    	LOG_OUT(LOGOUT_INFO, "[Request] Method:%s URL:%s -> /redirect.html",
    			szHttpMethodStr[pstCtx->stReq.eMethod],
				pstCtx->stReq.szURL);

    	strcpy(pstCtx->stReq.szURL, "/redirect.html");
    	HTTP_GetRedirectHtml(pstCtx);
    	goto end;
    }

    for ( uiLoop = 0; uiLoop < HTTP_ROUTER_MAP_MAX; uiLoop++ )
    {
        if ( stHttpRouterMap[uiLoop].eMethod == pstCtx->stReq.eMethod &&
             HTTP_RouterIsMatch(stHttpRouterMap[uiLoop].szURL, pstCtx->stReq.szURL) )
        {
            if ( pstCtx->stReq.uiRecvCurLen == pstCtx->stReq.uiRecvLen )
            {
                LOG_OUT(LOGOUT_INFO, "[Request] Method:%s URL:%s", szHttpMethodStr[pstCtx->stReq.eMethod], pstCtx->stReq.szURL);
            }
            pstCtx->stReq.pcRouter = stHttpRouterMap[uiLoop].szURL;
            pstCtx->stReq.pfHandler = stHttpRouterMap[uiLoop].pfHttpHandler;

            uiRet = stHttpRouterMap[uiLoop].pfHttpHandler(pstCtx);
            goto end;
        }
    }

    if ( uiLoop >= HTTP_ROUTER_MAP_MAX )
    {
        LOG_OUT(LOGOUT_INFO, "[Request] Method:%s URL:%s", szHttpMethodStr[pstCtx->stReq.eMethod], pstCtx->stReq.szURL);

        uiRet = HTTP_NotFound( pstCtx );
        goto end;
    }

end:
    if ( HTTP_IS_SEND_FINISH( pstCtx ) )
    {
    	pstCtx->uiTimeOut = WEB_CONTINUE_TMOUT;
        LOG_OUT(LOGOUT_INFO, "[Response] Method:%s URL:%s Code:%s",
                szHttpMethodStr[pstCtx->stReq.eMethod],
                pstCtx->stReq.szURL,
                szHttpCodeMap[pstCtx->stResp.eHttpCode]);
        HTTP_RequestInit( pstCtx );
        HTTP_ResponInit( pstCtx );
    }

    return uiRet;
}

UINT HTTP_SetRespHeader( HTTP_CTX *pstCtx )
{
    if ( pstCtx->stResp.eHttpCode < HTTP_CODE_Buff )
    {
        pstCtx->stResp.uiPos += snprintf(
                pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
                pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
                "HTTP/1.1 %s %s \r\n",
                szHttpCodeMap[pstCtx->stResp.eHttpCode],
                szHttpStatusMap[pstCtx->stResp.eHttpCode] );
    }

    // websocketЭ��
    if ( pstCtx->stReq.eProtocol == HTTP_websocket )
    {
		pstCtx->stResp.uiPos += snprintf(
				pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
				pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
				"Upgrade: websocket \r\n");

		pstCtx->stResp.uiPos += snprintf(
				pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
				pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
				"Connection: Upgrade \r\n\r\n");
    }
    // httpЭ��
    else
    {
		if ( pstCtx->stResp.eContentType < HTTP_CONTENT_TYPE_Buff )
		{
			pstCtx->stResp.uiPos += snprintf(
					pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
					pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
					"Content-Type: %s \r\n", szHttpContentTypeStr[pstCtx->stResp.eContentType]);
		}

		if (pstCtx->stResp.eCacheControl >= HTTP_CACHE_CTL_TYPE_Buff )
		{
			pstCtx->stResp.uiPos += snprintf(
					pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
					pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
					"Cache-Control: %s \r\n", szHttpCacheControlStr[pstCtx->stResp.eCacheControl]);
		}

		pstCtx->stResp.uiPos += snprintf(
				pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
				pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
				"Accept-Ranges: bytes \r\n");

		pstCtx->stResp.uiPos += snprintf(
				pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
				pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
				"Connection: Keep-Alive \r\n");

		pstCtx->stResp.uiPos += snprintf(
				pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
				pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
					"Content-Length:           \r\n\r\n");
    }
    pstCtx->stResp.uiHeaderLen = pstCtx->stResp.uiPos;
    return OK;
}

UINT HTTP_SetCustomHeader( HTTP_CTX *pstCtx, CHAR* pcKey, CHAR *pcValues, ... )
{
    va_list Arg;

    if ( pstCtx == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx is NULL.");
        return FAIL;
    }

    //�����header�����body��bodyһ�����Ͳ���������header��
    if ( pstCtx->stResp.uiBodyLen != 0 )
    {
        LOG_OUT(LOGOUT_ERROR, "response body has be set, Cannot set herder.");
        return FAIL;
    }

    pstCtx->stResp.uiPos -= 2;

    pstCtx->stResp.uiPos += snprintf(
            pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
            pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
            "%s: ", pcKey);

    va_start(Arg, pcValues);
    pstCtx->stResp.uiPos += vsnprintf(
            pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
            pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
            pcValues, Arg);
    va_end(Arg);

    pstCtx->stResp.uiPos += snprintf(
            pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
            pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
            "\r\n\r\n");

    pstCtx->stResp.uiHeaderLen = pstCtx->stResp.uiPos;

    return OK;
}


UINT HTTP_SetBodyLen( HTTP_CTX *pstCtx, UINT uiBodyLen )
{
    UINT uiLen = 0;
    CHAR* pcPos = NULL;

    if ( pstCtx == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx is NULL.");
        return FAIL;
    }

    pcPos = strstr(pstCtx->stResp.pcResponBody, "Content-Length: ");
    if ( pcPos != NULL )
    {
        uiLen = sprintf( pcPos, "Content-Length: %d", uiBodyLen );
        *(pcPos + uiLen) = ' ';
    }

    return OK;
}

UINT HTTP_SendOnce( HTTP_CTX *pstCtx )
{
    UINT uiRet = 0;
    UINT uiLen = 0;
    CHAR* pcLength = NULL;

    if ( pstCtx == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx is NULL.");
        return FAIL;
    }

    if( pstCtx->stResp.uiBodyLen != pstCtx->stResp.uiPos - pstCtx->stResp.uiHeaderLen )
    {
        pstCtx->stResp.uiBodyLen = pstCtx->stResp.uiPos - pstCtx->stResp.uiHeaderLen;
    }

    pcLength = strstr(pstCtx->stResp.pcResponBody, "Content-Length: ");
    if ( pcLength != NULL )
    {
        uiLen = sprintf( pcLength, "Content-Length: %d", pstCtx->stResp.uiBodyLen );
        *(pcLength + uiLen) = ' ';
    }

    pstCtx->stResp.uiSendCurLen   = pstCtx->stResp.uiPos;
    pstCtx->stResp.uiSendTotalLen = pstCtx->stResp.uiSendCurLen;

    //LOG_OUT(LOGOUT_DEBUG, "[%s]", pstCtx->stResp.pcResponBody);

    uiRet = WEB_WebSend(pstCtx);
    if ( uiRet != OK )
    {
        LOG_OUT(LOGOUT_ERROR, "send failed");
        return FAIL;
    }

    return OK;
}



UINT HTTP_SetResponseBody( HTTP_CTX *pstCtx, CHAR* pcBody )
{
    UINT uiLen = 0;
    CHAR* pcLength = NULL;

    if ( pstCtx == NULL || pcBody == NULL )
    {
        LOG_OUT(LOGOUT_ERROR, "pstCtx:%p, pcBody:%p", pstCtx, pcBody);
        return FAIL;
    }

    pstCtx->stResp.uiBodyLen = snprintf(
            pstCtx->stResp.pcResponBody + pstCtx->stResp.uiPos,
            pstCtx->stResp.uiSendBufLen - pstCtx->stResp.uiPos,
            pcBody);

    pstCtx->stResp.uiPos += pstCtx->stResp.uiBodyLen;

    return OK;
}



