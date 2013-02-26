#ifndef HTTP_CONF_H_
#define HTTP_CONF_H_

#include "utils/lwiplib.h"
#include "httpserver_raw/httpd.h"

#define SSI_INDEX_STATE    0
#define SSI_INDEX_MACADDR  1
#define SSI_INDEX_CHANNEL  2
#define SSI_INDEX_SENSITIVITY  3
#define SSI_INDEX_PANFILTER  4
#define SSI_INDEX_CRCFILTER  5
#define SSI_INDEX_IPADDR   6
#define SSI_INDEX_NMASK    7
#define SSI_INDEX_GW       8
#define SSI_INDEX_REIPPADDR 9
#define SSI_INDEX_REPORT 10

#define SSI_INDEX_CHN 11
#define SSI_INDEX_DHCP 12

#define SSI_INDEX_REDIR 13

#define SSI_INDEX_P_THR 14

#define SSI_INDEX_P_DROP 15

#define SSI_INDEX_CHN_LOW 16
#define SSI_INDEX_CHN_HIGH 42

static const char *g_pcConfigSSITags[] = 
{
"state", 
"macaddr",
"channel",
"sens",
"panfilt",
"crcfilt",
"ipaddr",
"nmask",
"gw",
"reipaddr",
"report",
"chn",
"dhcp",
"redirect",
"packthr",
"packdrop",
"edch0","edch1","edch2","edch3","edch4","edch5","edch6","edch7","edch8","edch9",
"edch10","edch11","edch12","edch13","edch14","edch15","edch16","edch17","edch18","edch19",
"edch20","edch21","edch22","edch23","edch24","edch25","edch26"
};

void HttpdInit(tDeviceSettings *ptr);

int SSIHandler(int iIndex, char *pcInsert, int iInsertLen);

char *StatusCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *SettingsCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *IpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
char *RemoteIpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);

static const tCGI g_psConfigCGIURIs[] =
{
    { "/status.cgi", StatusCGIHandler },      
    { "/settings.cgi", SettingsCGIHandler },
    { "/ipset.cgi", IpsetCGIHandler },         
    
};

#define NUM_CONFIG_SSI_TAGS     (sizeof(g_pcConfigSSITags) / sizeof (char *))
#define NUM_CONFIG_CGI_URIS     (sizeof(g_psConfigCGIURIs) / sizeof(tCGI))
#define DEFAULT_CGI_RESPONSE    "/index.shtml"
#define PARAM_ERROR_RESPONSE    "/perr.htm"
#define REDIR_CGI_RESPONSE      "/redir.shtml"



#endif /*HTTP_CONF_H_*/
