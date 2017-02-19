#ifndef _SETTINGS_H_
#define _SETTINGS_H_


//----- DEFINES -----
#define SETTINGS_FILE                  "SETTINGS.INI"

//settingsmenu[] infos
#define F_NONE                         (0) //-
#define F_NR                           (1) //p1-p2, p3=step size
#define F_OR                           (2) //p1 or p2
#define F_STR                          (3) //p1=max len
#define F_MAC                          (4) //-
#define F_IP                           (5) //-
#define F_RGB                          (6) //-
#define F_RUN                          (7) //-
#define F_TIME                         (8) //-
#define F_INFO                         (9) //-
#define SETTINGSITEMS                  (40)

typedef struct
{
  const char *name; //name
  const char *ini;  //name
  const int format; //format
  const int p1;     //param 1
  const int p2;     //param 2
  const int p3;     //param 3
  void(*set)(void*);
} SETTINGSMENU;


//----- GLOBALS -----
extern const SETTINGSMENU settingsmenu[];


//----- PROTOTYPES -----
unsigned int                           settings_getplaymode(void);
void                                   settings_read(void);
unsigned int                           settings_openitem(unsigned int item);
void                                   settings_getitem(unsigned int item, char *name);
unsigned int                           settings_items(void);
void                                   settings_init(void);


#endif //_SETTINGS_H_
