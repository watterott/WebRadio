#ifndef _MMC_H_
#define _MMC_H_


//----- PROTOTYPES -----
void                                   fs_unmount(void);
void                                   fs_mount(void);

#ifndef LOADER

//unsigned int                           fs_checkitem(FILINFO *finfo);
unsigned int                           fs_isdir(const char *path, unsigned int item);
void                                   fs_getitemtag(const char *path, unsigned int item, char *dst, unsigned int len);
void                                   fs_getitem(const char *path, unsigned int item, char *dst, unsigned int len);
unsigned int                           fs_items(const char *path);


//void                                   ini_stripfile(FIL *file, unsigned int pos, unsigned int len);
//void                                   ini_extendfile(FIL *file, unsigned int pos, unsigned int len);
//void                                   ini_delspace(FIL *file);
//unsigned int                           ini_searchentry(FIL *file, const char *entry, unsigned int *entry_start);
unsigned int                           ini_getentry(const char *filename, const char *entry, char *value, unsigned int len);
unsigned int                           ini_setentry(const char *filename, const char *entry, const char *value);
unsigned int                           ini_delentry(const char *filename, const char *entry);
unsigned int                           ini_renentry(const char *filename, const char *entry, const char *newentry);

#endif


#endif //_MMC_H_
