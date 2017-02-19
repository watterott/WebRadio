#ifndef _CARD_H_
#define _CARD_H_


//----- DEFINES -----
#define CARD_TIMEOUT                   (3) //seconds

#define CARD_CLOSED                    (0)
#define CARD_PLAY                      (1)


//----- PROTOTYPES -----
void                                   card_service(void);
void                                   card_closeitem(void);
unsigned int                           card_openfile(const char *file);
unsigned int                           card_nextitem(void);
unsigned int                           card_openitem(unsigned int item);
void                                   card_getitem(unsigned int item, char *name);
unsigned int                           card_items(void);
void                                   card_init(void);


#endif //_CARD_H_
