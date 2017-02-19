#ifndef _UPNP_H_
#define _UPNP_H_


//----- DEFINES -----
#define UPNP_PORT                      (8080)


//----- PROTOTYPES -----
unsigned int                           upnp_getport(void);
char*                                  upnp_getuuid(void);
void                                   upnp_tcpapp(unsigned int idx, const char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_UPNP_H_
