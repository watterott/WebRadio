#ifndef _SHARE_H_
#define _SHARE_H_


//----- DEFINES -----
#define SHARE_FILE                     "SHARE.PLS"


//----- PROTOTYPES -----
void                                   share_service(void);
void                                   share_closeitem(void);
unsigned int                           share_openitem(unsigned int item);
void                                   share_getitem(unsigned int item, char *name);
unsigned int                           share_items(void);
void                                   share_init(void);


#endif //_SHARE_H_
