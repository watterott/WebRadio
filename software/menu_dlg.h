#ifndef _MENU_DLG_H_
#define _MENU_DLG_H_


//----- PROTOTYPES -----
unsigned int                           dlg_ip(const char* title, IP_Addr *value);
unsigned int                           dlg_rgb(const char* title, unsigned int *value);
unsigned int                           dlg_or(const char* title, int *value, int p1, int p2);
unsigned int                           dlg_nr(const char* title, int *value, int min, int max, int step);
unsigned int                           dlg_str(const char* title, const char *value, unsigned int len, char *buf, unsigned int buf_len);
unsigned int                           dlg_alarmtime(ALARMTIME *time);
unsigned int                           dlg_rawir(unsigned int i);
unsigned int                           dlg_msg(const char* title, const char *msg);
int                                    dlg_service(void);


#endif //_MENU_DLG_H_
