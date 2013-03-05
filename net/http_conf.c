//*****************************************************************************
//
//! @file
//! @brief HTTP configure
//
//*****************************************************************************

// STELARISWARE
#include "string.h"
#include "utils/lwiplib.h"
#include "httpd.h"
#include "utils/ustdlib.h"
#include "lwip/inet.h"

// PROJECT INCLUDES
#include "../myutils/uartstdio.h"
#include "../device.h"
#include "http_conf.h"
#include "cgifuncs.h"
//******************************************************************************

tDeviceSettings *ptrDeviceSettings;

//******************************************************************************

void HttpdInit(tDeviceSettings *ptr) {
    ptrDeviceSettings = ptr;
    http_set_ssi_handler(SSIHandler, g_pcConfigSSITags,NUM_CONFIG_SSI_TAGS);
    http_set_cgi_handlers(g_psConfigCGIURIs, NUM_CONFIG_CGI_URIS);
    httpd_init();
}

//******************************************************************************

void DisplayIPAddress(unsigned long ipaddr, unsigned long ulCol, unsigned long ulRow)
{
    char pucBuf[16];
    unsigned char *pucTemp = (unsigned char *)&ipaddr;

    //
    // Convert the IP Address into a string.
    //
    usprintf(pucBuf, "%d.%d.%d.%d", pucTemp[0], pucTemp[1], pucTemp[2], pucTemp[3]);
}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void lwIPHostTimerHandler(void)
{
    static unsigned long ulLastIPAddress = 0;
    unsigned long ulIPAddress;

    ulIPAddress = lwIPLocalIPAddrGet();

	//
    // Check if IP address has changed, and display if it has.
    //
    if(ulLastIPAddress != ulIPAddress)
    {
        ulLastIPAddress = ulIPAddress;
        DisplayIPAddress(ulIPAddress, 36, 16);
        ulIPAddress = lwIPLocalNetMaskGet();
        DisplayIPAddress(ulIPAddress, 36, 24);
        ulIPAddress = lwIPLocalGWAddrGet();
        DisplayIPAddress(ulIPAddress, 36, 32);
    }
}

//*****************************************************************************
//
// SSIHandler
//
//*****************************************************************************
int SSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{
    if(iIndex >= SSI_INDEX_CHN_STATE_LOW && iIndex <= SSI_INDEX_CHN_STATE_HIGH) {
        //int chIndex = iIndex - SSI_INDEX_CHN_STATE_LOW;
        //TODO read channel status
        //if((ptrDeviceSettings->channelSettings[chIndex]).controlReg)
        
        usnprintf(pcInsert, iInsertLen, "OK");
        
        
    }else{
        switch(iIndex)
        {

            case SSI_INDEX_REDIR:
                usnprintf(pcInsert, iInsertLen,  "%d.%d.%d.%d", ptrDeviceSettings->ipaddr[0],
                          ptrDeviceSettings->ipaddr[1], 
                          ptrDeviceSettings->ipaddr[2], 
                          ptrDeviceSettings->ipaddr[3]);  


                ptrDeviceSettings->setIpConfig=1;
            break;

            case SSI_INDEX_IPADDR:
                    usnprintf(pcInsert, iInsertLen,  "%d.%d.%d.%d", ptrDeviceSettings->ipaddr[0],
                                                                                                  ptrDeviceSettings->ipaddr[1], 
                                                                                                  ptrDeviceSettings->ipaddr[2], 
                                                                                                  ptrDeviceSettings->ipaddr[3]);
            break;

            case SSI_INDEX_NMASK:
                    usnprintf(pcInsert, iInsertLen,  "%d.%d.%d.%d", ptrDeviceSettings->nmask[0],
                                                                                                  ptrDeviceSettings->nmask[1], 
                                                                                                  ptrDeviceSettings->nmask[2], 
                                                                                                  ptrDeviceSettings->nmask[3]);
            break;

            case SSI_INDEX_GW:
                    usnprintf(pcInsert, iInsertLen,  "%d.%d.%d.%d", ptrDeviceSettings->gw[0],
                                                                                                  ptrDeviceSettings->gw[1], 
                                                                                                  ptrDeviceSettings->gw[2], 
                                                                                                  ptrDeviceSettings->gw[3]);
            break;

            case SSI_INDEX_MACADDR:
                    usnprintf(pcInsert, iInsertLen,  "%02X-%02X-%02X-%02X-%02X-%02X", 
                    ptrDeviceSettings->macaddr[0],
                    ptrDeviceSettings->macaddr[1],
                    ptrDeviceSettings->macaddr[2],
                    ptrDeviceSettings->macaddr[3],
                    ptrDeviceSettings->macaddr[4],
                    ptrDeviceSettings->macaddr[5]);
            break;



            case SSI_INDEX_DHCP:
                    usnprintf(pcInsert, iInsertLen, (ptrDeviceSettings->dhcpOn) ? "0" : "1");
            break;

            case SSI_INDEX_REIPPADDR:
                    usnprintf(pcInsert, iInsertLen,  "%d.%d.%d.%d",
                              ptrDeviceSettings->reipaddr[0],
                              ptrDeviceSettings->reipaddr[1], 
                              ptrDeviceSettings->reipaddr[2], 
                              ptrDeviceSettings->reipaddr[3]);
            break;
            case SSI_INDEX_SETT:
                    usnprintf(pcInsert, iInsertLen,  "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                              ptrDeviceSettings->dhcpOn,
                              ptrDeviceSettings->ipaddr[0],
                              ptrDeviceSettings->ipaddr[1], 
                              ptrDeviceSettings->ipaddr[2], 
                              ptrDeviceSettings->ipaddr[3],
                              ptrDeviceSettings->port,
                              ptrDeviceSettings->nmask[0],
                              ptrDeviceSettings->nmask[1],
                              ptrDeviceSettings->nmask[2],
                              ptrDeviceSettings->nmask[3],
                              ptrDeviceSettings->gw[0],
                              ptrDeviceSettings->gw[1],
                              ptrDeviceSettings->gw[2],
                              ptrDeviceSettings->gw[3],
                              ptrDeviceSettings->pr,
                              ptrDeviceSettings->reipaddr[0],
                              ptrDeviceSettings->reipaddr[1],
                              ptrDeviceSettings->reipaddr[2],
                              ptrDeviceSettings->reipaddr[3],
                              ptrDeviceSettings->report,
                              ptrDeviceSettings->mfEnabled
                            );
            break;

            case SSI_INDEX_MAC_LIST:
                if(ptrDeviceSettings->macFilterListLen == 0){
                    usnprintf(pcInsert, iInsertLen,"");
                }else{
                    for(volatile int i=0;i<ptrDeviceSettings->macFilterListLen;i++){
                            usnprintf(pcInsert+(18*i), iInsertLen, "%02X:%02X:%02X:%02X:%02X:%02X,",
                                (ptrDeviceSettings->macFilter[i]).macaddr[0],
                                (ptrDeviceSettings->macFilter[i]).macaddr[1],
                                (ptrDeviceSettings->macFilter[i]).macaddr[2],
                                (ptrDeviceSettings->macFilter[i]).macaddr[3],
                                (ptrDeviceSettings->macFilter[i]).macaddr[4],
                                (ptrDeviceSettings->macFilter[i]).macaddr[5]);

                    }
                }
            break;

            case  SSI_INDEX_REPORT:
                    usnprintf(pcInsert, iInsertLen,"%d", ptrDeviceSettings->report);
            break; 

            case SSI_INDEX_FPGA_FW_VERSION:
                    usnprintf(pcInsert, iInsertLen,FPGA_FW_VERSION);
            break;

            case SSI_INDEX_MCU_FW_VERSION:
                    usnprintf(pcInsert, iInsertLen,MCU_FW_VERSION);
            break;

            case SSI_INDEX_DATE:
                    usnprintf(pcInsert, iInsertLen, "2013-Feb-22");
            break;

        }
    }
    return(strlen(pcInsert));
} 

//******************************************************************************


char *StatusCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    //(ptrDeviceSettings->channelSettings[0]).controlReg[1] = 1;
    DebugMsg("\nEntering status clear\n");
   return(STATUS_CGI_RESPONSE);
}

/*******************************************************************************
 *
 * Handler for adding MAC address to list
 * 
 ******************************************************************************/
char *MacAddCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    long mac1,mac2,mac3,mac4,mac5,mac6;
    char pcDecodedString[32];
    unsigned char chn;
    
    mac1 = FindCGIParameter("mac1", pcParam, iNumParams);
    mac2 = FindCGIParameter("mac2", pcParam, iNumParams);
    mac3 = FindCGIParameter("mac3", pcParam, iNumParams);
    mac4 = FindCGIParameter("mac4", pcParam, iNumParams);
    mac5 = FindCGIParameter("mac5", pcParam, iNumParams);
    mac6 = FindCGIParameter("mac6", pcParam, iNumParams);
    if(mac1 == -1 || mac2 == -1 || mac3 == -1 || mac4 == -1 || mac5 == -1 || mac6 == -1) {
        return(PARAM_ERROR_RESPONSE);
    }
    
    if(ptrDeviceSettings->macFilterListLen >= MAC_LIST_SZIZE ){
        ptrDeviceSettings->macFilterListLen = MAC_LIST_SZIZE - 1;
    }
    DecodeFormString(pcValue[mac1], pcDecodedString, 32);
    chn = (unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[0] = chn;
    
    DecodeFormString(pcValue[mac2], pcDecodedString, 32);
    chn = (unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[1] = chn;
    
    DecodeFormString(pcValue[mac3], pcDecodedString, 32);
    chn = (unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[2] = chn;
    
    DecodeFormString(pcValue[mac4], pcDecodedString, 32);
    chn=(unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[3] = chn;
    
    DecodeFormString(pcValue[mac5], pcDecodedString, 32);
    chn=(unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[4] = chn;
    
    DecodeFormString(pcValue[mac6], pcDecodedString, 32);
    chn=(unsigned char)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr[5] = chn;
    
    ptrDeviceSettings->macFilterListLen++;
    
    if(!Settings_Write(ptrDeviceSettings)) {
        DebugMsg("Save to Flash failed!!\n"); 
    } else {
        DebugMsg("Setting saved to Flash memory\n");
    }
    return(SETTINGS_CGI_RESPONSE);
}

/**
 * Clear maclist
 */
char *ClearMacCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for(int i=0;i<MAC_LIST_SZIZE;i++){
        (ptrDeviceSettings->macFilter[i]).macaddr[0] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr[1] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr[2] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr[3] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr[4] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr[5] = 0;
    }
    ptrDeviceSettings->macFilterListLen = 0;
    return(SETTINGS_CGI_RESPONSE);
}

//******************************************************************************

//http://10.10.10.2/settings.cgi?chn=ch0&rxsens=highest&panfilter=none
char *SettingsCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{  
   long chn;
   
   chn = FindCGIParameter("ch", pcParam, iNumParams);
   

   if((chn == -1) )
   		return(PARAM_ERROR_RESPONSE); 
   
   //------------------------------------------------

   
   DebugMsg("Web Settings: %d, %d, %d, %s \n", chn);
   
   //       
   //Save setting to Flash memory   
   //       
   //if(!Settings_Write(ptrDeviceSettings)) 
     DebugMsg("Save to Flash failed!!\n"); 
   DebugMsg("Setting saved to Flash memory\n");

   return(DEFAULT_CGI_RESPONSE);
}


//******************************************************************************

char *IpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{ 
   long ipmode, ipaddr, nmask, gw;
   char pcDecodedString[32];
   struct in_addr temp_ip;
   unsigned long *pucTemp, ulIp, ulMask, ulGw;
    
   ipmode = FindCGIParameter("dhcp", pcParam, iNumParams);
   ipaddr = FindCGIParameter("ip1", pcParam, iNumParams);
   nmask = FindCGIParameter("nm1", pcParam, iNumParams);
   gw = FindCGIParameter("gw1", pcParam, iNumParams);	 
   DebugMsg("\nEntering ipset handler\n");
   if((ipmode == -1) || (ipaddr == -1) || (nmask == -1) || (gw == -1)) 
   		return(PARAM_ERROR_RESPONSE); 
    
   // TODO: Decode, parse and apply settings  
   
   DecodeFormString(pcValue[ipmode], pcDecodedString, 32);
   DebugMsg("%s\n", pcDecodedString);
   
   if(!strcmp(pcDecodedString,"enabled"))
   {
       pucTemp = (unsigned long *)&ptrDeviceSettings->ipaddr;     
      *pucTemp=0;
      
      pucTemp = (unsigned long *)&ptrDeviceSettings->nmask;      
      *pucTemp=0;
      
      pucTemp = (unsigned long *)&ptrDeviceSettings->gw;         
      *pucTemp=0;
       
  
       ptrDeviceSettings->dhcpOn=1;
          
   
   }
   else if(!strcmp(pcDecodedString,"disabled"))
   {
     
     
     DecodeFormString(pcValue[ipaddr], pcDecodedString, 32);
     DebugMsg("%s\n", pcDecodedString); 
     
     if(!inet_aton(pcDecodedString,&temp_ip))
       return(PARAM_ERROR_RESPONSE);      
     ulIp=(unsigned long)temp_ip.s_addr;      
     
     DecodeFormString(pcValue[nmask], pcDecodedString, 32);
     DebugMsg("%s\n", pcDecodedString);
     
     if(!inet_aton(pcDecodedString,&temp_ip))
       return(PARAM_ERROR_RESPONSE);
     ulMask=(unsigned long)temp_ip.s_addr;
     
     DecodeFormString(pcValue[gw], pcDecodedString, 32);
     DebugMsg("%s\n", pcDecodedString);
     
     if(!inet_aton(pcDecodedString,&temp_ip))
       return(PARAM_ERROR_RESPONSE);
     ulGw=(unsigned long)temp_ip.s_addr;  
     
     //
     // IP addresses can not be zeros in static configure
     //
     if(!ulIp || !ulMask || !ulGw) 
       return(PARAM_ERROR_RESPONSE);   
     
     //
     // IP addresses can not be match together
     //
     if(ulIp==ulMask || ulMask==ulGw || ulIp==ulGw)
       return(PARAM_ERROR_RESPONSE);
     
     //
     // IP address of device must be in same (sub)network like Gateway
     //
     if( (ulIp & ulMask) != (ulGw & ulMask) ) 
       return(PARAM_ERROR_RESPONSE);
     
    
    //
    // Save settings
    //      
    pucTemp = (unsigned long *)&ptrDeviceSettings->ipaddr; 
    *pucTemp=ulIp;
    
    pucTemp = (unsigned long *)&ptrDeviceSettings->nmask; 
    *pucTemp=ulMask;
    
    pucTemp = (unsigned long *)&ptrDeviceSettings->gw;   
    *pucTemp=ulGw;
    
    ptrDeviceSettings->dhcpOn=0;     
   
   }
   else return(PARAM_ERROR_RESPONSE);
       
   
   //       
   //Save setting to Flash memory   
   //       
   if(!Settings_Write(ptrDeviceSettings)) 
     DebugMsg("Save to Flash failed!!\n"); 
   DebugMsg("Setting saved to Flash memory\n");
   
   
   return(REDIR_CGI_RESPONSE);
}

//******************************************************************************

char *ProtocosetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[])
{
    DebugMsg("\nEntering protocol settings handler\n");
    return(SETTINGS_CGI_RESPONSE);
}
char *RemoteIpsetCGIHandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[])
{/*
   long remoteip, remoteport;
   char pcDecodedString[32];
   struct in_addr temp_ip;
   unsigned long *pucTemp;
   	
   remoteip = FindCGIParameter("remoteip", pcParam, iNumParams);
   remoteport = FindCGIParameter("remoteport", pcParam, iNumParams);	

   if((remoteip == -1) || (remoteport == -1)) 
   		return(PARAM_ERROR_RESPONSE); 

   DecodeFormString(pcValue[remoteip], pcDecodedString, 32);
   DebugMsg("%s\n", pcDecodedString);
   
   if(!inet_aton(pcDecodedString,&temp_ip))
       return(PARAM_ERROR_RESPONSE);
   remoteip=(unsigned long)temp_ip.s_addr;
   DebugMsg("Decode:%u\n", remoteip);

   DecodeFormString(pcValue[remoteport], pcDecodedString, 32);
   DebugMsg("%s\n", pcDecodedString);
   
   if((remoteport=ustrtoul(pcDecodedString,NULL,10))==0)
     return(PARAM_ERROR_RESPONSE);
   DebugMsg("Decode:%u\n", remoteport);
   
   pucTemp = (unsigned long *)&ptrDeviceSettings->ipaddr;
   
   if(remoteip==*pucTemp)  //p�i rovnosti adresy za��zen� a vzd�len� adresy
       return(PARAM_ERROR_RESPONSE);
   
   
   //
   //  Save parameters to global variables and apply.
   //
    pucTemp = (unsigned long *)&ptrDeviceSettings->reipaddr;
     
    *pucTemp=(unsigned long)remoteip;
    
    ptrDeviceSettings->report=remoteport;
    
         
    //   
    //Save setting to Flash memory 
    //                 
    if(!Settings_Write(ptrDeviceSettings))   
      DebugMsg("Save to Flash failed!!\n");  
    DebugMsg("Setting saved to Flash memory\n");
*/
   return(DEFAULT_CGI_RESPONSE);	
}
