#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../debug.h"
#include "../tools.h"
#include "../main.h"
#include "../vs.h"
#include "../eth.h"
#include "../menu.h"
#include "utils.h"
#include "http.h"
#include "http_files.h"
#include "upnp.h"


#define CONTROL_ACTION_RESP "" \
  "<?xml version='1.0'?>\r\n" \
  "<s:Envelope xmlns:s='http://schemas.xmlsoap.org/soap/envelope/' s:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>\r\n" \
  "  <s:Body>\r\n" \
  "    <u:%sResponse xmlns:u='urn:schemas-upnp-org:service:REMOTE:1'>\r\n" \
  "      <%s>%s</%s>\r\n" \
  "    </u:%sResponse>\r\n" \
  "  </s:Body>\r\n" \
  "</s:Envelope>\r\n" \
  "\r\n"


char upnp_id[20+1] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //uuid: xxxx-xx-xx-xx-xxxxxx


unsigned int upnp_getport(void)
{
  return UPNP_PORT;
}


char *upnp_getuuid(void)
{
  return upnp_id;
}


void upnp_tcpapp(unsigned int idx, const char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len;
  const char *s;
  char tmp[16];

  DEBUGOUT("UPnP: TCP app\n");

  if(rx_len)
  {
    if(strncmpi(rx, "GET ", 4) == 0)
    {
      rx     += 4;
      rx_len -= 4;
      http_sendfile(idx, rx, tx);
    }
    else if(strncmpi(rx, "POST /control", 13) == 0)
    {
      rx     += 13;
      rx_len -= 13;
  
      s = strstri(rx, "REMOTE:1#");
      if(s != 0)
      {
        s += 9;
  
        tx_len = sprintf((char*)tx, HTTP_XML_HEADER"\r\n\r\n");
        tx    += tx_len;
        if(strncmpi(s, "SETVOLUME", 7) == 0)
        {
          strstrk(tmp, (const char*)rx, "<s:Envelope\0<s:Body\0<u:SETVOLUME\0<VOLUME>\0\0");
          vs_setvolume(atoi(tmp));
          sprintf(tmp, "%i", vs_getvolume());
          tx_len += sprintf((char*)tx, CONTROL_ACTION_RESP, "SETVOLUME", "VOLUME", tmp, "VOLUME", "SETVOLUME");
        }
        else if(strncmpi(s, "GETVOLUME", 7) == 0)
        {
          sprintf(tmp, "%i", vs_getvolume());
          tx_len += sprintf((char*)tx, CONTROL_ACTION_RESP, "GETVOLUME", "VOLUME", tmp, "VOLUME", "GETVOLUME");
        }
        tcp_send(idx, tx_len, 0);
        tcp_close(idx);
      }
    }
    else
    {
      tx_len = sprintf((char*)tx, HTTP_400_HEADER);
      tcp_send(idx, tx_len, 0);
      tcp_close(idx);
    }
  }
  else
  {
    http_sendfile(idx, 0, tx);
  }

  return;
}
