#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "dns.h"
#include "utils.h"


unsigned int base64_test(char c)
{
  const char table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned int i;

  for(i=0; i<64; i++)
  {
    if(table[i] == c)
    {
      return i+1;
    }
  }

  return 0;
}


unsigned int base64_decode(unsigned char *dst, const unsigned char *src, unsigned int len)
{
  unsigned int i, pos=0, written=0;
  unsigned char c, buf[4];

  //skip none-base64 characters
  while((base64_test(*src) == 0) && (pos<len)){ src++; pos++; }

  //decode data
  while(1)
  {
    for(i=0; (i<4) && (pos<len); src++, pos++)
    {
      c = base64_test(*src);
      if(c)
      {
        buf[i++] = c-1; //save to buffer
      }
      else if(*src == '=')
      {
        buf[i++] = 0xFF; //skip
      }
      else
      {
        break;
      }
    }

    if(i == 4)
    {
      *dst++ =  (buf[0]       << 2) | ((buf[1]&0x30) >> 4);
      *dst++ = ((buf[1]&0x0F) << 4) | ((buf[2]&0x3C) >> 2);
      *dst++ = ((buf[2]&0x03) << 6) |   buf[3];
      written += 3;
      if(buf[2] == 0xFF)
      {
        written--;
      }
      if(buf[3] == 0xFF)
      {
        written--;
      }
    }
    else
    {
      break;
    }
  }

  return written;
}


unsigned int uuid_test(char *uuid) //uuid: xxxx-xx-xx-xx-xxxxxx
{
  unsigned int i;

  for(i=20; i!=0; i--)
  {
    if(*uuid++ != 0)
    {
      return 1;
    }
  }

  return 0;
}


void uuid_generate(char *uuid) //uuid: xxxx-xx-xx-xx-xxxxxx
{
  unsigned long r;
  unsigned int a, b, c, d, e, f;

  r = generate_id();
  a = (r>> 0) & 0xFFFF;
  b = (r>>16) & 0xFF;
  c = (r>>24) & 0xFF;
  r = generate_id();
  d = (r>> 0) & 0xFF;
  e = (r>> 8) & 0xFFFF;
  f = (r>>24) & 0xFF;

  sprintf(uuid, "%04X-%02X-%02X-%02X-%04X%02X", (a&0xFFFF), (b&0xFF), (c&0xFF), (d&0xFF), (e&0xFFFF), (f&0xFF));

  return;
}


unsigned int nbns_decode(char *dst, const char *src)
{
  unsigned int i, j;
  char c;

  for(i=0, j=0; i<15; i++)
  {
    c  = (src[j++]-'A')<<4;
    c |= (src[j++]-'A')<<0;
    if(c == ' ')
    {
      break;
    }
    dst[i] = toupper(c);
  }
  dst[i] = 0;

  return (((src[30]-'A')<<4)|(src[31]-'A')); //0x00 = Workstation
}


void nbns_encode(char *dst, const char *src, unsigned int type)
{
  char c;
  unsigned int i, j;

  //encode name
  for(i=0, j=0; (i<15) && src[i]; i++)
  {
    c = toupper(src[i]);
    dst[j++] = 'A'+((c>>4)&0x0f);
    dst[j++] = 'A'+((c>>0)&0x0f);
  }

  //add spaces
  for(; i<15; i++)
  {
    dst[j++] = 'A'+((' '>>4)&0x0f);
    dst[j++] = 'A'+((' '>>0)&0x0f);
  }

  //set type (0x00 = Workstation)
  dst[j++] = 'A'+((type>>4)&0x0f);
  dst[j++] = 'A'+((type>>0)&0x0f);

  return;
}


unsigned int url_decode(char *dst, const char *src, unsigned int len)
{
  unsigned int i;
  char c, *ptr, buf[4]={0,0,0,0};

  ptr = dst; //save dst

  for(i=0; i<len;)
  {
    c = *src++; i++;
    if((c == 0)    || 
       (c == '&')  ||
       (c == ' ')  ||
       (c == '\n') ||
       (c == '\r'))
    {
      break;
    }
    else if(c == '%')
    {
      buf[0] = *src++; i++;
      buf[1] = *src++; i++;
      *dst++ = (unsigned char)atou_hex(buf);
    }
    else if(c == '+')
    {
      *dst++ = ' ';
    }
    else
    {
      *dst++ = c;
    }
  }

  *dst = 0;

  //remove space at start and end of string
  strrmvspace(ptr, ptr);

  return i;
}


char* http_skiphd(const char *src, unsigned int *len)
{
  unsigned int i;
 
  for(i=*len; i!=0; i--, src++)
  {
    if(i >= 4)
    {
      if((src[0] == '\r') && (src[1] == '\n') && (src[2] == '\r') && (src[3] == '\n'))
      {
        src += 4;
        i   -= 4;
        break;
      }
    }
  }

  *len = i;

  return (char*)src;
}


unsigned int http_hdparamcontentlen(const char *src)
{
  char buf[16];
  unsigned int i;

  if(http_hdparam(buf, 16-1, src, "CONTENT-LENGTH:") == 0)
  {
    for(i=0; buf[i] && !isdigit(buf[i]); i++);//skip non-digits
    return atoi(&buf[i]);
  }

  return 0;
}


unsigned long http_hdparamul(const char *src, const char *param)
{
  char buf[16];
  unsigned int i;

  if(http_hdparam(buf, 16-1, src, param) == 0)
  {
    for(i=0; buf[i] && !isdigit(buf[i]); i++);//skip non-digits
    return atou(&buf[i]);
  }

  return 0;
}


unsigned int http_hdparam(char *dst, size_t dst_len, const char *src, const char *param)
{
  char *ptr;

  ptr = dst; //save dst

  src = strstri(src, param);
  if(src)
  {
    src += strlen(param);
    for(; dst_len!=0; dst_len--)
    {
      if((*src==0) || (*src=='\n') || (*src=='\r'))
      {
        break;
      }
      *dst++ = *src++;
    }
    *dst = 0;
    //remove space at start and end of string
    strrmvspace(ptr, ptr);
    if(strlen(ptr))
    {
      return 0;
    }
  }

  return 1;
}


unsigned int http_response(const char *src)
{
  unsigned int search=16;

  while((*src==' ') && search){ src++; search--; } //skip spaces

  if((strncmpi(src, "ICY", 3)  == 0) ||
     (strncmpi(src, "HTTP", 4) == 0) ||
     (strncmpi(src, "RTSP", 4) == 0))
  {
    while(*src && (*src!=' ') && search){ src++; search--; } //skip proto name
    while(        (*src==' ') && search){ src++; search--; } //skip spaces
    if(search)
    {
      return atoi(src);
    }
  }

  return 0;
}


unsigned long generate_id(void)
{
  if(getontime()&1)
  {
    srand(getontime()+rand());
  }
  else
  {
    srand(getontime()-rand());
  }

  return eth_getmac()+rand();
}


//proto://user:password@xxx.xxx.xxx.xxx:port/file
//proto://user:password@host:port/file
void atoaddr(char *s, char *proto, char *user, char *pwrd, char *host, unsigned int *port, char *file)
{
  if(proto){ *proto = 0; }
  if(user) { *user  = 0; }
  if(pwrd) { *pwrd  = 0; }
  if(host) { *host  = 0; }
  if(port) { *port  = 0; }
  if(file) { *file++ = '/'; *file = 0; }

  while(*s==' '){ s++; } //skip spaces

  //get proto
  if(strncmpi(s, "ftp://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "ftp");
    }
  }
  else if(strncmpi(s, "http://", 7) == 0)
  {
    s += 7;
    if(proto)
    {
      strcpy(proto, "http");
    }
  }
  else if(strncmpi(s, "mms://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "mms");
    }
  }
  else if(strncmpi(s, "nfs://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "nfs");
    }
  }
  else if(strncmpi(s, "rtsp://", 7) == 0)
  {
    s += 7;
    if(proto)
    {
      strcpy(proto, "rtsp");
    }
  }
  else if(strncmpi(s, "smb://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "smb");
    }
  }
  else
  {
    return;
  }

  //get user & password
  if(strstr(s, "@") != 0)
  {
    while(*s && (*s!=':'))
    {
      if(user)
      {
        *user++ = *s;
        *user   = 0;
      }
      s++;
    }
    s++; //skip ":"
    while(*s && (*s!='@'))
    {
      if(pwrd)
      {
        *pwrd++ = *s;
        *pwrd   = 0;
      }
      s++;
    }
    s++; //skip "@"
  }

  //get host
  while(*s && (*s!=':') && (*s!='/'))
  {
    if(host)
    {
      *host++ = *s;
      *host   = 0;
    }
    s++;
  }

  //get port
  if(*s==':')
  {
    s++; //skip ":"
    if(port)
    {
      *port = atoi(s);
    }
    while(isdigit(*s)){ s++; }; //skip port
  }

  //get file
  if(*s == '/')
  {
    s++; //skip "/"
    while(*s && !isspace(*s))
    {
      if(file)
      {
        *file++ = *s;
        *file   = 0;
      }
      s++;
    }
  }

  return;
}


char* mactoa(MAC_Addr mac_addr) //xx:xx:xx:xx:xx:xx
{
  static char addr[18];
  MAC mac;

  mac.b64 = mac_addr;
  sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X", mac.b8[0], mac.b8[1], mac.b8[2], mac.b8[3], mac.b8[4], mac.b8[5]);

  return addr;
}


MAC_Addr atomac(char *s)
{
  MAC_Addr mac=0;
  uint64_t i;

  i = atou_hex(s);
  mac |= i<<0;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atou_hex(s);
  mac |= i<<8;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atou_hex(s);
  mac |= i<<16;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atou_hex(s);
  mac |= i<<24;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atou_hex(s);
  mac |= i<<32;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atou_hex(s);
  mac |= i<<40;

  return mac;
}


char* iptoa(IP_Addr ip_addr) //xxx.xxx.xxx.xxx
{
  static char addr[16];
  IP ip;

  ip.b32 = ip_addr;
  sprintf(addr, "%03i.%03i.%03i.%03i", ip.b8[0], ip.b8[1], ip.b8[2], ip.b8[3]);

  return addr;
}


IP_Addr atoip(char *s)
{
  IP_Addr ip=0;

  if(isdigit(*s)) //ip
  {
    ip |= atou(s)<<0;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atou(s)<<8;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atou(s)<<16;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atou(s)<<24;
  }
  else //get ip -> dns resolve
  {
    ip = dns_getip(s);
  }

  return ip;
}


unsigned long long swap64(unsigned long long i)
{
  return (((i&0xFF00000000000000ULL)>>56)|
          ((i&0x00FF000000000000ULL)>>40)|
          ((i&0x0000FF0000000000ULL)>>24)|
          ((i&0x000000FF00000000ULL)>> 8)|
          ((i&0x00000000FF000000ULL)<< 8)|
          ((i&0x0000000000FF0000ULL)<<24)|
          ((i&0x000000000000FF00ULL)<<40)|
          ((i&0x00000000000000FFULL)<<56));
}


unsigned long swap32(unsigned long i)
{
  return (((i&0xFF000000UL)>>24)|
          ((i&0x00FF0000UL)>> 8)|
          ((i&0x0000FF00UL)<< 8)|
          ((i&0x000000FFUL)<<24));
}


unsigned int swap16(unsigned int i)
{
  return (((i&0x00FF)<<8)|
          ((i&0xFF00)>>8));
}
