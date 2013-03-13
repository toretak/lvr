#ifndef HTTP_CONF_H_
#define HTTP_CONF_H_

#include "utils/lwiplib.h"
#include "httpd.h"

#define SSI_INDEX_STATE    0
#define SSI_INDEX_MACADDR  1
#define SSI_INDEX_IPADDR   2
#define SSI_INDEX_PORT  3
#define SSI_INDEX_NMASK    4
#define SSI_INDEX_GW       5
#define SSI_INDEX_REIPPADDR 6
#define SSI_INDEX_REPORT 7

#define SSI_INDEX_CHN 8
#define SSI_INDEX_DHCP 9

#define SSI_INDEX_REDIR 10
#define SSI_INDEX_MAC_LIST 11
#define SSI_INDEX_SETT 12

#define SSI_INDEX_CHN_STATE_LOW 13
#define SSI_INDEX_CHN_STATE_HIGH 20
#define SSI_INDEX_ERRMSG        21
#define SSI_INDEX_MCU_FW_VERSION 22
#define SSI_INDEX_FPGA_FW_VERSION 23
#define SSI_INDEX_DATE 24
#define SSI_INDEX_ID 25
#define SSI_INDEX_SN 26

static const char *g_pcConfigSSITags[] = 
{
"state", 
"macaddr",
"ipaddr",
"port",
"nmask",
"gw",
"reipaddr",
"report",
"chn",
"dhcp",
"redirect",
"maclist",
"sett",
"ch1state",
"ch2state",
"ch3state",
"ch4state",
"ch5state",
"ch6state",
"ch7state",
"ch8state",
"errmsg",
"mcufwv",
"fpgafwv",
"date",
"id",
"sn"
};

void HttpdInit(tDeviceSettings *ptr);

int SSIHandler(int iIndex, char *pcInsert, int iInsertLen);

char *StatusCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *SettingsCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *IpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *ProtocosetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *RemoteIpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *MacAddCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *ToggleMacFilterCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *ClearMacCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);

static const tCGI g_psConfigCGIURIs[] =
{
    { "/clear.cgi", StatusCGIHandler },      
    { "/channelsettings.cgi", SettingsCGIHandler },
    { "/ipset.cgi", IpsetCGIHandler },         
    { "/prset.cgi", ProtocosetCGIHandler },         
    { "/mac.cgi", MacAddCGIHandler },              
    { "/mac_filter.cgi", ToggleMacFilterCGIHandler },         
    { "/clearmac.cgi", ClearMacCGIHandler },         
    
};

#define NUM_CONFIG_SSI_TAGS     (sizeof(g_pcConfigSSITags) / sizeof (char *))
#define NUM_CONFIG_CGI_URIS     (sizeof(g_psConfigCGIURIs) / sizeof(tCGI))
#define DEFAULT_CGI_RESPONSE    "/index.shtml"
#define SETTINGS_CGI_RESPONSE    "/settings.shtml"
#define PARAM_ERROR_RESPONSE    "/prerror.htm"
#define REDIR_CGI_RESPONSE      "/redir.shtml"
#define STATUS_CGI_RESPONSE      "/status.shtml"



#endif /*HTTP_CONF_H_*/
