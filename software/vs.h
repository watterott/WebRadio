#ifndef _VS_H_
#define _VS_H_


//----- DEFINES -----
#define DEFAULT_VOLUME                 (40)     //   0 -   100 %
#define DEFAULT_BASSAMP                (5)      //   0 -    15 dB
#define DEFAULT_BASSFREQ               (100)    //  20 -   150 Hz
#define DEFAULT_TREBLEAMP              (0)      //  -8 -     7 dB
#define DEFAULT_TREBLEFREQ             (15000)  //1000 - 15000 Hz

//VS FiFo
#define VS_BUFSIZE                     (42*1024) //42 kBytes

//VS Type
#define VS_UNKNOWN                     (0)
#define VS1033C                        (1)
#define VS1033D                        (2)
#define VS1053B                        (3)

//Audio Format
#define FORMAT_UNKNOWN                 (0)
#define FORMAT_WAV                     (1)
#define FORMAT_MP3                     (2)
#define FORMAT_AAC                     (3)
#define FORMAT_OGG                     (4)
#define FORMAT_WMA                     (5)
#define FORMAT_FLAC                    (6)

//Clock
#define VS_XTAL                        (12288000UL)
//Opcode
#define VS_READ                        (0x03)
#define VS_WRITE                       (0x02)
//Register
#define VS_MODE                        (0x00)   //Mode control
#define SM_RESET                       (1<< 2)  //Soft Reset
#define SM_CANCEL                      (1<< 3)  //Cancel Decoding
#define SM_STREAM                      (1<< 6)  //Stream Mode
#define SM_SDINEW                      (1<<11)  //VS1002 native SPI modes
#define VS_STATUS                      (0x01)   //Status
#define VS_BASS                        (0x02)   //Built-in bass/treble enhancer
#define VS_CLOCKF                      (0x03)   //Clock freq + multiplier
#define VS1033_SC_MUL_2X               (0x4000)
#define VS1033_SC_MUL_3X               (0x8000)
#define VS1033_SC_MUL_4X               (0xC000)
#define VS1053_SC_MUL_2X               (0x2000)
#define VS1053_SC_MUL_3X               (0x6000)
#define VS1053_SC_MUL_4X               (0xA000)
#define VS_DECODETIME                  (0x04)   //Decode time in seconds
#define VS_AUDATA                      (0x05)   //Misc. audio data
#define VS_WRAM                        (0x06)   //RAM write/read
#define VS_WRAMADDR                    (0x07)   //Base address for RAM write/read
#define VS_HDAT0                       (0x08)   //Stream header data 0
#define VS_HDAT1                       (0x09)   //Stream header data 1
#define VS_AIADDR                      (0x0A)   //Start address of application
#define VS_VOL                         (0x0B)   //Volume control
//RAM Data
#define VS_RAM_ENDFILLBYTE             (0x1E06)  //End fill byte


typedef union
{
  uint8_t  b8[VS_BUFSIZE];
  uint16_t b16[VS_BUFSIZE/2];
  uint32_t b32[VS_BUFSIZE/4];
} VSBUFFER;


//ASF Header
/*typedef struct __attribute__((packed))
{
  //ASF Header
  uint8_t          hd[16];         //16byte ASFHeader GUID 75B22630-668E-11CF-A6D9-00AA0062CE6C
  uint64_t         hdlen     : 64; //64bit Size
  unsigned long    obj       : 32; //32bit Object count
  unsigned int     reserved1 :  8; // 8bit Reserved1 = 0x01
  unsigned int     reserved2 :  8; // 8bit Reserved2 = 0x02
  //ASF File Properties
  uint8_t          file[16];       //16byte ASFFileProperties GUID 8CABDCA1-A947-11CF-8EE4-00C00C205365
  uint64_t         filelen   : 64; //64bit Size
  uint8_t          fileid[16];     //16byte File GUID
  uint64_t         len       : 64; //64bit File size
  uint64_t         date      : 64; //64bit Creation date
  uint64_t         pkt       : 64; //64bit Data packets count
  uint64_t         play      : 64; //64bit Play duration
  uint64_t         send      : 64; //64bit Send duration
  uint64_t         preroll   : 64; //64bit Preroll
  unsigned long    flags     : 32; //32bit Flags (Broadcast = 0x01 (LSB))
  unsigned long    minpkt    : 32; //32bit Minimum data packet size
  unsigned long    maxpkt    : 32; //32bit Maximum data packet size
  unsigned long    maxbit    : 32; //32bit Maximum bitrate
  //ASF Stream Properties
  uint8_t          stream[16];     //16byte ASFStreamProperties GUID B7DC0791-A9B7-11CF-8EE6-00C00C205365
  uint64_t         streamlen : 64; //64bit Size
  uint8_t          type[16];       //16byte Stream Type GUID F8699E40-5B4D-11CF-A8FD-00805F5C442B
  uint8_t          error[16];      //16byte Error Correction Type GUID
  uint64_t         timeoff   : 64; //64bit Time offset
  unsigned long    typelen   : 32; //32bit Type data length
  unsigned long    errorlen  : 32; //32bit Error correction data length
  unsigned int     streamnr  : 16; //16bit Flags (Stream Number = bit 0-7)
  unsigned long    reserved3 : 32; //32bit Reserved
  unsigned int     codec     : 16; //16bit Codec (0x0161 = WMA 7/8/9, 0x0162 = WMA 9 pro, 0x0163 = WMA 9 lossless)
  unsigned int     chn       : 16; //16bit Channels
  unsigned long    freq      : 32; //32bit Frequency / Samples Per Second
  unsigned long    bytes     : 32; //32bit Bytes per second
  unsigned int     align     : 16; //16bit Block alignment
  unsigned int     bits      : 16; //16bit Bits per sample
  unsigned int     optlen    : 16; //16bit Options size (WMA = 10)
  uint8_t          options[10];    //10byte Options (WMA = 10 bytes)
  //ASF Data Object
  uint8_t          data[16];       //16byte ASFDataObject GUID 75B22636-668E-11CF-A6D9-00AA0062CE6C
  uint64_t         datalen   : 64; //64bit Size
  uint8_t          dataid[16];     //16byte File GUID
  uint64_t         totalpkt  : 64; //64bit Total data packets
  unsigned int     reserved4 : 16; //16bit Reserved
} ASF_Header;*/

//ASF Data Packet
//0x82 0x00 0x00...

//----- GLOBALS -----
extern VSBUFFER vs_buf;


//----- PROTOTYPES -----
void                                   vs_requesthandler(void);
unsigned char                          vs_bufgetc(void);
void                                   vs_bufputs(const unsigned char *s, unsigned int len);
unsigned int                           vs_buffree(void);
unsigned int                           vs_buflen(void);
void                                   vs_bufsethead(unsigned int head);
void                                   vs_bufreset(void);

int                                    vs_gettreblefreq(void);
void                                   vs_settreblefreq(int freq); //1000 - 15000 Hz
int                                    vs_gettrebleamp(void);
void                                   vs_settrebleamp(int amp);   //  -8 -     7 dB
int                                    vs_getbassfreq(void);
void                                   vs_setbassfreq(int freq);   //  20 -   150 Hz
int                                    vs_getbassamp(void);
void                                   vs_setbassamp( int amp);    //   0 -    15 dB
int                                    vs_getvolume(void);
void                                   vs_setvolume(int vol);      //   0 -   100 %, 0=off
unsigned int                           vs_request(void);
void                                   vs_data(unsigned int c);
void                                   vs_write_bass(void);
void                                   vs_write_volume(void);
void                                   vs_write_plugin(const unsigned short *plugin, unsigned int len);
unsigned int                           vs_read_ram(unsigned int addr);
void                                   vs_write_reg(unsigned int reg, unsigned int data);
unsigned int                           vs_read_reg(unsigned int reg);
void                                   vs_pause(void);
void                                   vs_play(void);
void                                   vs_stopstream(void);
void                                   vs_stop(void);
void                                   vs_start(void);
void                                   vs_reset(void);
void                                   vs_init(void);


#endif //_VS_H_
