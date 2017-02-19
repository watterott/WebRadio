#ifndef _BUFFER_H_
#define _BUFFER_H_


//----- DEFINES -----
#define CARD_READBUF                   (1024)   //bytes


typedef struct
{
  char name[MAX_NAME];
  char info[MAX_INFO];
  char file[MAX_ADDR];
} MENUBUFFER;

typedef struct
{
  char         name[MAX_NAME];
  char         info[MAX_INFO];
  char         addr[MAX_ADDR];
  char         host[MAX_ADDR];
  MAC_Addr     mac;
  IP_Addr      ip;
  unsigned int port;
  char         file[MAX_URLFILE];
} STATIONBUFFER;

typedef struct
{
  char name[MAX_NAME];
  char info[MAX_INFO];
  char file[MAX_ADDR];
} SHAREBUFFER;

typedef struct
{
  char          name[MAX_NAME];
  char          info[MAX_INFO];
  char          file[MAX_ADDR];
  FIL           fsrc;
  unsigned char buf[CARD_READBUF];
} CARDBUFFER;

typedef union
{
  MENUBUFFER    menu;
  STATIONBUFFER station;
  SHAREBUFFER   share;
  CARDBUFFER    card;
} BUFFER;


//----- GLOBALS -----
extern BUFFER gbuf;


//----- PROTOTYPES -----
void                                   buf_service(void);
void                                   buf_puts(const unsigned char *s, unsigned int len);
unsigned int                           buf_size(void);
unsigned int                           buf_free(void);
unsigned int                           buf_len(void);
void                                   buf_reset(void);


#endif //_BUFFER_H_
