#ifndef _NBNS_H_
#define _NBNS_H_


//----- DEFINES -----
#define NBNS_PORT                      (137)

//Proto: NBNS Question
#define NBNSQ_HEADERLEN                (38)
#define NBNSQ_TYPE_NB                  (0x0020)
#define NBNSQ_CLASS_IN                 (0x0001)
typedef struct __attribute__((packed))
{
  unsigned int     len       :   8; // 8bit Len: 32
  char             name[33];        //33byte Name + null
  unsigned int     type      :  16; //16bit Type
  unsigned int     clas      :  16; //16bit Class
} NBNS_Question;

//Proto: NBNS Answer
#define NBNSA_HEADERLEN                (50)
#define NBNSA_TYPE_NB                  (0x0020)
#define NBNSA_CLASS_IN                 (0x0001)
typedef struct __attribute__((packed))
{
  unsigned int     len       :   8; // 8bit Len: 32
  char             name[33];        //33byte Name + null
  unsigned int     type      :  16; //16bit Type
  unsigned int     clas      :  16; //16bit Class
  unsigned long    ttl       :  32; //32bit Time to live
  unsigned int     rdlen     :  16; //16bit Data len
  unsigned int     flags     :  16; //16bit Flags    - rdata
  unsigned long    addr      :  32; //32bit IP Addr  - rdata
} NBNS_Answer;

//Proto: NBNS (NetBIOS Name service)
#define NBNS_OFFSET                    (ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN)
#define NBNS_HEADERLEN                 (12)
#define NBNS_OPMASK                    (0x7800)
#define NBNS_REPLYMASK                 (0x000F)
#define NBNS_FLAG_RESPONSE             (1<<15)
#define NBNS_FLAG_QUERY                (0<<15)
#define NBNS_FLAG_AUTHORITY            (1<<10)
#define NBNS_OP_QUERY                  (0<<11)

typedef struct __attribute__((packed))
{
  unsigned int     id        :  16; //16bit Transaction ID
  unsigned int     flags_op  :  16; //16bit Flags
  unsigned int     qdcount   :  16; //16bit Question Entries
  unsigned int     ancount   :  16; //16bit Answer RRs
  unsigned int     nscount   :  16; //16bit Authority RRs
  unsigned int     arcount   :  16; //16bit Additional RRs
  union
  {
    NBNS_Question qd;
    NBNS_Answer   an;
  } data;
} NBNS_Header;


//----- PROTOTYPES -----
void                                   nbns_reply(unsigned int idx, unsigned int id);
void                                   nbns_udpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_NBNS_H_
