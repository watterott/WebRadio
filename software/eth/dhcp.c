#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../debug.h"
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "dhcp.h"


volatile unsigned int dhcp_status=DHCP_CLOSED;
volatile long dhcp_timeout=0L;
unsigned long dhcp_id=0UL;
IP_Addr dhcp_server=0UL;
IP_Addr dhcp_ip=0UL;
IP_Addr dhcp_netmask=0UL;
IP_Addr dhcp_router=0UL;
IP_Addr dhcp_dns=0UL;
IP_Addr dhcp_ntp=0UL;


unsigned int dhcp_request(unsigned int idx, unsigned int msg)
{
  DHCP_Header *tx_dhcp;
  unsigned int i=0;

  dhcp_timeout = getontime()+DHCP_TIMEOUT;

  tx_dhcp = (DHCP_Header*) &eth_txbuf[DHCP_OFFSET];

  memset(tx_dhcp, 0, DHCP_HEADERLEN);

  tx_dhcp->op           = DHCP_OP_REQUEST;
  tx_dhcp->htype        = DHCP_HTYPE_ETH;
  tx_dhcp->hlen         = DHCP_HLEN_MAC;
  tx_dhcp->xid          = swap32(dhcp_id);
  tx_dhcp->chaddr.mac   = eth_getmac();
  tx_dhcp->mcookie      = SWAP32(DHCP_MCOOKIE);

  tx_dhcp->options[i++] = DHCP_OPTION_MSGTYPE; 
  tx_dhcp->options[i++] = 1;   //Len
  tx_dhcp->options[i++] = msg; //Type

  tx_dhcp->options[i++] = DHCP_OPTION_PARAMLIST;
  tx_dhcp->options[i++] = 4; //Len
  tx_dhcp->options[i++] = DHCP_OPTION_NETMASK;
  tx_dhcp->options[i++] = DHCP_OPTION_ROUTER;
  tx_dhcp->options[i++] = DHCP_OPTION_TIMESERVER; 
  tx_dhcp->options[i++] = DHCP_OPTION_DNS;

  tx_dhcp->options[i++] = DHCP_OPTION_CLIENTID;
  tx_dhcp->options[i++] = 6; //Len
  tx_dhcp->options[i++] = (eth_getmac()>> 0)&0xff;
  tx_dhcp->options[i++] = (eth_getmac()>> 8)&0xff;
  tx_dhcp->options[i++] = (eth_getmac()>>16)&0xff;
  tx_dhcp->options[i++] = (eth_getmac()>>24)&0xff;
  tx_dhcp->options[i++] = (eth_getmac()>>32)&0xff;
  tx_dhcp->options[i++] = (eth_getmac()>>40)&0xff;

  if(msg == DHCP_MSG_REQUEST)
  {
    tx_dhcp->options[i++] = DHCP_OPTION_SERVERID;
    tx_dhcp->options[i++] = 4; //Len
    tx_dhcp->options[i++] = (dhcp_server>> 0)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>> 8)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>>16)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>>24)&0xff;

    tx_dhcp->options[i++] = DHCP_OPTION_REQUESTEDIP;
    tx_dhcp->options[i++] = 4; //Len
    tx_dhcp->options[i++] = (dhcp_ip>> 0)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>> 8)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>>16)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>>24)&0xff;
  }

  tx_dhcp->options[i++] = DHCP_OPTION_HOSTNAME; //Host Name
  tx_dhcp->options[i++] = strlen(eth_getname()); //Len
  i += sprintf((char*)&tx_dhcp->options[i], "%s", eth_getname()); //Name

  tx_dhcp->options[i++] = 0xff; //END Option
  tx_dhcp->options[i++] = 0x00;
  tx_dhcp->options[i++] = 0x00;

  idx = udp_open(idx, 0x000000000000ULL, 0xFFFFFFFFUL, DHCPSERVER_PORT, DHCPCLIENT_PORT, 0, DHCP_HEADERLEN+4+i); //header + mcookie + options

  return idx;
}


unsigned int dhcp_getcfg(void)
{
  long timeout;
  IP_Addr ip;
  unsigned int idx;

  ip = eth_getip(); //save current device ip
  eth_setip(0UL); //set device ip to zero

  dhcp_status    = DHCP_DISCOVER;
  dhcp_timeout   = getontime()+DHCP_TIMEOUT;
  dhcp_id        = generate_id();
  dhcp_server    = 0UL;
  dhcp_ip        = 0UL;
  dhcp_netmask   = 0UL;
  dhcp_router    = 0UL;
  dhcp_dns       = 0UL;
  dhcp_ntp       = 0UL;

  idx = dhcp_request(UDP_ENTRIES, DHCP_MSG_DISCOVER); 

  if(idx < UDP_ENTRIES)
  {
    timeout = getontime()+ETH_TIMEOUT;
    for(;;)
    {
      eth_service();
  
      if(dhcp_status == DHCP_ACK)
      {
        break;
      }
      if(getdeltatime(dhcp_timeout) > 0)
      {
        switch(dhcp_status)
        {
          case DHCP_DISCOVER:
            idx = dhcp_request(idx, DHCP_MSG_DISCOVER); 
            break;
          case DHCP_REQUEST:
            idx = dhcp_request(idx, DHCP_MSG_REQUEST);
            break;
        }
      }
      if(getdeltatime(timeout) > 0)
      {
        break;
      }
    }
  
    udp_close(idx);
  }

  if(dhcp_status == DHCP_ACK) //DHCP request successful
  {
    if(dhcp_ip != 0UL)
    {
      eth_setip(dhcp_ip);
    }
    else
    {
      eth_setip(ip); //set last ip
    }
    if(dhcp_netmask != 0UL)
    {
      eth_setnetmask(dhcp_netmask);
    }
    if(dhcp_router != 0UL)
    {
      eth_setrouter(dhcp_router);
    }
    if(dhcp_dns != 0UL)
    {
      eth_setdns(dhcp_dns);
    }
    if(dhcp_ntp != 0UL)
    {
      eth_setntp(dhcp_ntp);
    }
    dhcp_status = DHCP_CLOSED;

    return 0;
  }

  eth_setip(ip); //set last ip

  dhcp_status = DHCP_CLOSED;

  return 1;
}


void dhcp_udpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  const DHCP_Header *rx_dhcp;
  const unsigned char *ptr;
  unsigned char c, len, msg;

  DEBUGOUT("DHCP: UDP app\n");

  rx_dhcp = (const DHCP_Header*) rx;

  if((rx_dhcp->op              == DHCP_OP_REPLY)   &&
     (rx_dhcp->htype           == DHCP_HTYPE_ETH)  &&
     (rx_dhcp->hlen            == DHCP_HLEN_MAC)   &&
     (rx_dhcp->xid             == swap32(dhcp_id)) &&
     (rx_dhcp->chaddr.mac      == eth_getmac())    &&
     (rx_dhcp->mcookie         == SWAP32(DHCP_MCOOKIE)))
  {
    switch(dhcp_status)
    {
      case DHCP_DISCOVER:
        msg = 0;
        ptr = rx_dhcp->options;
        while(*ptr != 0xff)
        {
          c  = *ptr++;
          if(c == 0xff) //END
          {
            break;
          }
          len = *ptr++;
          switch(c)
          {
            case DHCP_OPTION_MSGTYPE:    if(len == 1){ msg = ptr[0]; }
              break;
            case DHCP_OPTION_NETMASK:    if(len == 4){ dhcp_netmask = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0); }
              break;
            case DHCP_OPTION_ROUTER:     if(len == 4){ dhcp_router  = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0); }
              break;
            case DHCP_OPTION_TIMESERVER: if(len == 4){ dhcp_ntp     = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0); }
              break;
            case DHCP_OPTION_DNS:        if(len == 4){ dhcp_dns     = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0); }
              break;
            case DHCP_OPTION_SERVERID:   if(len == 4){ dhcp_server  = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0); }
              break;
          }
          while(len--){ ptr++; }
        }
        if(msg == DHCP_MSG_OFFER)
        {
          dhcp_ip = rx_dhcp->yiaddr; //get ip
          dhcp_request(idx, DHCP_MSG_REQUEST);
          dhcp_status = DHCP_REQUEST;
        }
        break;

      case DHCP_REQUEST:
        msg = 0;
        ptr = rx_dhcp->options;
        while(*ptr != 0xff)
        {
          c  = *ptr++;
          if(c == 0xff) //END
          {
            break;
          }
          len = *ptr++;
          switch(c)
          {
            case DHCP_OPTION_MSGTYPE:    if(len == 1){ msg = ptr[0]; }
              break;
          }
          while(len--){ ptr++; }
        }
        if(msg == DHCP_MSG_ACK)
        {
          dhcp_status = DHCP_ACK;
        }
        break;
    }
  }

  return;
}
