#ifndef _HTTP_H_
#define _HTTP_H_


//----- DEFINES -----
#define HTTP_SERVERPORT                (80)

#define HTTP_CLOSED                    (0)
#define HTTP_SEND                      (1)
#define HTTP_STATION                   (2)
#define HTTP_ALARM                     (3)
#define HTTP_SETTINGS                  (4)

#define HTTP_HTML_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=0\r\n" \
  "Pragma: no-cache\r\n" \
  "Content-Type: text/html\r\n" \
  "Content-Length: "

#define HTTP_XML_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=0\r\n" \
  "Pragma: no-cache\r\n" \
  "Content-Type: text/xml\r\n" \
  "Content-Length: "

#define HTTP_JS_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=86400\r\n" \
  "Content-Type: application/x-javascript\r\n" \
  "Content-Length: "

#define HTTP_CSS_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=86400\r\n" \
  "Content-Type: text/css\r\n" \
  "Content-Length: "

#define HTTP_TXT_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=0\r\n" \
  "Content-Type: text/plain\r\n" \
  "Content-Length: "

#define HTTP_ICON_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=86400\r\n" \
  "Content-Type: image/x-icon\r\n" \
  "Content-Length: "

#define HTTP_GIF_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=86400\r\n" \
  "Content-Type: image/gif\r\n" \
  "Content-Length: "

#define HTTP_JPEG_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "Cache-Control: no-store, no-cache, max-age=86400\r\n" \
  "Content-Type: image/jpeg\r\n" \
  "Content-Length: "

#define HTTP_200_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "\r\n"

#define HTTP_400_HEADER "" \
  "HTTP/1.0 400 Bad request\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "\r\n"

#define HTTP_404_HEADER "" \
  "HTTP/1.0 404 Not found\r\n" \
  "Server: "APPNAME"\r\n" \
  "Connection: Close\r\n" \
  "\r\n"


typedef struct
{
  unsigned int status;
  unsigned int file;
  unsigned int ftype;
  unsigned int flen;
  unsigned int fpos;
  unsigned int fparse;
  unsigned int fparam;
} HTTP_Table;


//----- PROTOTYPES -----
void                                   http_station(char *rx, unsigned int rx_len);
void                                   http_alarm(char *rx, unsigned int rx_len);
void                                   http_settings(char *rx, unsigned int rx_len);
unsigned int                           http_sendfile(unsigned int idx, const char *name, unsigned char *tx);
void                                   http_close(unsigned int idx);
void                                   http_tcpapp(unsigned int idx, char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_HTTP_H_
