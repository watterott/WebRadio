#ifndef _UTILS_H_
#define _UTILS_H_


//----- DEFINES -----
#define SWAP16(x)                      ((((x)&0x00FF)<<8)| \
                                        (((x)&0xFF00)>>8))

#define SWAP32(x)                      ((((x)&0xFF000000UL)>>24)| \
                                        (((x)&0x00FF0000UL)>> 8)| \
                                        (((x)&0x0000FF00UL)<< 8)| \
                                        (((x)&0x000000FFUL)<<24))

#define SWAP64(x)                      ((((x)&0xFF00000000000000ULL)>>56)| \
                                        (((x)&0x00FF000000000000ULL)>>40)| \
                                        (((x)&0x0000FF0000000000ULL)>>24)| \
                                        (((x)&0x000000FF00000000ULL)>> 8)| \
                                        (((x)&0x00000000FF000000ULL)<< 8)| \
                                        (((x)&0x0000000000FF0000ULL)<<24)| \
                                        (((x)&0x000000000000FF00ULL)<<40)| \
                                        (((x)&0x00000000000000FFULL)<<56))

#define MULTICAST_MAC(x)               (SWAP64(0x01005E0000000000ULL)|((x&0xFFFF7F00ULL)<<16ULL))


//----- PROTOTYPES -----

unsigned int                           base64_test(char c);
unsigned int                           base64_decode(unsigned char *dst, const unsigned char *src, unsigned int len);

unsigned int                           uuid_test(char *uuid);
void                                   uuid_generate(char *uuid);

unsigned int                           nbns_decode(char *dst, const char *src);
void                                   nbns_encode(char *dst, const char *src, unsigned int type);

unsigned int                           url_decode(char *dst, const char *src, unsigned int len);

char*                                  http_skiphd(const char *src, unsigned int *len);
unsigned int                           http_hdparamcontentlen(const char *src);
unsigned long                          http_hdparamul(const char *src, const char *param);
unsigned int                           http_hdparam(char *dst, size_t dst_len, const char *src, const char *param);
unsigned int                           http_response(const char *src);

unsigned long                          generate_id(void);

void                                   atoaddr(char *s, char *proto, char *user, char *pwrd, char *host, unsigned int *port, char *file);
char*                                  mactoa(MAC_Addr mac_addr);
MAC_Addr                               atomac(char *s);
char*                                  iptoa(IP_Addr ip_addr);
IP_Addr                                atoip(char *s);

unsigned long long                     swap64(unsigned long long i);
unsigned long                          swap32(unsigned long i);
unsigned int                           swap16(unsigned int i);


#endif //_UTILS_H_
