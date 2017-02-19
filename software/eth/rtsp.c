#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fatfs/ff.h"
#include "../debug.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../vs.h"
#include "../eth.h"
#include "../menu.h"
#include "../buffer.h"
#include "../station.h"
#include "utils.h"
#include "rtsp.h"


volatile unsigned int rtsp_status=RTSP_CLOSED;
unsigned int rtsp_localport=0;


void rtsp_close(void)
{
  long timeout;

  if(rtsp_status != RTSP_CLOSED)
  {
    rtsp_status = RTSP_CLOSE;

    timeout = getontime()+3; //3 seconds
    while(rtsp_status != RTSP_CLOSED)
    {
      eth_service();
      if(getdeltatime(timeout) > 0)
      {
        rtsp_status = RTSP_CLOSED;
        break;
      }
    }
  }

  return;
}


unsigned int rtsp_open(void)
{
  long timeout;
  unsigned int idx, trying, status;

  rtsp_status = RTSP_OPEN;

  //calc next local port
  switch(rtsp_localport)
  {
    case RTSP_CLIENTPORT1: rtsp_localport = RTSP_CLIENTPORT2; break;
    case RTSP_CLIENTPORT2: rtsp_localport = RTSP_CLIENTPORT3; break;
    default:               rtsp_localport = RTSP_CLIENTPORT1; break;
  }

  if(gbuf.station.port == 0)
  {
    gbuf.station.port = RTSP_SERVERPORT;
  }

  idx     = tcp_open(TCP_ENTRIES, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, rtsp_localport);
  timeout = getontime()+RTSP_TIMEOUT;
  trying  = RTSP_TRY;
  for(;;)
  {
    eth_service();

    status = rtsp_status;
    if((status == RTSP_CLOSED)     ||
       (status == RTSP_OPENED)     ||
       (status == RTSP_ADDRMOVED)  ||
       (status == RTSP_ERROR))
    {
      break;
    }
    if(keys_sw() || (ir_cmd() == SW_ENTER))
    {
        rtsp_status = RTSP_ERROR;
        tcp_abort(idx);
        break;
    }
    if(getdeltatime(timeout) > 0)
    {
      timeout = getontime()+RTSP_TIMEOUT;
      if(--trying)
      {
        rtsp_status = RTSP_OPEN;
        idx = tcp_open(idx, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, rtsp_localport);
      }
      else
      {
        rtsp_status = RTSP_ERRTIMEOUT;
        tcp_abort(idx);
        break;
      }
    }
  }

  if(rtsp_status == RTSP_OPENED)
  {
    return STATION_OPENED;
  }

  status = rtsp_status;
  rtsp_status = RTSP_CLOSED;
  switch(status)
  {
    case RTSP_ADDRMOVED:  return STATION_ADDRMOVED;  break;
    case RTSP_ERRTIMEOUT: return STATION_ERRTIMEOUT; break;
  }

  return STATION_ERROR;
}


void rtsp_putdata(const unsigned char *s, unsigned int len)
{
  unsigned int f;
  long timeout;

  timeout = getontime()+2;
  while(len)
  {
    buf_service();

    f = buf_free();
    if(f < len)
    {
      buf_puts(s, f);
      s   += f;
      len -= f;
      if(getdeltatime(timeout) > 0)
      {
        rtsp_close();
        break;
      }
    }
    else
    {
      buf_puts(s, len);
      break;
    }
  }

  return;
}


void rtsp_tcpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len, i;
  static char session[32]={0,};
  static unsigned int seq=0, content_len=0, pkt_len=0;
  const RTSP_Header *rtsp;

static long keepalive=0;

  switch(rtsp_status)
  {
    case RTSP_OPENED:
      tcp_send(idx, 0, 0); //send ack
      //save audio data
      while(rx_len)
      {
        
        if(pkt_len == 0)
        {
          //send keepalive
          if(getdeltatime(keepalive) > 0)
          {
            keepalive = getontime()+60;
            tx_len = sprintf((char*)tx, "GET_PARAMETER rtsp://%s%s/audio RTSP/1.0\r\n"
                                        "CSeq: %i\r\n"
                                        "Session: %s\r\n"
                                        "User-Agent: "APPNAME"\r\n"
                                        "\r\n",
                                        iptoa(gbuf.station.ip), gbuf.station.file, ++seq, session);
            tcp_send(idx, tx_len, 0);
          }

          //read rtsp header
          rtsp = (const RTSP_Header*)rx;
          if((rx_len >= (RTSP_HEADERLEN+RTP_HEADERLEN+RTPASF_HEADERLEN)) &&
             (rtsp->magic == 0x24))
          {
            pkt_len = swap16(rtsp->len) - RTP_HEADERLEN - RTPASF_HEADERLEN; //get data len
            //skip header
            rx     += RTSP_HEADERLEN + RTP_HEADERLEN + RTPASF_HEADERLEN;
            rx_len -= RTSP_HEADERLEN + RTP_HEADERLEN + RTPASF_HEADERLEN;
          }
          else
          {
            rx++;
            rx_len--;
          }
        }
        else
        {
          //save data
          if(rx_len > pkt_len)
          {
            rtsp_putdata(rx, pkt_len);
            rx     += pkt_len;
            rx_len -= pkt_len;
            pkt_len = 0;
          }
          else
          {
            rtsp_putdata(rx, rx_len);
            pkt_len -= rx_len;
            rx_len = 0;
            break;
          }
        }
      }
      break;

    case RTSP_PLAY:
      if(rx_len)
      {
        if(http_response((const char*)rx) == 200) //OK
        {
          rtsp_status = RTSP_OPENED;
          tcp_send(idx, 0, 0); //send ack
          DEBUGOUT("RTSP: stream ready\n");
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(idx);
        }
      }
      break;

    case RTSP_SETUP:
      if(rx_len)
      {
        if(http_response((const char*)rx) == 200) //OK
        {
          rtsp_status = RTSP_PLAY;

          //get session id
          http_hdparam(session, 32-1, (const char*)rx, "SESSION:");
          //cut off additional parameters
          for(i=0; isdigit(session[i]) && (i<(32-1)); i++);
          session[i] = 0;

          tx_len = sprintf((char*)tx, "PLAY rtsp://%s%s RTSP/1.0\r\n"
                                      "CSeq: %i\r\n"
                                      "Session: %s\r\n"
                                      "Range: npt=0.000-\r\n"
                                      "User-Agent: "APPNAME"\r\n"
                                      "\r\n",
                                      iptoa(gbuf.station.ip), gbuf.station.file, ++seq, session);
          tcp_send(idx, tx_len, 0);
          DEBUGOUT("RTSP: PLAY\n");
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(idx);
        }
      }
      break;

    case RTSP_DESCRIBE2:
      if(rx_len)
      {
        //save ASF header
        i = strlen((char*)vs_buf.b8);
        memcpy((vs_buf.b8+i), rx, rx_len);
        vs_buf.b8[i+rx_len] = 0;

        content_len -= rx_len;
        if(content_len == 0)
        {
          i = 0;
          if(strncmpi((char*)vs_buf.b8, "BASE32", 6) == 0)
          {
            //i = base32_decode(vs_buf.b8, (vs_buf.b8+6), VS_BUFSIZE-1);
          }
          else if(strncmpi((char*)vs_buf.b8, "BASE64", 6) == 0)
          {
            i = base64_decode(vs_buf.b8, (vs_buf.b8+6), VS_BUFSIZE-1);
          }

          if(i)
          {
            vs_bufsethead(i);
            DEBUGOUT("RTSP: ASF header: %i\n", i);

            rtsp_status = RTSP_SETUP;
            tx_len = sprintf((char*)tx, "SETUP rtsp://%s%s/audio RTSP/1.0\r\n"
                                        "CSeq: %i\r\n"
                                        "Transport: RTP/AVP/TCP;unicast;interleaved=0-1;mode=PLAY\r\n"
                                        "User-Agent: "APPNAME"\r\n"
                                        "\r\n",
                                        iptoa(gbuf.station.ip), gbuf.station.file, ++seq);
            tcp_send(idx, tx_len, 0);
            DEBUGOUT("RTSP: SETUP\n");
          }
          else
          {
            rtsp_status = RTSP_ERROR;
            tcp_abort(idx);
          }
        }
        else
        {
          tcp_send(idx, 0, 0);
        }
      }
      break;

    case RTSP_DESCRIBE1:
      if(rx_len)
      {
        if(http_response((const char*)rx) == 200) //OK
        {
          content_len = http_hdparamcontentlen((const char*)rx);
          if(content_len)
          {
            //search SDP start (skip http header)
            rx = (const unsigned char*)http_skiphd((const char*)rx, &rx_len);
            //get ASF header
            if(rx_len)
            {
              rtsp_status = RTSP_DESCRIBE2;
              content_len -= rx_len;
              vs_bufreset();
              http_hdparam((char*)vs_buf.b8, VS_BUFSIZE-1, (const char*)rx, "WMS-HDR.ASFV1;");
              DEBUGOUT("RTSP: get ASF header\n");
            }
          }
          tcp_send(idx, 0, 0);
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(idx);
        }
      }
      break;

    case RTSP_GET:
      if(rx_len)
      {
        i = http_response((const char*)rx);
        if(i == 200) //OK
        {
          menu_drawpopup("Station: OK");
          rtsp_status = RTSP_DESCRIBE1;
          tx_len = sprintf((char*)tx, "DESCRIBE rtsp://%s%s RTSP/1.0\r\n"
                                      "CSeq: %i\r\n"
                                      "User-Agent: "APPNAME"\r\n"
                                      "\r\n",
                                      iptoa(gbuf.station.ip), gbuf.station.file, ++seq);
          tcp_send(idx, tx_len, 0);
          DEBUGOUT("RTSP: DESCRIPE\n");
        }
        else
        {
          switch(i)
          {
            case 301: //301 Moved Permanently
            case 302: //302 Moved Temporarily
            case 303: //303 See Other
              menu_drawpopup("Station: Addr moved");
              break;
            case 500: //500 Internal Server Error
              menu_drawpopup("Station: Server error");
              break;
            default: //Error
              menu_drawpopup("Station: Error");
              break;
          }
          rtsp_status = RTSP_ERROR;
          tcp_abort(idx);
          delay_ms(1000); //for reading popup
        }
      }
      break;

    case RTSP_OPEN:
      memset(vs_buf.b8, 0, VS_BUFSIZE-1);
      pkt_len = 0;
      rtsp_status = RTSP_GET;
      station_setbitrate(0);
      menu_setformat(FORMAT_WMA);
      seq = 1;
      tx_len = sprintf((char*)tx, "GET rtsp://%s%s RTSP/1.0\r\n"
                                  "CSeq: %i\r\n"
                                  "User-Agent: "APPNAME"\r\n"
                                  "\r\n",
                                  iptoa(gbuf.station.ip), gbuf.station.file, seq);
      tcp_send(idx, tx_len, 0);
      DEBUGOUT("RTSP: GET\n");
      break;

    case RTSP_CLOSE:
      rtsp_status = RTSP_CLOSED;
      tx_len = sprintf((char*)tx, "TEARDOWN rtsp://%s%s RTSP/1.0\r\n"
                                  "CSeq: %i\r\n"
                                  "User-Agent: "APPNAME"\r\n"
                                  "\r\n",
                                  iptoa(gbuf.station.ip), gbuf.station.file, ++seq);
      tcp_send(idx, tx_len, 0);
      tcp_close(idx);
      DEBUGOUT("RTSP: TEARDOWN\n");
      break;

    case RTSP_CLOSED:
      break;
  }

  return;
}
