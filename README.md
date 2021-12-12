# QSNX v1.0
**snx** is GUI to proprietary Check Point's SNX VPN client.  

It contains four subprojects:  

**qsnx_service** - dbus service  
**qsnx_client**  - main client program  
**setbuf**  - a helper to disable buffering for stdin, stdout and stderr  

# COMPILATION:

**pkgdir** is for pkgbild (ignore it if you are going to compile manually)  
**srcdir** is the main dir with qpacman.pro  

cd $srcdir  
qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug  
make  
make install  

You can use **CONFIG+=DISCONNECT_SWITCH=value**, **CONFIG+=URL_SWITCH=value**, **CONFIG+=CERTIFICATE_SWITCH=value**, **CONFIG+=PORT_SWITCH=value**, **CONFIG+=USER_SWITCH=value** in qmake arguments if you need to use other snx switches  


**QT5 only**  

Depends: '**Qt >= 5: core,network,gui,dbus,widgets,svg**', '**snx**' and '**gawk**' as a makedepend. 
  

