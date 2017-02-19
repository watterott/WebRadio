#ifndef _RTSP_H_
#define _RTSP_H_


//----- DEFINES -----
#define RTSP_SERVERPORT                (554) //554 or 8554
#define RTSP_CLIENTPORT1               (1011)
#define RTSP_CLIENTPORT2               (1012)
#define RTSP_CLIENTPORT3               (1013)

#define RTSP_TIMEOUT                   (15) //s
#define RTSP_TRY                       (3)  //times

#define RTSP_CLOSED                    (0)
#define RTSP_CLOSE                     (1)
#define RTSP_OPENED                    (2)
#define RTSP_PLAY                      (3)
#define RTSP_SETUP                     (4)
#define RTSP_DESCRIBE2                 (5)
#define RTSP_DESCRIBE1                 (6)
#define RTSP_GET                       (7)
#define RTSP_OPEN                      (8)
#define RTSP_ADDRMOVED                 (9)
#define RTSP_ERROR                     (10)
#define RTSP_ERRTIMEOUT                (11)


//Proto: RTSP (Real-Time Streaming Protocol)
#define RTSP_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN+TCP_HEADERLEN)
#define RTSP_HEADERLEN                  (4)
typedef struct __attribute__((packed))
{
  unsigned int     magic     :  8; // 8bit Magic: 0x24
  unsigned int     chn       :  8; // 8bit Channel
  unsigned int     len       : 16; //16bit Length
} RTSP_Header;

//Proto: RTP (Real-Time Transport Protocol)
#define RTP_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN+TCP_HEADERLEN+RTSP_HEADERLEN)
#define RTP_HEADERLEN                  (12)
typedef struct __attribute__((packed))
{
  unsigned int     flags     :  8; // 8bit Flags
  unsigned int     type      :  8; // 8bit Type
  unsigned int     seq       : 16; //16bit Sequence Number
  unsigned long    time      : 32; //32bit Time Stamp
  unsigned long    ssrc      : 32; //32bit Synchronization Source
} RTP_Header;


//Proto: RTPASF (Real-Time Transport Protocol ASF Payload Header)
#define RTPASF_HEADERLEN               (4)
typedef struct __attribute__((packed))
{
  unsigned int     flags     :  8; // 8bit Flags
  unsigned int     len       : 24; //24bit Length
} RTPASF_Header;


//----- PROTOTYPES -----
void                                   rtsp_close(void);
unsigned int                           rtsp_open(void);
void                                   rtsp_putdata(const unsigned char *s, unsigned int len);
void                                   rtsp_tcpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_RTSP_H_
