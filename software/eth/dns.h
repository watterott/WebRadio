#ifndef _DNS_H_
#define _DNS_H_


//----- DEFINES -----
#define DNS_PORT                       (53)

#define DNS_TIMEOUT                    (3) //seconds

//Proto: DNS Answer
#define DNSA_HEADERLEN                 (12)
typedef struct __attribute__((packed))
{
  unsigned int     name      :  16; //16bit Name
  unsigned int     type      :  16; //16bit Type
  unsigned int     clas      :  16; //16bit Class
  unsigned long    ttl       :  32; //32bit Time to live
  unsigned int     rdlen     :  16; //16bit Data len
  //data
  unsigned long    addr      :  32; //32bit IP Address
} DNS_Answer;

//Proto: DNS (Domain Name System)
#define DNS_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN)
#define DNS_HEADERLEN                  (12)
#define DNS_DATASTART                  (ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN+DNS_HEADERLEN)
#define DNS_FLAGS_RESPONSE             (0x8000)
#define DNS_FLAGS_QUERY                (0x0100)
#define DNS_TYPE_A                     (0x0001)
#define DNS_TYPE_CNAME                 (0x0005)
#define DNS_CLASS_IN                   (0x0001)
typedef struct __attribute__((packed))
{
  unsigned int     id        : 16; //16bit ID
  unsigned int     flags     : 16; //16bit Flags
/*unsigned int     qr        :  1; // 1bit QR query or response
  unsigned int     opcode    :  4; // 4bit Opcode
  unsigned int     flags     :  7; // 7bit Flags
  unsigned int     rcode     :  4; // 4bit RCode*/
  unsigned int     qdcount   : 16; //16bit Entries in question section
  unsigned int     ancount   : 16; //16bit Entries in answer section
  unsigned int     nscount   : 16; //16bit Entries in authority section
  unsigned int     arcount   : 16; //16bit Entries in additional records section
} DNS_Header;


//----- PROTOTYPES -----
unsigned int                           dns_request(unsigned int idx, const char *domain);
IP_Addr                                dns_getip(const char *domain);
void                                   dns_udpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_DNS_H_
