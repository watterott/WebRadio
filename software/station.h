#ifndef _STATION_H_
#define _STATION_H_


//----- DEFINES -----
#define STATION_FILE                   "STATION.PLS"

#define STATION_TIMEOUT                (15) //s (play->buffer->play)
#define STATION_TRY                    (3)  //x times

#define STATION_CLOSED                 (0)
#define STATION_OPENED                 (1)
#define STATION_BUFFER                 (2)
#define STATION_OPEN                   (3)
#define STATION_ADDRMOVED              (4)
#define STATION_ERROR                  (5)
#define STATION_ERRTIMEOUT             (6)


//----- PROTOTYPES -----
void                                   station_setbitrate(unsigned int bitrate);
void                                   station_close(void);
unsigned int                           station_open(unsigned int item);
void                                   station_service(void);
void                                   station_closeitem(void);
unsigned int                           station_openitem(unsigned int item);

void                                   station_delitem(unsigned int item);
void                                   station_moveitem(unsigned int item, unsigned int direction);
void                                   station_setitemaddr(unsigned int item, const char *addr);
unsigned int                           station_getitemaddr(unsigned int item, char *addr);
void                                   station_setitem(unsigned int item, const char *name);
void                                   station_getitem(unsigned int item, char *name);
void                                   station_setitems(unsigned int items);
unsigned int                           station_items(void);
void                                   station_init(void);


#endif //_STATION_H_
