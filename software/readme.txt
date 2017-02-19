ARM Cortex-M3 WebRadio
======================
  Compiled with Rowley's CrossWorks for ARM 2.1.1
                         CPU Support Pkg. Stellaris 1.8


Third party software
--------------------
  Luminary Micro StellarisWare Driver Library 5727
    http://www.luminarymicro.com
    Modifications:
      lmi/driverlib/cpu.c: 43: PRIMASK -> primask
      lmi/driverlib/cpu.c:125: PRIMASK -> primask
      lmi/driverlib/interrupt.c:108: vtable -> .vectors_ram

  ChaN's FatFs 0.08b
    http://elm-chan.org/fsw/ff/00index_e.html

  Makefile from WinARM (by Martin Thomas)
    http://www.siwawi.arubi.uni-kl.de/avr_projects/arm_projects/


License
-------
  See license.txt


History
-------
  Oct 02 2011  v0.14  RTSP support improved (keepalive)

  Sep 29 2011  v0.13  Bugfix in base64_decode()
                      Streaming improved

  May 09 2011  v0.12  VS1053 patch 1.8
                      FatFS 0.08b

  Dec 25 2010  v0.11  VS1053 patch 1.5
                      FatFS 0.08a

  Jul 23 2010  v0.10  SD init bugfix

  May 03 2010  v0.09  SD power control disabled

  Mar 12 2010  v0.08  Improved LS020 code
                      Changed MMC init to old version

  Feb 20 2010  v0.07  CodeSourcery Toolchain support
                      (Thanks to Daniel Buergin)
                      Added Chuck Norris Facts
                      Some Bugfixes

  Jan 07 2010  v0.06  Bugfix release

  Dec 22 2009  v0.05  IR commands changeable
                      Different Alarm actions

  Nov 29 2009  v0.04  Changes in VS interface
                      LCD L2F50 support

  Nov 18 2009  v0.03  Switched to CrossWorks CPU Support Pkg. Stellaris 1.1
                      Bootloader added
                      New buffer system
                      F-RAM support

  Aug 19 2009  v0.02  VS1053b FLAC decoder plugin
                      Auto detection of VS type (VS1053b, VS1033c, VS1033d)
                      Automatic standby
                      LFN support
                      Modify settings via Webinterface
                      Modify settings in the radio menu
                      DNS client

  Mar 16 2009  v0.01  First release
