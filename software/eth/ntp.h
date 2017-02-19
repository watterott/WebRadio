#ifndef _NTP_H_
#define _NTP_H_


//----- DEFINES -----
#define NTP_PORT                       (123)

#define NTP_TIMEOUT                    (2) //seconds

//Proto: NTP (Network Time Protocol)
#define NTP_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN)
#define NTP_HEADERLEN                  (48)
typedef struct __attribute__((packed))
{
  unsigned int     flags     :  8; // 8bit Flags: LI, VN, Mode
  unsigned int     stratum   :  8; // 8bit Stratum
  unsigned int     poll      :  8; // 8bit Poll
  unsigned int     prec      :  8; // 8bit Precision
  unsigned long    root_del  : 32; //32bit Root delay
  unsigned long    root_dis  : 32; //32bit Root dispersion
  unsigned long    ref_id    : 32; //32bit Reference identifier
  uint64_t         ref_ts    : 64; //64bit Reference timestamp
  uint64_t         org_ts    : 64; //64bit Originate timestamp
  uint64_t         rcv_ts    : 64; //64bit Receive timestamp
  uint64_t         trn_ts    : 64; //64bit Transmit timestamp
} NTP_Header;


//----- PROTOTYPES -----
unsigned int                           ntp_request(unsigned int idx);
unsigned long                          ntp_gettime(void);
void                                   ntp_udpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_NTP_H_
