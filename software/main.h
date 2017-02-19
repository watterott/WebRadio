#ifndef _MAIN_H_
#define _MAIN_H_


//----- DEFINES -----
//Application settings
#define VERSION                        "0.14"
#define RELEASE                        //Release version
#define APPNAME                        "WebRadio" //max 15 characters
#if defined DEBUG
# define APPVERSION                    VERSION"d"
#elif defined RELEASE
# define APPVERSION                    VERSION
#else
# define APPVERSION                    VERSION"*"
#endif

//Max characters
#define MAX_NAME                       (32)  // 31 chars + zero (min 32)  "Station Name"
#define MAX_INFO                       (100) // 99 chars + zero (min 32)  "Station info"
#define MAX_ADDR                       (260) //259 chars + zero           "http://192.168.0.100/stream.mp3" or "/test/abc/xyz.mp3"
#define MAX_URLFILE                    (50)  // 49 chars + zero           "/stream.mp3"

#define DRAWALL                        (1<<0)
#define SEC_CHANGED                    (1<<1)
#define MIN_CHANGED                    (1<<2)
#define HOUR_CHANGED                   (1<<3)
#define DAY_CHANGED                    (1<<4)
#define ALARM_PLAY                     (1<<5)
#define ALARM_STANDBY                  (1<<6)


//----- PROTOTYPES -----
void                                   systick(void);
char*                                  getclock(void);
char*                                  getdate(void);
void                                   gettime(TIME* t);
void                                   settime(unsigned long s);
long                                   getdeltatime(long t);
long                                   getontime(void);

unsigned int                           standby_isactive(void);
void                                   standby_off(void);
unsigned int                           standby(unsigned int param);


#endif //_MAIN_H_
