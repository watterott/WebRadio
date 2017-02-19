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
#include "upnp.h"
#include "ssdp.h"


void ssdp_advertise(void)
{
  unsigned int idx, len;
  char *ssdp;

  if(uuid_test(upnp_getuuid()) == 0)
  {
    uuid_generate(upnp_getuuid());
  }

  ssdp = (char*) &eth_txbuf[SSDP_OFFSET];

  len = sprintf(ssdp, "NOTIFY * HTTP/1.0\r\n"
                      "Host: %i.%i.%i.%i:%i\r\n"
                      "Cache-Control: max-age=900\r\n"
                      "Location: http://%i.%i.%i.%i:%i/device.xml\r\n"
                      "NT: upnp:rootdevice\r\n"
                      "USN: uuid:%s::upnp:rootdevice\r\n"
                      "NTS: ssdp:alive\r\n"
                      "Server: "APPNAME"/"APPVERSION" UPnP/1.0 %s\r\n\r\n",
                      (int)((SSDP_MULTICAST>>0)&0xff), (int)((SSDP_MULTICAST>>8)&0xff), (int)((SSDP_MULTICAST>>16)&0xff), (int)((SSDP_MULTICAST>>24)&0xff),
                      SSDP_PORT,
                      (int)((eth_getip()>>0)&0xff), (int)((eth_getip()>>8)&0xff), (int)((eth_getip()>>16)&0xff), (int)((eth_getip()>>24)&0xff),
                      UPNP_PORT,
                      upnp_getuuid(),
                      eth_getname());

  idx = udp_open(UDP_ENTRIES, MULTICAST_MAC(SSDP_MULTICAST), SSDP_MULTICAST, SSDP_PORT, SSDP_PORT, 0, len);
  udp_close(idx);

  DEBUGOUT("SSDP: advertise\n");

  return;
}


void ssdp_udpapp(unsigned int idx, const char *rx, unsigned int rx_len, unsigned char *tx)
{
  DEBUGOUT("SSDP: UDP app\n");

  if(strncmpi(rx, "M-SEARCH *", 10) == 0)
  {
    ssdp_advertise();
  }

  return;
}
