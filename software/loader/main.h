#ifndef _MAIN_H_
#define _MAIN_H_


//----- DEFINES -----
//Application settings
#define VERSION                        "0.08"
#define RELEASE                        //Release version
#define APPNAME                        "Loader"
#if defined DEBUG
# define APPVERSION                    VERSION"d"
#elif defined RELEASE
# define APPVERSION                    VERSION
#else
# define APPVERSION                    VERSION"*"
#endif

//Loader settings
#define APPSTARTADDR                   (0x5000)  // 20kByte
#define FLASHSIZE                      (0x40000) //256 kByte
#define FLASHBUF                       (1024)
#define FIRMWARE_FILE                  "/FIRMWARE.BIN"
#define FIRMWARE_BAKFILE               "/FIRMWARE.BAK"


//----- PROTOTYPES -----
void                                   systick(void);
long                                   backup_app(const char* fname);
long                                   flash_app(const char* fname);
void                                   start_app(void);


#endif //_MAIN_H_
