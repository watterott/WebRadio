#ifndef _ETH_H_
#define _ETH_H_


//----- DEFINES -----
#define DEFAULT_MAC                    "00:01:23:45:67:89"
#define DEFAULT_DHCP                   (1)
#define DEFAULT_IP                     "192.168.000.050"
#define DEFAULT_NETMASK                "255.255.255.000"
#define DEFAULT_ROUTER                 "192.168.000.001"
#define DEFAULT_DNS                    "192.168.000.001"
#define DEFAULT_NTP                    "078.046.194.189" //078.046.194.189 = 0.de.pool.ntp.org
#define DEFAULT_TIMEDIFF               (3600) //seconds (3600sec = 1h = GMT+1)
#define DEFAULT_SUMMER                 (0)    //summer time on

#define ETH_RXFIFO                     (8) //rx fifo (x * ETH_MTUSIZE)
#define ETH_MTUSIZE                    (1500+ETH_HEADERLEN) //1500 bytes (rx and tx buffer)
#define ETH_TIMEOUT                    (15) //seconds (ARP / DHCP / DNS request...)
#define ETH_USE_DSCP                   //use Differentiated Services Code Point (QoS -> DSCP)

#define TCP_MSS                        (ETH_MTUSIZE-ETH_HEADERLEN-IP_HEADERLEN-TCP_HEADERLEN) //Maximum Segment Size
#define TCP_WINDOW                     ((ETH_RXFIFO*ETH_MTUSIZE)/2) //(2048-256) //window size (ARM has 2kByte Rx FIFO for up to 31 frames)
#define TCP_ENTRIES                    (20) //max TCP Table Entries
#define TCP_TIMEOUT                    (3) //seconds
#define TCP_MAXERROR                   (3) //try x times
#define TCP_CLOSED                     (0)
#define TCP_OPENED                     (1)
#define TCP_OPEN                       (2)
#define TCP_ABORT                      (3)
#define TCP_CLOSE                      (4)
#define TCP_FIN                        (5)

#define UDP_ENTRIES                    (5) //max UDP Table Entries
#define UDP_TIMEOUT                    (4) //seconds
#define UDP_CLOSED                     (0)
#define UDP_OPENED                     (1)


typedef union
{
  uint8_t  b8[6];
  uint16_t b16[3];
  uint32_t b32[2];
  uint64_t b64;
} MAC;

typedef union
{
  uint8_t  b8[4];
  uint16_t b16[2];
  uint32_t b32;
} IP;

typedef uint64_t MAC_Addr;
typedef uint32_t IP_Addr;

typedef struct
{
  MAC_Addr mac;
  IP_Addr  ip;
  IP_Addr  netmask;
  IP_Addr  router;
  int      dhcp;     //0 = DHCP off
  IP_Addr  dns;
  IP_Addr  ntp;
  int      timediff; //Time Diff to GMT in seconds
  int      summer;   //Summer Time
  char     name[16]; //NetBios Name
} Device;

typedef struct
{
  MAC_Addr      mac;
  IP_Addr       ip;
  unsigned int  port;
  unsigned int  local_port;
  unsigned long acknum;
  unsigned long seqnum;
  unsigned int  flags;
  unsigned int  status;
  unsigned int  time;
  unsigned int  error;
} TCP_Table;

typedef struct
{
  MAC_Addr      mac;
  IP_Addr       ip;
  unsigned int  port;
  unsigned int  local_port;
  unsigned int  status;
  unsigned int  time;
} UDP_Table;

//Proto: Ethernet
#define ETH_OFFSET                     (0x0000)
#define ETH_HEADERLEN                  (14)
#define ETH_TYPE_IP                    SWAP16(0x0800)
#define ETH_TYPE_ARP                   SWAP16(0x0806)
typedef struct __attribute__((packed))
{
  uint64_t         dst_mac   : 48; //48bit Dst MAC
  uint64_t         src_mac   : 48; //48bit Src MAC
  unsigned int     type      : 16; //16bit Type: 0x0800=IP, 0x0806=ARP
} ETH_Header;

//Proto: ARP (Address Resolution Protocol)
#define ARP_OFFSET                     (ETH_HEADERLEN)
#define ARP_HEADERLEN                  (28)
#define ARP_HW_TYPE                    SWAP16(0x0001)
#define ARP_PRO_TYPE                   SWAP16(0x0800)
#define ARP_HW_LEN                     (0x06)
#define ARP_PRO_LEN                    (0x04)
#define ARP_OP_REQUEST                 SWAP16(0x0001)
#define ARP_OP_REPLY                   SWAP16(0x0002)
typedef struct __attribute__((packed))
{
  unsigned int     hw_type   : 16; //16bit Hardware Type
  unsigned int     pro_type  : 16; //16bit Protocol Type
  unsigned int     hw_len    :  8; // 8bit Hardware Addr Len (6 -> 6byte MAC Addr)
  unsigned int     pro_len   :  8; // 8bit Protocol Addr Len (4 -> 4byte IP Addr)
  unsigned int     op        : 16; //16bit Operation
  uint64_t         src_mac   : 48; //48bit Src MAC
  unsigned long    src_ip    : 32; //32bit Src IP
  uint64_t         dst_mac   : 48; //48bit Dst MAC
  unsigned long    dst_ip    : 32; //32bit Dst IP
} ARP_Header;

//Proto: IP v4 (Internet Protocol)
#define IP_OFFSET                      (ETH_HEADERLEN)
#define IP_HEADERLEN                   (20)
#define IP_PROTO_ICMP                  (0x01)
#define IP_PROTO_TCP                   (0x06)
#define IP_PROTO_UDP                   (0x11)
typedef struct __attribute__((packed))
{
  unsigned int     hd_len    :  4; // 4bit Version
  unsigned int     ver       :  4; // 4bit Header Len
  unsigned int     tos       :  8; // 8bit Type of Service
  unsigned int     len       : 16; //16bit Packet Len
  unsigned int     id        : 16; //16bit ID
  unsigned int     flg_offs  : 16; // 3bit Flags + 13bit Fragment-Offset
  unsigned int     ttl       :  8; // 8bit Time to Live
  unsigned int     proto     :  8; // 8bit Protocol
  unsigned int     checksum  : 16; //16bit Checksum of IP Header
  unsigned long    src_ip    : 32; //32bit IP Src
  unsigned long    dst_ip    : 32; //32bit IP Dst
} IP_Header;

//Proto: ICMP (Internet Control Message Protocol)
#define ICMP_OFFSET                    (ETH_HEADERLEN+IP_HEADERLEN)
#define ICMP_HEADERLEN                 (4)
#define ICMP_DATASTART                 (ETH_HEADERLEN+IP_HEADERLEN+ICMP_HEADERLEN)
#define ICMP_ECHO_REP                  (0x00)
#define ICMP_ECHO_REQ                  (0x08)
typedef struct __attribute__((packed))
{
  unsigned int     type      :  8; // 8bit Type (0=Echo Reply, 8=Echo Request)
  unsigned int     code      :  8; // 8bit Code
  unsigned int     checksum  : 16; //16bit Checksum
  unsigned int     seq       :  8; // 8bit Sequence (data start)
} ICMP_Header;

//Proto: TCP (Transmission Control Protocol)
#define TCP_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN)
#define TCP_HEADERLEN                  (20)
#define TCP_DATASTART                  (ETH_HEADERLEN+IP_HEADERLEN+TCP_HEADERLEN)
#define TCP_FLAG_FIN                   (0x01)
#define TCP_FLAG_SYN                   (0x02)
#define TCP_FLAG_RST                   (0x04)
#define TCP_FLAG_PSH                   (0x08)
#define TCP_FLAG_ACK                   (0x10)
typedef struct __attribute__((packed))
{
  unsigned int     src_port  : 16; //16bit Src Port
  unsigned int     dst_port  : 16; //16bit Dst Port
  unsigned long    seqnum    : 32; //32bit Seqence Nubmer
  unsigned long    acknum    : 32; //32bit Acknowledgment Number
  unsigned int     reserved  :  4; // 4bit Reserved
  unsigned int     len       :  4; // 4bit Header Len in 32bit Steps
  unsigned int     flags     :  8; // 8bit Flags
  unsigned int     window    : 16; //16bit Window (max. 64Kbyte)
  unsigned int     checksum  : 16; //16bit Checksum
  unsigned int     urgent    : 16; //16bit Urgent Pointer
  uint8_t          options[44];
} TCP_Header;

//Proto: UDP (User Datagram Protocol)
#define UDP_OFFSET                     (ETH_HEADERLEN+IP_HEADERLEN)
#define UDP_HEADERLEN                  (8)
#define UDP_DATASTART                  (ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN)
typedef struct __attribute__((packed))
{
  unsigned int     src_port  : 16; //16bit Src Port
  unsigned int     dst_port  : 16; //16bit Dst Port
  unsigned int     len       : 16; //16bit Len (Header + Data)
  unsigned int     checksum  : 16; //16bit Checksum
} UDP_Header;


//----- GLOBALS -----
extern unsigned char *eth_rxbuf, eth_txbuf[];


//----- PROTOTYPES -----
void                                   udp_close(unsigned int idx);
unsigned int                           udp_open(unsigned int idx, MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int dst_port, unsigned int src_port, unsigned char *data, unsigned int len);
void                                   udp_send(unsigned int idx, unsigned int len);
void                                   udp_app(unsigned int idx);
void                                   udp_service(void);

void                                   tcp_abort(unsigned int idx);
void                                   tcp_close(unsigned int idx);
unsigned int                           tcp_open(unsigned int idx, MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int dst_port, unsigned int src_port);
void                                   tcp_send(unsigned int idx, unsigned int len, unsigned int options);
void                                   tcp_app(unsigned int idx);
void                                   tcp_service(void);

void                                   icmp_service(void);
void                                   arp_request(IP_Addr ip);
MAC_Addr                               arp_getmac(IP_Addr ip);
void                                   arp_service(void);

void                                   make_udp_header(unsigned int idx, unsigned int len);
unsigned int                           checksum_tcp(unsigned char *s, unsigned int len, IP_Addr dst_ip);
void                                   make_tcp_header(unsigned int idx, unsigned int len, unsigned int options);
unsigned int                           checksum_ip(unsigned char *s, unsigned int len);
void                                   make_ip_header(MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int len, unsigned int proto);
void                                   make_arp_header(MAC_Addr dst_mac, MAC_Addr arp_dst_mac, IP_Addr arp_dst_ip, unsigned int op);
void                                   make_eth_header(MAC_Addr dst_mac, unsigned int type);

void                                   eth_timerservice(void);
void                                   eth_service(void);

void                                   eth_setname(char *name);
void                                   eth_setsummer(int on);
void                                   eth_settimediffh(int hours);  //hours
void                                   eth_settimediff(int seconds); //seconds
void                                   eth_setntp(IP_Addr ntp);
void                                   eth_setdns(IP_Addr dns);
unsigned int                           eth_setdhcp(int on);
void                                   eth_setrouter(IP_Addr r);
void                                   eth_setnetmask(IP_Addr nm);
void                                   eth_setip(IP_Addr ip);
void                                   eth_setmac(MAC_Addr mac);

char*                                  eth_getname(void);
int                                    eth_getsummer(void);
int                                    eth_gettimediffh(void); //hours
int                                    eth_gettimediff(void);  //seconds
IP_Addr                                eth_getntp(void);
IP_Addr                                eth_getdns(void);
int                                    eth_getdhcp(void);
IP_Addr                                eth_getrouter(void);
IP_Addr                                eth_getnetmask(void);
IP_Addr                                eth_getip(void);
MAC_Addr                               eth_getmac(void);

unsigned int                           eth_rxget(void);
unsigned int                           eth_rxput(void);
unsigned int                           eth_rxfree(void);

void                                   eth_init(void);


#endif //_ETH_H_
