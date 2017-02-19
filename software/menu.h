#ifndef _MENU_H_
#define _MENU_H_


//----- DEFINES -----
#define DEFAULT_BGCOLOR                RGB(255,255,255) //background
#define DEFAULT_FGCOLOR                RGB(  0,  0,  0) //foreground
#define DEFAULT_SELCOLOR               RGB(255,  0,  0) //selection
#define DEFAULT_EDGECOLOR              RGB(  0,144,240) //edges

#define MENU_LINES                     (8)  //lines
#define MENU_LINEHEIGHT                (15) //pixel

#define MENU_NOP                       (0)
#define MENU_PLAY                      (1)
#define MENU_UPDATE                    (2)
#define MENU_ERROR                     (3)
#define MENU_BACK                      (4)
#define MENU_BACKTXT                   "<< back <<"

#define MENU_STATE_STOP                (0)
#define MENU_STATE_BUF                 (1)
#define MENU_STATE_PLAY                (2)

#define CTRL_TEXT                      (0)
#define CTRL_BUTTON                    (1)
#define CTRL_CHECKBOX                  (2)
#define CTRL_INPUT                     (3)


typedef struct
{
  const char *name;
  const unsigned char *img[3];
  void(*init)(void);                      //init routine
  unsigned int(*items)(void);             //get item count
  void(*get)(unsigned int item, char *s); //get item name
  unsigned int(*open)(unsigned int item); //open item (ret: 0=play, 1=update, 2=error, 3=back)
  void(*close)(void);                     //close item
  void(*service)(void);                   //service routine
} MAINMENUITEM;

typedef struct
{
  unsigned int type; //text, button, checkbox, input
  unsigned int x1;
  unsigned int y1;
  unsigned int x2;
  unsigned int y2;
  char *val;        //value
  unsigned int len; //chars
  unsigned int sel; //control active/selected
  unsigned int p1;  //text: - | button: - | checkbox: checked | input: first
  unsigned int p2;  //text: - | button: - | checkbox: -       | input: sel (0xffff = select all)
} CONTROL;


//----- PROTOTYPES -----
unsigned int                           menu_openfile(char *file);
void                                   menu_stopfile(void);
unsigned int                           menu_sw(void);
unsigned int                           menu_swlong(void);
void                                   menu_up(void);
void                                   menu_down(void);
void                                   menu_steps(int steps);
void                                   menu_service(unsigned int draw);

void                                   menu_alarm(void);

void                                   menu_drawwndsub(unsigned int redraw);

void                                   menu_drawwndmain(unsigned int redraw);

void                                   menu_drawclock(unsigned int draw);
void                                   menu_drawdate(void);
void                                   menu_drawvol(void);
void                                   menu_drawstatus(void);
void                                   menu_setinfo(const char *info);
void                                   menu_setname(const char *name);
void                                   menu_setbitrate(unsigned int bitrate);
void                                   menu_setformat(unsigned int format);
void                                   menu_setstatus(unsigned int status);
void                                   menu_drawwndinfo(unsigned int redraw);

void                                   menu_drawwnd(unsigned int redraw);

void                                   menu_drawctrl(CONTROL *ctrl);
void                                   menu_createctrl(CONTROL *ctrl, unsigned int type, unsigned int sel, unsigned int x, unsigned int y, unsigned int p, char *value);
void                                   menu_drawdlg(const char *title, const char *msg);
void                                   menu_drawpopup(const char *msg);

void                                   menu_setedgecolor(unsigned int color);
void                                   menu_setselcolor(unsigned int color);
void                                   menu_setfgcolor(unsigned int color);
void                                   menu_setbgcolor(unsigned int color);
unsigned int                           menu_getedgecolor(void);
unsigned int                           menu_getselcolor(void);
unsigned int                           menu_getfgcolor(void);
unsigned int                           menu_getbgcolor(void);

void                                   menu_init(void);


#endif //_MENU_H_
