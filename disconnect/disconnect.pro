QT = core dbus network

CONFIG += c++11 cmdline
TARGET = qsnx_disconnect

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}
DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        ../service/singleapplication.cpp \
        ../service/singleapplication_p.cpp \
        ../client/qsnxclient.cpp

HEADERS += \
    ../service/singleapplication.h \
    ../service/singleapplication_p.h \
    ../client/qsnxclient.h

DBUS_INTERFACES = ../qsnx.xml

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

fileschange1.input = qsnx-hibernate.in
fileschange1.output = $$OUT_PWD/qsnx-hibernate
QMAKE_SUBSTITUTES += fileschange1

DISTFILES += \
    qsnx-hibernate

qsnx_hibernate.path = /tmp
qsnx_hibernate.extra = mkdir -p $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system-sleep; cp $$OUT_PWD/qsnx-hibernate $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system-sleep/qsnx-hibernate; chmod a+x $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system-sleep/qsnx-hibernate
qsnx_hibernate.files = $$OUT_PWD/qsnx-hibernate

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target qsnx_hibernate
