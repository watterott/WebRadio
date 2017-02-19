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
#include "dns.h"


volatile IP_Addr dns_ip=0UL;
unsigned int dns_id=0;


unsigned int dns_request(unsigned int idx, const char *domain)
{
  MAC_Addr mac;
  DNS_Header *tx_dns;
  unsigned char *ptr, *ptr_len;
  unsigned int len=0, i;

  tx_dns = (DNS_Header*) &eth_txbuf[DNS_OFFSET];

  tx_dns->id      = swap16(dns_id);
  tx_dns->flags   = SWAP16(DNS_FLAGS_QUERY);
  tx_dns->qdcount = SWAP16(0x0001);
  tx_dns->ancount = 0;
  tx_dns->nscount = 0;
  tx_dns->arcount = 0;

  //convert domain name
  // www.google.de
  // 03 www 06 google 02 de
  ptr = &eth_txbuf[DNS_DATASTART];
  len = 0;
  while((*domain != 0) && (*domain != '/') && (*domain != ':'))
  {
    ptr_len = ptr++; len++;
    for(i=0; (*domain != 0) && (*domain != '.') && (*domain != '/') && (*domain != ':'); i++)
    {
      *ptr++ = *domain++; len++;
    }
    while((*domain != 0) && (*domain == '.')){ *domain++; }
    *ptr_len = i; //write len
  }
  *ptr++ = 0;
  //type: A
  *ptr++ = 0x00;
  *ptr++ = 0x01;
  //class: IN
  *ptr++ = 0x00;
  *ptr++ = 0x01;

  len += 5;

  mac = arp_getmac(eth_getdns());
  idx = udp_open(idx, mac, eth_getdns(), DNS_PORT, DNS_PORT, 0, DNS_HEADERLEN+len);

  return idx;
}


IP_Addr dns_getip(const char *domain)
{
  long timeout, timeout_dns;
  unsigned int idx;

  dns_ip = 0UL;
  dns_id = generate_id();

  idx = dns_request(UDP_ENTRIES, domain);

  if(idx < UDP_ENTRIES)
  {
    timeout     = getontime()+ETH_TIMEOUT;
    timeout_dns = getontime()+DNS_TIMEOUT;
    for(;;)
    {
      eth_service();
  
      if(dns_ip != 0UL)
      {
        break;
      }
      if(getdeltatime(timeout_dns) > 0)
      {
        timeout_dns = getontime()+DNS_TIMEOUT;
        idx = dns_request(idx, domain);
      }
      if(getdeltatime(timeout) > 0)
      {
        break;
      }
    }
  
    udp_close(idx);
  }

  return dns_ip;
}


void dns_udpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  const DNS_Header *rx_dns;
  const DNS_Answer *rx_dnsa;
  const unsigned char *data;
  unsigned int len;

  DEBUGOUT("DNS: UDP app\n");

  rx_dns = (const DNS_Header*) rx;

  if((rx_dns->id == swap16(dns_id))               &&
     (rx_dns->flags & SWAP16(DNS_FLAGS_RESPONSE)) &&
    ((rx_dns->flags&SWAP16(0x000F)) == 0)) //0 = no error
  {
    data = (const unsigned char*) &rx[DNS_HEADERLEN];
    len  = rx_len - DNS_HEADERLEN;
  
    //query
    while((*data != 0) && (len != 0)){ data++; len--; }
    if(len)
    {
      data += 5; len -= 5; //5 = 1 (null) + 2 (type) + 2 (class)
    }
  
    //answers
    while(len > DNSA_HEADERLEN)
    {
      rx_dnsa = (const DNS_Answer*) data;
  
      if((rx_dnsa->type  == SWAP16(DNS_TYPE_A))   &&
         (rx_dnsa->clas  == SWAP16(DNS_CLASS_IN)) &&
         (rx_dnsa->rdlen == SWAP16(0x0004))) //4 bytes = ip address
      {
        dns_ip = rx_dnsa->addr;
        break;
      }
      else
      {
        data += DNSA_HEADERLEN + swap16(rx_dnsa->rdlen);
        len  -= DNSA_HEADERLEN + swap16(rx_dnsa->rdlen);
      }
    }
  }

  return;
}
