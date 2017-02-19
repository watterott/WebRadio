#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../lcd.h"
#include "../mmc.h"
#include "../vs.h"
#include "../eth.h"
#include "../menu.h"
#include "../station.h"
#include "../share.h"
#include "../card.h"
#include "../alarm.h"
#include "../settings.h"
#include "utils.h"
#include "upnp.h"
#include "http.h"
#include "http_files.h"


const unsigned char INDEX_HTM[] =
{
  "<html>\r\n" \
  "<head>" \
  "<title>"APPNAME"</title>" \
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">" \
  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">" \
  "<script type=\"text/javascript\" src=\"/script.js\"></script>" \
  "</head>\r\n" \
  "<body>\r\n" \
  "<div id=\"container\">\r\n" \
  "  <div id=\"head\"><h1>"APPNAME"</h1><a href=\"/\">Home</a> | <a href=\"/station\">Station</a> | <a href=\"/alarm\">Alarm</a> | <a href=\"/settings\">Settings</a></div>\r\n" \
  "  <div id=\"content\">\r\n" \
/*
  "<form method=\"post\">\r\n" \
  "  <input type=\"submit\" name=\"minus\" id=\"minus\" value=\"-\">\r\n" \
  "  <input type=\"submit\" name=\"plus\"  id=\"plus\"  value=\"+\">\r\n" \
  "  <input type=\"submit\" name=\"enter\" id=\"enter\" value=\"Enter\">\r\n" \
  "</form>\r\n" \
  "$MENU" \
*/

  "<br><br>\r\n" \
  "<pre>\r\n" \
  "On Time: $ONTIME sec\r\n" \
  "Clock:   $CLOCK\r\n" \
  "Date:    $DATE\r\n" \
  "</pre>\r\n" \

  "<br><br>\r\n" \
  "<pre>\r\n" \
  "<b>"APPNAME" v"APPVERSION"</b> ("__DATE__" "__TIME__")\r\n" \
  "Hardware: "LM3S_NAME", "LCD_NAME"\r\n" \
  "visit <a href=\"http://www.watterott.net\">www.watterott.net</a> for updates\r\n" \
  "</pre>\r\n" \

  "</div>\r\n" \
  "</div>\r\n" \
  "<img src=\"/load.gif\" width=\"1\" height=\"1\" border=\"0\" style=\"visibility:hidden;display:none;\">\r\n" \
  "</body>\r\n" \
  "</html>\r\n"
};


const unsigned char STATION_HTM[] =
{
  "<html>\r\n" \
  "<head>" \
  "<title>Station - "APPNAME"</title>" \
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">" \
  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">" \
  "<script type=\"text/javascript\" src=\"/script.js\"></script>" \
  "</head>\r\n" \
  "<body>\r\n" \
  "<div id=\"container\">\r\n" \
  "<div id=\"head\"><h1>"APPNAME"</h1><a href=\"/\">Home</a> | <a href=\"/station\">Station</a> | <a href=\"/alarm\">Alarm</a> | <a href=\"/settings\">Settings</a></div>\r\n" \
  "<div id=\"content\">\r\n" \
  "<table width=\"100%\" border=\"0\" cellpadding=\"4\"><tr><td valign=\"top\" align=\"center\">\r\n" \
  "<form name=\"list\" id=\"list\">\r\n" \
  "  <select name=\"station\" id=\"station\" size=\"15\" onClick=\"getStation(document.list.station.selectedIndex+1);\"></select>\r\n" \
  "</form>\r\n" \
  "<form name=\"move\" id=\"move\" method=\"post\">\r\n" \
  "  <input type=\"submit\" name=\"up\"   id=\"up\"   value=\"Up\">\r\n" \
  "  <input type=\"submit\" name=\"down\" id=\"down\" value=\"Down\">\r\n" \
  "  <input type=\"submit\" name=\"del\"  id=\"del\"  value=\"Del\"><br>\r\n" \
  "  <input type=\"submit\" name=\"play\" id=\"play\" value=\"Play\">\r\n" \
  "  <input type=\"submit\" name=\"stop\" id=\"stop\" value=\"Stop\">\r\n" \
  "  <input type=\"hidden\" name=\"item\" id=\"item\" value=\"\">\r\n" \
  "</form>\r\n" \
  "</td><td valign=\"top\">\r\n" \
  "<pre><b>Edit Station</b> <form name=\"edit\" id=\"edit\" method=\"post\">\r\n" \
  "Name    <input type=\"text\"   name=\"name\" id=\"name\" value=\"\" size=\"40\">\r\n" \
  "Address <input type=\"text\"   name=\"addr\" id=\"addr\" value=\"\" size=\"40\">\r\n" \
  "        <input type=\"submit\" name=\"save\" id=\"save\" value=\"Save\">\r\n" \
  "        <input type=\"hidden\" name=\"item\" id=\"item\" value=\"\"></form>\r\n" \
  "<b>Add Station</b>       <form name=\"add\"  id=\"add\"  method=\"post\">\r\n" \
  "Name    <input type=\"text\"   name=\"name\" id=\"name\" value=\"\" size=\"40\">\r\n" \
  "Address <input type=\"text\"   name=\"addr\" id=\"addr\" value=\"\" size=\"40\">\r\n" \
  "        <input type=\"submit\" name=\"add\"  id=\"add\"  value=\"Add\"></form></pre>\r\n" \
  "</td></tr></table>\r\n" \
  "<script type=\"text/javascript\">\r\n" \
  "  window.onload = getStationList;\r\n" \
  "</script>\r\n" \
  "</div>\r\n" \
  "</div>\r\n" \
  "<img src=\"/load.gif\" width=\"1\" height=\"1\" border=\"0\" style=\"visibility:hidden;display:none;\">\r\n" \
  "</body>\r\n" \
  "</html>\r\n"
};


const unsigned char STATIONNAME_TXT[] =
{
  "$STATIONNAME"
};


const unsigned char STATIONADDR_TXT[] =
{
  "$STATIONADDR"
};


const unsigned char ALARM_HTM[] =
{
  "<html>\r\n" \
  "<head>" \
  "<title>Alarm - "APPNAME"</title>" \
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">" \
  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">" \
  "<script type=\"text/javascript\" src=\"/script.js\"></script>" \
  "</head>\r\n" \
  "<body>\r\n" \
  "<div id=\"container\">\r\n" \
  "<div id=\"head\"><h1>"APPNAME"</h1><a href=\"/\">Home</a> | <a href=\"/station\">Station</a> | <a href=\"/alarm\">Alarm</a> | <a href=\"/settings\">Settings</a></div>\r\n" \
  "<div id=\"content\">\r\n" \
  "<table width=\"100%\" border=\"0\" cellpadding=\"4\"><tr><td valign=\"top\">\r\n" \
  "<form name=\"list\" id=\"list\">\r\n" \
  "  <select name=\"alarm\" id=\"alarm\" size=\"10\" onClick=\"getAlarm(document.list.alarm.selectedIndex+1);\"></select>\r\n" \
  "</form>\r\n" \
  "</td><td valign=\"top\">\r\n" \
  "<pre><b>Edit Alarm</b><form name=\"edit\" id=\"edit\" method=\"post\">\r\n" \
  "Time <input type=\"text\"   name=\"time\" id=\"time\" value=\"\" size=\"40\">\r\n" \
  "     <input type=\"submit\" name=\"save\" id=\"save\" value=\"Save\">\r\n" \
  "     <input type=\"hidden\" name=\"item\" id=\"item\" value=\"\"></form>\r\n" \
  "      !     -> Alarm is off / inactive\r\n" \
  "      -     -> Alarm: go into Standby\r\n" \
  "  all other -> Alarm: play the Alarm file\r\n" \
  "\r\n" \
  "  Mo=Monday, Tu=Tuesday, We=Wednesday, Th=Thursday,\r\n" \
  "  Fr=Friday, Sa=Saturday, Su=Sunday\r\n" \
  "</pre>\r\n" \
  "</td></tr></table>\r\n" \
  "<script type=\"text/javascript\">\r\n" \
  "  window.onload = getAlarmList;\r\n" \
  "</script>\r\n" \
  "</div>\r\n" \
  "</div>\r\n" \
  "<img src=\"/load.gif\" width=\"1\" height=\"1\" border=\"0\" style=\"visibility:hidden;display:none;\">\r\n" \
  "</body>\r\n" \
  "</html>\r\n"
};


const unsigned char ALARMTIME_TXT[] =
{
  "$ALARMTIME"
};


const unsigned char SETTINGS_HTM[] =
{
  "<html>\r\n" \
  "<head>" \
  "<title>Settings - "APPNAME"</title>" \
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">" \
  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">" \
  "<script type=\"text/javascript\" src=\"/script.js\"></script>" \
  "</head>\r\n" \
  "<body>\r\n" \
  "<div id=\"container\">\r\n" \
  "<div id=\"head\"><h1>"APPNAME"</h1><a href=\"/\">Home</a> | <a href=\"/station\">Station</a> | <a href=\"/alarm\">Alarm</a> | <a href=\"/settings\">Settings</a></div>\r\n" \
  "<div id=\"content\">\r\n" \
  "<table width=\"100%\" border=\"0\" cellpadding=\"4\"><tr><td valign=\"top\"><pre>" \
  "<b>General</b><form method=\"post\">\r\n" \
  "Play Mode   <input type=\"text\" name=\"playmode\"  id=\"playmode\"    value=\"$PLAYMODE\">\r\n" \
  "Volume      <input type=\"text\" name=\"volume\"    id=\"volume\"      value=\"$VOLUME\">\r\n" \
  "AutoStart   <input type=\"text\" name=\"autostart\" id=\"autostart\"   value=\"$AUTOSTART\">\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "<b>Alarm</b><form method=\"post\">\r\n" \
  "Volume      <input type=\"text\" name=\"alarmvol\"   id=\"alarmvol\"   value=\"$ALARMVOL\">\r\n" \
  "File1       <input type=\"text\" name=\"alarmfile1\" id=\"alarmfile1\" value=\"$ALARMFILE1\">\r\n" \
  "File2       <input type=\"text\" name=\"alarmfile2\" id=\"alarmfile2\" value=\"$ALARMFILE2\">\r\n" \
  "File3       <input type=\"text\" name=\"alarmfile3\" id=\"alarmfile3\" value=\"$ALARMFILE3\">\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "<b>Audio</b><form method=\"post\">\r\n" \
  "Bass-Freq   <input type=\"text\" name=\"bassfreq\"   id=\"bassfreq\"   value=\"$BASSFREQ\"> Hz\r\n" \
  "Bass-Amp    <input type=\"text\" name=\"bassamp\"    id=\"bassamp\"    value=\"$BASSAMP\"> dB\r\n" \
  "Treble-Freq <input type=\"text\" name=\"treblefreq\" id=\"treblefreq\" value=\"$TREBLEFREQ\"> Hz\r\n" \
  "Treble-Amp  <input type=\"text\" name=\"trebleamp\"  id=\"trebleamp\"  value=\"$TREBLEAMP\"> dB\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "<b>Colors</b><form method=\"post\">\r\n" \
  "BG          <input type=\"text\" name=\"colorbg\"    id=\"colorbg\"    value=\"$COLORBG\">\r\n" \
  "FG          <input type=\"text\" name=\"colorfg\"    id=\"colorfg\"    value=\"$COLORFG\">\r\n" \
  "Sel         <input type=\"text\" name=\"colorsel\"   id=\"colorsel\"   value=\"$COLORSEL\">\r\n" \
  "Edge        <input type=\"text\" name=\"coloredge\"  id=\"coloredge\"  value=\"$COLOREDGE\">\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "</pre></td><td valign=\"top\"><pre>" \
  "<b>Ethernet</b><form method=\"post\">\r\n" \
  "Name        <input type=\"text\" name=\"name\"       id=\"name\"       value=\"$NAME\">\r\n" \
  "MAC         <input type=\"text\" name=\"mac\"        id=\"mac\"        value=\"$MAC\"> *\r\n" \
  "DHCP        <input type=\"text\" name=\"dhcp\"       id=\"dhcp\"       value=\"$DHCP\">\r\n" \
  "IP          <input type=\"text\" name=\"ip\"         id=\"ip\"         value=\"$IP\">\r\n" \
  "Mask        <input type=\"text\" name=\"netmask\"    id=\"netmask\"    value=\"$NETMASK\">\r\n" \
  "Router      <input type=\"text\" name=\"router\"     id=\"router\"     value=\"$ROUTER\">\r\n" \
  "DNS         <input type=\"text\" name=\"dns\"        id=\"dns\"        value=\"$DNS\">\r\n" \
  "NTP         <input type=\"text\" name=\"ntp\"        id=\"ntp\"        value=\"$NTP\">\r\n" \
  "TimeDiff    <input type=\"text\" name=\"timediff\"   id=\"timediff\"   value=\"$TIMEDIFF\">\r\n" \
  "Summer      <input type=\"text\" name=\"summer\"     id=\"summer\"     value=\"$SUMMER\">\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "<b>IR</b><form method=\"post\">\r\n" \
  "IR Addr     <input type=\"text\" name=\"iraddr\"     id=\"iraddr\"     value=\"$IRADDR\">\r\n" \
  "IR Key Power<input type=\"text\" name=\"irkeypower\" id=\"irkeypower\" value=\"$IRKEYPOWER\">\r\n" \
  "IR Key Up   <input type=\"text\" name=\"irkeyup\"    id=\"irkeyup\"    value=\"$IRKEYUP\">\r\n" \
  "IR Key Down <input type=\"text\" name=\"irkeydown\"  id=\"irkeydown\"  value=\"$IRKEYDOWN\">\r\n" \
  "IR Key OK   <input type=\"text\" name=\"irkeyok\"    id=\"irkeyok\"    value=\"$IRKEYOK\">\r\n" \
  "IR Key Vol+ <input type=\"text\" name=\"irkeyvolp\"  id=\"irkeyvolp\"  value=\"$IRKEYVOLP\">\r\n" \
  "IR Key Vol- <input type=\"text\" name=\"irkeyvolm\"  id=\"irkeyvolm\"  value=\"$IRKEYVOLM\">\r\n" \
  "<input type=\"reset\" name=\"reset\" value=\"Reset\"> <input type=\"submit\" name=\"save\" value=\"Save\"></form>\r\n" \
  "\r\n* Restart required\r\n" \
  "\r\n\r\n<form method=\"post\"><input type=\"submit\" name=\"restartwebradio\" id=\"restartwebradio\" value=\"Restart "APPNAME"\"></form>\r\n" \
  "</pre></td></tr></table>\r\n" \
  "</div>\r\n" \
  "</div>\r\n" \
  "<img src=\"/load.gif\" width=\"1\" height=\"1\" border=\"0\" style=\"visibility:hidden;display:none;\">\r\n" \
  "</body>\r\n" \
  "</html>\r\n"
};


const unsigned char SCRIPT_JS[] =
{
  "function getWndWidth()\r\n" \
  "{\r\n" \
  "  if(window.innerWidth) //all except IE\r\n" \
  "  {\r\n" \
  "    return window.innerWidth;\r\n" \
  "  }\r\n" \
  "  else if(document.documentElement && document.documentElement.clientWidth) //IE6+ Strict Mode\r\n" \
  "  {\r\n" \
  "    return document.documentElement.clientWidth;\r\n" \
  "  }\r\n" \
  "  else\r\n" \
  "  {\r\n" \
  "    return document.body.clientWidth;\r\n" \
  "  }\r\n" \
  "}\r\n" \
  "\r\n" \
  "function getWndHeight()\r\n" \
  "{\r\n" \
  "  if(window.innerHeight) //all except IE\r\n" \
  "  {\r\n" \
  "    return window.innerHeight;\r\n" \
  "  }\r\n" \
  "  else if(document.documentElement && document.documentElement.clientHeight) //IE6+ Strict Mode\r\n" \
  "  {\r\n" \
  "    return document.documentElement.clientHeight;\r\n" \
  "  }\r\n" \
  "  else\r\n" \
  "  {\r\n" \
  "    return document.body.clientHeight;\r\n" \
  "  }\r\n" \
  "}\r\n" \
  "\r\n" \
  "function hideLoadImg()\r\n" \
  "{\r\n" \
  "  var loadImg = document.getElementById('loadImg');\r\n" \
  "  if(loadImg)\r\n" \
  "  {\r\n" \
  "    loadImg.style.display    = 'none';\r\n" \
  "    loadImg.style.visibility = 'hidden';\r\n" \
  "  }\r\n" \
  "  return;\r\n" \
  "}\r\n" \
  "\r\n" \
  "function showLoadImg()\r\n" \
  "{\r\n" \
  "  var loadImg = document.getElementById('loadImg');\r\n" \
  "  if(!loadImg)\r\n" \
  "  {\r\n" \
  "    var parent = document.getElementById('head');\r\n" \
  "    loadImg = document.createElement('img');\r\n" \
  "    parent.appendChild(loadImg);\r\n" \
  "    loadImg.setAttribute('id', 'loadImg');\r\n" \
  "    loadImg.src              = '/load.gif';\r\n" \
  "    loadImg.style.border     = '0px none';\r\n" \
  "    loadImg.style.position   = 'absolute';\r\n" \
  "  }\r\n" \
  "  loadImg.style.left       = ((getWndWidth()-loadImg.width)/2) + 'px';\r\n" \
  "  loadImg.style.top        = ((getWndHeight()-loadImg.height)/2) + 'px';\r\n" \
  "  loadImg.style.display    = 'block';\r\n" \
  "  loadImg.style.visibility = 'visible';\r\n" \
  "  return;\r\n" \
  "}\r\n" \
  "\r\n" \
  "function httpRequest(url)\r\n" \
  "{\r\n" \
  "  var value   = null;\r\n" \
  "  var request = null;\r\n" \
  "  if(window.XMLHttpRequest) //Standard\r\n" \
  "  {\r\n" \
  "    request = new XMLHttpRequest();\r\n" \
  "  }\r\n" \
  "  else if (window.ActiveXObject) //IE\r\n" \
  "  {\r\n" \
  "    request = new ActiveXObject('Msxml2.XMLHTTP');\r\n" \
  "    if(!request)\r\n" \
  "    {\r\n" \
  "      request = new ActiveXObject('Microsoft.XMLHTTP');\r\n" \
  "    }\r\n" \
  "  }\r\n" \
  "  if(request)\r\n" \
  "  {\r\n" \
  "    request.open('GET', url, false);\r\n" \
  "    request.send(null);\r\n" \
  "    if((request.readyState == 4) && (request.status == 200))\r\n" \
  "    {\r\n" \
  "      value = request.responseText;\r\n" \
  "    }\r\n" \
  "  }\r\n" \
  "  return value;\r\n" \
  "}\r\n" \
  "\r\n" \
  "function getStation(item)\r\n" \
  "{\r\n" \
  "  document.edit.name.value = httpRequest('/stationname?'+item);\r\n" \
  "  document.edit.addr.value = httpRequest('/stationaddr?'+item);\r\n" \
  "  document.edit.item.value = item;\r\n" \
  "  document.move.item.value = item;\r\n" \
  "  return;\r\n" \
  "}\r\n" \
  "\r\n" \
  "function getStationList()\r\n" \
  "{\r\n" \
  "  var item, value;\r\n" \
  "  showLoadImg();\r\n" \
  "  for(item=1; item<=99; item++)\r\n" \
  "  {\r\n" \
  "    value = httpRequest('/stationname?'+item);\r\n" \
  "    if(value == null)\r\n" \
  "    {\r\n" \
  "      break;\r\n" \
  "    }\r\n" \
  "    else\r\n" \
  "    {\r\n" \
  "      var opt = new Option(value, item);\r\n" \
  "      document.list.station.options[item-1] = opt;\r\n" \
  "    }\r\n" \
  "  }\r\n" \
  "  hideLoadImg();\r\n" \
  "  return;\r\n" \
  "}\r\n" \
  "\r\n" \
  "function getAlarm(item)\r\n" \
  "{\r\n" \
  "  document.edit.time.value = httpRequest('/alarmtime?'+item);\r\n" \
  "  document.edit.item.value = item;\r\n" \
  "  return;\r\n" \
  "}\r\n"
  "\r\n" \
  "function getAlarmList()\r\n" \
  "{\r\n" \
  "  var item, value;\r\n" \
  "  showLoadImg();\r\n" \
  "  for(item=1; item<=10; item++)\r\n" \
  "  {\r\n" \
  "    value = httpRequest('/alarmtime?'+item);\r\n" \
  "    if(value == null)\r\n" \
  "    {\r\n" \
  "      break;\r\n" \
  "    }\r\n" \
  "    else\r\n" \
  "    {\r\n" \
  "      var opt = new Option(value, item);\r\n" \
  "      document.list.alarm.options[item-1] = opt;\r\n" \
  "    }\r\n" \
  "  }\r\n" \
  "  hideLoadImg();\r\n" \
  "  return;\r\n" \
  "}\r\n" \
};


const unsigned char STYLE_CSS[] =
{
  "html, body {\r\n" \
  "  border:0px none;\r\n" \
  "  width:100%;\r\n" \
  "  height:100%;\r\n" \
  "  padding:0px;\r\n" \
  "  margin:0px;\r\n" \
  "  background-color:#F0F2F4;\r\n" \
  "  color:#000000;\r\n" \
  "}\r\n" \
  "body {\r\n" \
  "  text-align:center;\r\n" \
  "  font-family:arial, helvetica, sans-serif;\r\n" \
  "  font-size:0.9em;\r\n" \
  "}\r\n" \
  "pre, ul, li, table, td, th {\r\n" \
  "  font-size:100%;\r\n" \
  "}\r\n" \
  "h1 {\r\n" \
  "  font-family:helvetica, arial, sans-serif;\r\n" \
  "  font-size:120%;\r\n" \
  "  font-weight:bold;\r\n" \
  "}\r\n" \
  "h2 {\r\n" \
  "  font-size:120%;\r\n" \
  "  font-weight:bold;\r\n" \
  "}\r\n" \
  "h3 {\r\n" \
  "  font-size:105%;\r\n" \
  "  font-weight:bold;\r\n" \
  "}\r\n" \
  "h4 {\r\n" \
  "  font-size:102%;\r\n" \
  "  font-weight:normal;\r\n" \
  "}\r\n" \
  "a, a:link {\r\n" \
  "  color:#0000BB;\r\n" \
  "  font-size:100%;\r\n" \
  "  text-decoration:none;\r\n" \
  "}\r\n" \
  "a:hover {\r\n" \
  "  color:#F00000;\r\n" \
  "  text-decoration:underline;\r\n" \
  "}\r\n" \
  "#container {\r\n" \
  "  width:600px;\r\n" \
  "  padding:0px;\r\n" \
  "  margin-left:auto;\r\n" \
  "  margin-top:0px;\r\n" \
  "  margin-right:auto;\r\n" \
  "  margin-bottom:0px;\r\n" \
  "}\r\n" \
  "#head {\r\n" \
  "  border-bottom:2px solid #808080;\r\n" \
  "  height:60px;\r\n" \
  "  padding:10px;\r\n" \
  "  margin:0px;\r\n" \
  "  vertical-align:middle;\r\n" \
  "  text-align:center;\r\n" \
  "  color:#808080;\r\n" \
  "}\r\n" \
  "#content {\r\n" \
  "  padding:20px;\r\n" \
  "  margin:0px;\r\n" \
  "  vertical-align:top;\r\n" \
  "  text-align:left;\r\n" \
  "}\r\n"
};


const unsigned char FAVICON_ICO[] =
{
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x61, 0xFE,
  0x00, 0x00, 0x4C, 0xAA, 0x00, 0x00, 0x52, 0xFE, 0x00, 0x00, 0x52, 0xC6,
  0x00, 0x00, 0x4C, 0xBA, 0x00, 0x00, 0x61, 0xC6, 0x00, 0x00, 0x7F, 0xFE,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xEF, 0xF7,
  0x00, 0x00, 0xF0, 0x07, 0x00, 0x00, 0xFF, 0xF3, 0x00, 0x00, 0xFF, 0xFB,
  0x00, 0x00, 0xFF, 0xF9, 0x00, 0x00
};


const unsigned char LOAD_GIF[] =
{
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x2B, 0x00, 0x0B, 0x00, 0x91, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x82, 0x82, 0x82, 0x00, 0x00,
  0x00, 0x21, 0xFF, 0x0B, 0x4E, 0x45, 0x54, 0x53, 0x43, 0x41, 0x50, 0x45,
  0x32, 0x2E, 0x30, 0x03, 0x01, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04, 0x20,
  0x0A, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x0B,
  0x00, 0x00, 0x08, 0x50, 0x00, 0x01, 0x08, 0x18, 0x48, 0x90, 0x20, 0x80,
  0x83, 0x08, 0x05, 0x16, 0x2C, 0x98, 0x10, 0xE1, 0x42, 0x86, 0x0F, 0x17,
  0x36, 0x8C, 0xC8, 0x30, 0x21, 0xC5, 0x8B, 0x12, 0x2D, 0x62, 0x9C, 0x88,
  0xB1, 0x23, 0xC7, 0x8B, 0x1F, 0x3B, 0x52, 0x0C, 0x19, 0x91, 0xA4, 0xC8,
  0x8C, 0x0E, 0x37, 0x6A, 0x3C, 0x59, 0x72, 0xE5, 0x48, 0x97, 0x2C, 0x2B,
  0xA6, 0x04, 0x09, 0x33, 0xE6, 0x40, 0x93, 0x28, 0x0F, 0xDA, 0x7C, 0x88,
  0x53, 0xA6, 0xCE, 0x8D, 0x34, 0x7B, 0x0A, 0x68, 0xF8, 0xB3, 0x64, 0x40,
  0x00, 0x21, 0xF9, 0x04, 0x20, 0x0A, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00,
  0x00, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x00, 0x08, 0x19, 0x00, 0x01, 0x0C,
  0x18, 0x48, 0x90, 0xA0, 0xC0, 0x82, 0x08, 0x13, 0x2A, 0x5C, 0xC8, 0xB0,
  0xA1, 0xC3, 0x87, 0x10, 0x21, 0x1E, 0x54, 0x08, 0x20, 0x20, 0x00, 0x21,
  0xF9, 0x04, 0x20, 0x0A, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00,
  0x1B, 0x00, 0x0B, 0x00, 0x00, 0x08, 0x48, 0x00, 0x01, 0x08, 0x18, 0x48,
  0x90, 0x20, 0x80, 0x83, 0x08, 0x01, 0x0C, 0x58, 0xC8, 0x90, 0xA1, 0xC0,
  0x82, 0x10, 0x05, 0x24, 0x54, 0xD8, 0xB0, 0xE2, 0x80, 0x88, 0x10, 0x27,
  0x5A, 0xAC, 0x88, 0xB1, 0xA0, 0xC6, 0x8D, 0x0C, 0x3B, 0x1A, 0x4C, 0x08,
  0x32, 0xA4, 0x48, 0x89, 0x24, 0x4B, 0x5E, 0x3C, 0xF9, 0xB1, 0xE4, 0x49,
  0x94, 0x08, 0x55, 0xAE, 0x14, 0xD9, 0x12, 0xE4, 0xCB, 0x9A, 0x1B, 0x1F,
  0x62, 0x9C, 0x78, 0xB0, 0x24, 0x80, 0x80, 0x00, 0x21, 0xF9, 0x04, 0x20,
  0x0A, 0x00, 0x00, 0x00, 0x2C, 0x10, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x0B,
  0x00, 0x00, 0x08, 0x48, 0x00, 0x01, 0x08, 0x18, 0x48, 0x90, 0x20, 0x80,
  0x83, 0x08, 0x01, 0x0C, 0x58, 0xC8, 0x90, 0xA1, 0xC0, 0x82, 0x10, 0x05,
  0x24, 0x54, 0xD8, 0xB0, 0xE2, 0x80, 0x88, 0x10, 0x27, 0x5A, 0xAC, 0x88,
  0xB1, 0xA0, 0xC6, 0x8D, 0x0C, 0x3B, 0x1A, 0x4C, 0x08, 0x32, 0xA4, 0x48,
  0x89, 0x24, 0x4B, 0x5E, 0x3C, 0xF9, 0xB1, 0xE4, 0x49, 0x94, 0x08, 0x55,
  0xAE, 0x14, 0xD9, 0x12, 0xE4, 0xCB, 0x9A, 0x1B, 0x1F, 0x62, 0x9C, 0x78,
  0xB0, 0x24, 0x80, 0x80, 0x00, 0x3B
};


const unsigned char DEVICE_XML[] =
{
  "<?xml version=\"1.0\"?>\r\n" \
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n" \
  "  <specVersion>\r\n" \
  "    <major>1</major>\r\n" \
  "    <minor>0</minor>\r\n" \
  "  </specVersion>\r\n" \
  "  <URLBase>http://$IP:$UPNP_PORT</URLBase>\r\n" \
  "  <device>\r\n" \
  "    <deviceType>urn:schemas-upnp-org:device:"APPNAME":1</deviceType>\r\n" \
  "    <friendlyName>$NAME</friendlyName>\r\n" \
  "    <modelName>"APPNAME" v"APPVERSION"</modelName>\r\n" \
  "    <UDN>uuid:$UPNP_UUID</UDN>\r\n" \
  "    <serviceList>\r\n" \
  "      <service>\r\n" \
  "        <serviceType>urn:schemas-upnp-org:service:REMOTE:1</serviceType>\r\n" \
  "        <serviceId>urn:upnp-org:serviceId:REMOTE</serviceId>\r\n" \
  "        <SCPDURL>/service.xml</SCPDURL>\r\n" \
  "        <controlURL>/control</controlURL>\r\n" \
  "        <eventSubURL>/event</eventSubURL>\r\n" \
  "      </service>\r\n" \
  "    </serviceList>\r\n" \
  "  </device>\r\n" \
  "</root>\r\n"
};


const unsigned char SERVICE_XML[] =
{
  "<?xml version=\"1.0\"?>\r\n" \
  "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n" \
  "  <specVersion>\r\n" \
  "    <major>1</major>\r\n" \
  "    <minor>0</minor>\r\n" \
  "  </specVersion>\r\n" \
  "  <actionList>\r\n" \
  "    <action>\r\n" \
  "      <name>SETVOLUME</name>\r\n" \
  "      <argumentList>\r\n" \
  "        <argument>\r\n" \
  "          <name>VOLUME</name>\r\n" \
  "          <relatedStateVariable>VOLUME</relatedStateVariable>\r\n" \
  "          <direction>in</direction>\r\n" \
  "        </argument>\r\n" \
  "     </argumentList>\r\n" \
  "    </action>\r\n" \
  "    <action>\r\n" \
  "      <name>GETVOLUME</name>\r\n" \
  "      <argumentList>\r\n" \
  "         <argument>\r\n" \
  "          <name>VOLUME</name>\r\n" \
  "          <relatedStateVariable>VOLUME</relatedStateVariable>\r\n" \
  "          <direction>out</direction>\r\n" \
  "        </argument>\r\n" \
  "       </argumentList>\r\n" \
  "    </action>\r\n" \
  "  </actionList>\r\n" \
  "  <serviceStateTable>\r\n" \
  "    <stateVariable sendEvents=\"no\">\r\n" \
  "      <name>VOLUME</name>\r\n" \
  "      <dataType>ui1</dataType>\r\n" \
  "      <allowedValueRange>\r\n" \
  "        <minimum>0</minimum>\r\n" \
  "        <maximum>100</maximum>\r\n" \
  "      </allowedValueRange>\r\n" \
  "    </stateVariable>\r\n" \
  "  </serviceStateTable>\r\n" \
  "</scpd>\r\n"
};


#define HTTPFILES (14)
const HTTPFILE httpfiles[HTTPFILES] = 
{
  {"/",            HTML_FILE, INDEX_HTM,       sizeof(INDEX_HTM)-1},
  {"/home",        HTML_FILE, INDEX_HTM,       sizeof(INDEX_HTM)-1},
  {"/station",     HTML_FILE, STATION_HTM,     sizeof(STATION_HTM)-1},
  {"/stationname", HTML_FILE, STATIONNAME_TXT, sizeof(STATIONNAME_TXT)-1},
  {"/stationaddr", HTML_FILE, STATIONADDR_TXT, sizeof(STATIONADDR_TXT)-1},
  {"/alarm",       HTML_FILE, ALARM_HTM,       sizeof(ALARM_HTM)-1},
  {"/alarmtime",   HTML_FILE, ALARMTIME_TXT,   sizeof(ALARMTIME_TXT)-1},
  {"/settings",    HTML_FILE, SETTINGS_HTM,    sizeof(SETTINGS_HTM)-1},
  {"/script.js",   JS_FILE,   SCRIPT_JS,       sizeof(SCRIPT_JS)-1},
  {"/style.css",   CSS_FILE,  STYLE_CSS,       sizeof(STYLE_CSS)-1},
  {"/favicon.ico", ICON_FILE, FAVICON_ICO,     sizeof(FAVICON_ICO)},
  {"/load.gif",    GIF_FILE,  LOAD_GIF,        sizeof(LOAD_GIF)},
  {"/device.xml",  XML_FILE,  DEVICE_XML,      sizeof(DEVICE_XML)-1},
  {"/service.xml", XML_FILE,  SERVICE_XML,     sizeof(SERVICE_XML)-1}
};


#define VAR_NONE (0)
#define VAR_NR   (1)
#define VAR_STR  (2)
#define VAR_INI  (3)
#define VAR_MAC  (4)
#define VAR_IP   (5)
#define VAR_RGB  (6)
#define VAR_MEN  (7)
#define VAR_STN  (8)
#define VAR_STA  (9)
#define VAR_ALA  (10)
#define HTTPVARS (41)
const HTTPVAR httpvars[HTTPVARS] =
{
  {"ONTIME",      VAR_NR,  (void*)getontime},
  {"DATE",        VAR_STR, (void*)getdate},
  {"CLOCK",       VAR_STR, (void*)getclock},

  {"MENU",        VAR_MEN, 0},
  {"STATIONNAME", VAR_STN, 0},
  {"STATIONADDR", VAR_STA, 0},
  {"ALARMTIME",   VAR_ALA, 0},

  {"VOLUME",      VAR_NR,  (void*)vs_getvolume},
  {"PLAYMODE",    VAR_INI, 0},
  {"AUTOSTART",   VAR_INI, 0},

  {"ALARMVOL",    VAR_NR,  (void*)alarm_getvol},
  {"ALARMFILE1",  VAR_INI, 0},
  {"ALARMFILE2",  VAR_INI, 0},
  {"ALARMFILE3",  VAR_INI, 0},

  {"BASSFREQ",    VAR_NR,  (void*)vs_getbassfreq},
  {"BASSAMP",     VAR_NR,  (void*)vs_getbassamp},
  {"TREBLEFREQ",  VAR_NR,  (void*)vs_gettreblefreq},
  {"TREBLEAMP",   VAR_NR,  (void*)vs_gettrebleamp},

  {"NAME",        VAR_STR, (void*)eth_getname},
  {"MAC",         VAR_MAC, (void*)eth_getmac},
  {"DHCP",        VAR_NR,  (void*)eth_getdhcp},
  {"IP",          VAR_IP,  (void*)eth_getip},
  {"NETMASK",     VAR_IP,  (void*)eth_getnetmask},
  {"ROUTER",      VAR_IP,  (void*)eth_getrouter},
  {"DNS",         VAR_IP,  (void*)eth_getdns},
  {"NTP",         VAR_IP,  (void*)eth_getntp},
  {"TIMEDIFF",    VAR_NR,  (void*)eth_gettimediff},
  {"SUMMER",      VAR_NR,  (void*)eth_getsummer},
  {"UPNP_UUID",   VAR_STR, (void*)upnp_getuuid},
  {"UPNP_PORT",   VAR_NR,  (void*)upnp_getport},

  {"IRADDR",      VAR_NR,  (void*)ir_getaddr},
  {"IRKEYPOWER",  VAR_NR,  (void*)ir_getkeypower},
  {"IRKEYUP",     VAR_NR,  (void*)ir_getkeyup},
  {"IRKEYDOWN",   VAR_NR,  (void*)ir_getkeydown},
  {"IRKEYOK",     VAR_NR,  (void*)ir_getkeyok},
  {"IRKEYVOLP",   VAR_NR,  (void*)ir_getkeyvolp},
  {"IRKEYVOLM",   VAR_NR,  (void*)ir_getkeyvolm},

  {"COLORBG",     VAR_RGB, (void*)menu_getbgcolor},
  {"COLORFG",     VAR_RGB, (void*)menu_getfgcolor},
  {"COLORSEL",    VAR_RGB, (void*)menu_getselcolor},
  {"COLOREDGE",   VAR_RGB, (void*)menu_getedgecolor},
};


unsigned int http_printf(char *dst, unsigned int format, unsigned int param, ...)
{
  unsigned int len=0;
  long i;
  char *ptr, buf[MAX_ADDR]={0,};

  if(dst)
  {
    *dst = 0;
  }

  va_list ap;

  va_start(ap, param);

  switch(format)
  {
    case VAR_NR:
      itoa(va_arg(ap, long), buf, 10);
      len = strlen(buf);
      if(dst)
      {
        strcpy(dst, buf);
      }
      break;

    case VAR_STR:
      ptr = va_arg(ap, char*);
      len = strlen(ptr);
      if(dst)
      {
        strcpy(dst, ptr);
      }
      break;

    case VAR_INI:
      ptr = va_arg(ap, char*);
      if(ini_getentry(SETTINGS_FILE, ptr, buf, MAX_ADDR) == 0)
      {
        len = strlen(buf);
        if(dst)
        {
          strcpy(dst, buf);
        }
      }
      break;

    case VAR_MAC:
      ptr = mactoa(va_arg(ap, MAC_Addr));
      len = strlen(ptr);
      if(dst)
      {
        strcpy(dst, ptr);
      }
      break;

    case VAR_IP:
      ptr = iptoa(va_arg(ap, IP_Addr));
      len = strlen(ptr);
      if(dst)
      {
        strcpy(dst, ptr);
      }
      break;

    case VAR_RGB:
      i = va_arg(ap, long);
      sprintf(buf, "%03i,%03i,%03i", (int)GET_RED(i), (int)GET_GREEN(i), (int)GET_BLUE(i));
      len = strlen(buf);
      if(dst)
      {
        strcpy(dst, buf);
      }
      break;
/*
    case VAR_MEN:
      break;
*/
    case VAR_STN: //station name
      if((param >= 1) && (param <= (station_items()-1)))
      {
        station_getitem(param, buf);
        len = strlen(buf);
        if(dst)
        {
          strcpy(dst, buf);
        }
      }
      break;

    case VAR_STA: //station addr
      if((param >= 1) && (param <= (station_items()-1)))
      {
        station_getitemaddr(param, buf);
        len = strlen(buf);
        if(dst)
        {
          strcpy(dst, buf);
        }
      }
      break;

    case VAR_ALA: //alarm time
      if((param >= 1) && (param <= ALARMTIMES))
      {
        alarm_getitem(param, buf);
        len = strlen(buf);
        if(dst)
        {
          strcpy(dst, buf);
        }
      }
      break;
  }

  va_end(ap);

  return len;
}


unsigned int http_fparse(char *dst, unsigned int file, unsigned int *start, unsigned int len, unsigned int param)
{
  unsigned int i, l=0, slen;
  const char *src;
  char c;

  if(file >= HTTPFILES)
  {
    return 0;
  }

  src = (const char*)httpfiles[file].data;

  if(len == 0) //calc file size
  {
    for(;;)
    {
      c = *src++;
      if(c == 0)
      {
        break;
      }
      else if(c == '$')
      {
        for(i=0; i<HTTPVARS; i++)
        {
          slen = strlen(httpvars[i].name);
          if(strncmp(src, httpvars[i].name, slen) == 0)
          {
            src += slen;
            if(httpvars[i].get)
            {
              i = http_printf(0, httpvars[i].format, param, httpvars[i].get());
            }
            else
            {
              if(httpvars[i].format == VAR_INI)
              {
                i = http_printf(0, httpvars[i].format, param, httpvars[i].name);
              }
              else
              {
                i = http_printf(0, httpvars[i].format, param, 0);
              }
            }
            l += i;
            i  = 0xffff;
            break;
          }
        }
        if(i == 0xffff)
        {
          continue;
        }
      }
      l++;
    }
  }
  else
  {
    src += *start;
    for(;;)
    {
      c = *src++;
      *start += 1;
      if(c == 0)
      {
        break;
      }
      else if(c == '$')
      {
        for(i=0; i<HTTPVARS; i++)
        {
          slen = strlen(httpvars[i].name);
          if(strncmp(src, httpvars[i].name, slen) == 0)
          {
            src    += slen;
            *start += slen;
            if(httpvars[i].get)
            {
              i = http_printf(dst, httpvars[i].format, param, httpvars[i].get());
            }
            else
            {
              if(httpvars[i].format == VAR_INI)
              {
                i = http_printf(dst, httpvars[i].format, param, httpvars[i].name);
              }
              else
              {
                i = http_printf(dst, httpvars[i].format, param, 0);
              }
            }
            dst += i;
            l   += i;
            i    = 0xffff;
            break;
          }
        }
        if(i == 0xffff)
        {
          continue;
        }
      }
      *dst++ = c;
      l++;
      if(l >= len)
      {
        break;
      }
    }
  }

  return l;
}


unsigned int http_fdata(unsigned char *dst, unsigned int file, unsigned int start, unsigned int len)
{
  if(file >= HTTPFILES)
  {
    return 0;
  }

  if((start+len) > httpfiles[file].len)
  {
    len = httpfiles[file].len-start;
  }

  memcpy(dst, httpfiles[file].data+start, len);

  return len;
}


unsigned int http_flen(unsigned int file, unsigned int param)
{
  if(file >= HTTPFILES)
  {
    return 0;
  }

  switch(httpfiles[file].type)
  {
    case TXT_FILE:
    case HTML_FILE:
    case XML_FILE:
      return http_fparse(0, file, 0, 0, param);
      break;
  }

  return httpfiles[file].len;
}


unsigned int http_ftype(unsigned int file)
{
  if(file >= HTTPFILES)
  {
    return UNKNOWN_FILE;
  }

  return httpfiles[file].type;
}


unsigned int http_fid(const char *name, unsigned int *param)
{
  unsigned int file, len, i;
  char buf[8];

  if(name == 0)
  {
    return 0xFFFF;
  }

  *param = 0;

  while(*name == ' '){ name++; } //skip spaces

  for(file=0; file<HTTPFILES; file++)
  {
    len = strlen(httpfiles[file].name);
    if(strncmpi(name, httpfiles[file].name, len) == 0)
    {
      if((name[len] == 0) || (name[len] == ' ')) //file found
      {
        break;
      }
      else if(name[len] == '?') ////file found, with parameter
      {
        name += len + 1; //skip name and "?"
        for(i=0; i<8;)
        {
          if(isdigit(*name))
          {
            buf[i++] = *name++;
          }
          else
          {
            break;
          }
        }
        buf[i] = 0;
        *param = atoi(buf);
        break;
      }
    }
  }

  if(file >= HTTPFILES)
  {
    return 0xFFFF;
  }

  return file;
}
