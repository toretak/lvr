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
#include "../fpga/fpga_defs.h"
#include "../fpga/FPGA_IO_LWIP.h"
#include "../myutils/uartstdio.h"
#include "../device.h"
#include "http_conf.h"
#include "cgifuncs.h"
#include "utils/arp.h"
//******************************************************************************

tDeviceSettings *ptrDeviceSettings;

//******************************************************************************

void HttpdInit(tDeviceSettings *ptr) {
    ptrDeviceSettings = ptr;
    http_set_ssi_handler(SSIHandler, g_pcConfigSSITags,NUM_CONFIG_SSI_TAGS);
    http_set_cgi_handlers(g_psConfigCGIURIs, NUM_CONFIG_CGI_URIS);
    httpd_init(ptr);
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
        int chIndex = iIndex - SSI_INDEX_CHN_STATE_LOW;
#ifndef SIMULATION_MODE        
        // read channel status
        // save adapter states to structure
        get_adapter_status(ptrDeviceSettings->channelSettings);       
        // save adapter states to flash
        if(!Settings_Write(ptrDeviceSettings)) {
                DebugMsg("Save to Flash failed!!\n"); 
        } else {
                DebugMsg("Setting saved to Flash memory\n");
        }
        
        switch(ptrDeviceSettings->channelSettings[chIndex].adapterStatusReg & 0x0F){
            case 0x00:
                usnprintf(pcInsert, iInsertLen, "CHOK"); // channel OK
                break;
            case 0x01:
                usnprintf(pcInsert, iInsertLen, "TXOC"); // tx open circuit
                break;
            case 0x02:
                usnprintf(pcInsert, iInsertLen, "TXSC"); // tx short circuit
                break;
            case 0x04:
                usnprintf(pcInsert, iInsertLen, "RXOC"); // rx open circuit
                break;
            case 0x05:
                usnprintf(pcInsert, iInsertLen, "TORO"); // tx open circuit, rx open circuit
                break;
            case 0x06:
                usnprintf(pcInsert, iInsertLen, "TSRO"); // tx short circuit, rx open circuit
                break;
            default:
                usnprintf(pcInsert, iInsertLen, "CHER");
                break;
        }
        
#else
    usnprintf(pcInsert, iInsertLen, "CHOK");
#endif       
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
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[0],
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[1],
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[2],
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[3],
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[4],
                                (ptrDeviceSettings->macFilter[i]).macaddr.addr[5]);

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
                    usnprintf(pcInsert, iInsertLen, BUILD_DATE);
            break;

	    case SSI_INDEX_ID:
		    usnprintf(pcInsert, iInsertLen, "%06d",ptrDeviceSettings->device_id);
	    break;
	    
	    case SSI_INDEX_SN:
		    usnprintf(pcInsert, iInsertLen, "#%05d",ptrDeviceSettings->serial_number);
	    break;
            
            //TODO: case defaults, dodelat posilani poradi znaku s udaji na webce
        }
    }
    return(strlen(pcInsert));
} 

//******************************************************************************


char *StatusCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
		
    //TODO: Presunout do SSIHandler a vyresit zobrazovani statusu, tady dodelat clear
    
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
    u8_t chn;
    
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
    chn = (u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[0] = chn;
    
    DecodeFormString(pcValue[mac2], pcDecodedString, 32);
    chn = (u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[1] = chn;
    
    DecodeFormString(pcValue[mac3], pcDecodedString, 32);
    chn = (u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[2] = chn;
    
    DecodeFormString(pcValue[mac4], pcDecodedString, 32);
    chn=(u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[3] = chn;
    
    DecodeFormString(pcValue[mac5], pcDecodedString, 32);
    chn=(u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[4] = chn;
    
    DecodeFormString(pcValue[mac6], pcDecodedString, 32);
    chn=(u8_t)ustrtoul((char*)&pcDecodedString,NULL,16);
    (ptrDeviceSettings->macFilter[ptrDeviceSettings->macFilterListLen]).macaddr.addr[5] = chn;
    
    ptrDeviceSettings->macFilterListLen++;
    
    if(!Settings_Write(ptrDeviceSettings)) {
        DebugMsg("Save to Flash failed!!\n"); 
    } else {
        DebugMsg("Setting saved to Flash memory\n");
    }
    return(SETTINGS_CGI_RESPONSE);
}

/**
 * Toggle MAC filter state
 */
char *ToggleMacFilterCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{    
    ptrDeviceSettings->mfEnabled = !ptrDeviceSettings->mfEnabled;
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
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[0] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[1] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[2] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[3] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[4] = 0;
        (ptrDeviceSettings->macFilter[i]).macaddr.addr[5] = 0;
    }
    ptrDeviceSettings->macFilterListLen = 0;
    if(!Settings_Write(ptrDeviceSettings)) {
        DebugMsg("Save to Flash failed!!\n"); 
    } else {
        DebugMsg("Setting saved to Flash memory\n");
    }
    return(SETTINGS_CGI_RESPONSE);
}

//******************************************************************************

//http://10.10.10.2/settings.cgi?chn=ch0&rxsens=highest&panfilter=none
char *SettingsCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{  
		DebugMsg("called SettingsCGIHandler\n"); 
    long chn;
		chn = FindCGIParameter("ch", pcParam, iNumParams); // chn: 0x31 - 0x38 (channel number), 0x61 (all channels))
		if(chn == -1) return(PARAM_ERROR_RESPONSE);
   
		char pcDecodedString[32];
		unsigned long temp; 
		char control_reg = 0, frame_sel_reg = 0;

		// fill up control register
		// get channel enabled/disabled selection from string
		temp = FindCGIParameter("en", pcParam, iNumParams);
		if(temp == -1) return(PARAM_ERROR_RESPONSE);
		DecodeFormString(pcValue[temp], pcDecodedString, 32);
		switch (pcDecodedString[0]) {
		case 't':
			control_reg |= ENABLE_CHANNEL;
			break;
		case 'f':
			control_reg |= RESET;
			break;
		default:
			control_reg |= ENABLE_CHANNEL;
			break;
		}
		
		// get BAUD rate from string
		temp = FindCGIParameter("br", pcParam, iNumParams);
		if(temp == -1) return(PARAM_ERROR_RESPONSE);
		DecodeFormString(pcValue[temp], pcDecodedString, 32);
		switch (pcDecodedString[0]) {
		case '1':
			switch (pcDecodedString[1]) {
			case '1': // 110 BAUDS
				control_reg |= 2;
			break;
			case '2': // 1200 BAUDS
				control_reg |= 6;
			break;
			case '5': // 150 BAUDS
				control_reg |= 3;
			break;
			default: // 9600 BAUDS
				control_reg |= 9;
			break;
			}
			break;
		case '2': // 2400 BAUDS
			control_reg |= 7;
			break;
		case '3': // 300 BAUDS
			control_reg |= 4;
			break;
		case '4': // 4800 BAUDS
			control_reg |= 8;
			break;
		case '5': // 50 BAUDS
			//control_reg |= 0;
			break;
		case '6': // 600 BAUDS
			control_reg |= 5;
			break;
		case '7': // 75 BAUDS
			control_reg |= 1;
			break;
		case '9': // 9600 BAUDS
			control_reg |= 9;
			break;
		default: // 9600 BAUDS
			control_reg |= 9;
			break;
		}
    DebugMsg("SettingsCGIHandler: control register content received from html string (%X)\n", control_reg);

		// fill up frame selection register
		// get data bits from string
		temp = FindCGIParameter("db", pcParam, iNumParams);
		if(temp == -1) return(PARAM_ERROR_RESPONSE);
		DecodeFormString(pcValue[temp], pcDecodedString, 32);
		// create frame_sel part
		// choose the data length
		switch (pcDecodedString[0]) {
		case '5':
			// DATA_BITS0 and DATA_BITS1 0 
			break;
		case '6':
			frame_sel_reg |= DATA_BITS0;
			break;
		case '7':
			frame_sel_reg |= DATA_BITS1;
			break;
		case '8':
			frame_sel_reg |= (DATA_BITS0 | DATA_BITS1);
			break;
		default:
			frame_sel_reg |= (DATA_BITS0 | DATA_BITS1);
			break;
		}
	
		// get stop bits from string
		temp = FindCGIParameter("sb", pcParam, iNumParams);
		if(temp == -1) return(PARAM_ERROR_RESPONSE);
		DecodeFormString(pcValue[temp], pcDecodedString, 32);
		switch (pcDecodedString[0]) {
		case 0:
			frame_sel_reg |= STOP_BITS0;
			break;
		case 1:
			frame_sel_reg |= STOP_BITS1;
			break;
		case 2:
			frame_sel_reg |= (STOP_BITS0 | STOP_BITS1);
			break;
		default:
			frame_sel_reg |= STOP_BITS0;
			break;
		}
		
		// get parity from string
		temp = FindCGIParameter("db", pcParam, iNumParams);
		if(temp == -1) return(PARAM_ERROR_RESPONSE);
		DecodeFormString(pcValue[temp], pcDecodedString, 32);
		switch (pcDecodedString[0]) {
		case 0:
			// no change, no parity
			break;
		case 1:
			frame_sel_reg |= PARITY_ENABLE;
			break;
		case 2:
			frame_sel_reg |= (PARITY_ENABLE | PARITY_ODD);
			break;
		default:
			// no change, no parity
			break;
		}
    DebugMsg("SettingsCGIHandler: frame selection register content received from html string (%X)\n", frame_sel_reg);
   
#ifdef ONE_CHANNEL
		// store settings to structure
		ptrDeviceSettings->channelSettings[0].controlReg = control_reg;
		ptrDeviceSettings->channelSettings[0].frameSelReg = frame_sel_reg;
		//TODO returns t_error_FPGA
#ifndef SIMULATION_MODE  
		configure_FPGA_channel(0, ptrDeviceSettings->channelSettings[0]);
    DebugMsg("SettingsCGIHandler: control register sent to FPGA");
#endif
#else		
		// single channel setting
		if (chn >= 0x31 && chn <= 0x38) {
			// store settings to structure
			ptrDeviceSettings->channelSettings[chn - 0x31].controlReg = control_reg;
			ptrDeviceSettings->channelSettings[chn - 0x31].frameSelReg = frame_sel_reg;
			//TODO returns t_error_FPGA
			configure_FPGA_channel(chn - 0x31, ptrDeviceSettings->channelSettings[chn - 0x31]);
		// set all channels
		} else if (chn == 0x61) {
			volatile int i;
			for (i = 0; i < 8; i++) {
				// store settings to structure
				ptrDeviceSettings->channelSettings[i].controlReg = control_reg;
				ptrDeviceSettings->channelSettings[i].frameSelReg = frame_sel_reg;
				//TODO returns t_error_FPGA
#ifndef SIMULATION_MODE				
        configure_FPGA_channel(i, ptrDeviceSettings->channelSettings[i]);
        DebugMsg("SettingsCGIHandler: frame selection register sent to FPGA");
#endif
			}
		}
#endif		
   
		DebugMsg("Web Settings: %d, %d, %d, %s \n", chn);
   
		//Save setting to Flash memory   
		if(!Settings_Write(ptrDeviceSettings)) DebugMsg("Save to Flash failed!!\n"); 
		else DebugMsg("Setting saved to Flash memory\n");
		
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
    
    //TODO everything
    
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
