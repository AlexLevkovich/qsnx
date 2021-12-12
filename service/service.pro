QT -= gui
QT += dbus network
TARGET = qsnx_service
CONFIG += c++11 console
CONFIG -= app_bundle

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

isEmpty(DISCONNECT_SWITCH) {
    DISCONNECT_SWITCH = -d
}
DEFINES += DISCONNECT_SWITCH=\\\"$$DISCONNECT_SWITCH\\\"

isEmpty(URL_SWITCH) {
    URL_SWITCH = -s
}
DEFINES += URL_SWITCH=\\\"$$URL_SWITCH\\\"

isEmpty(CERTIFICATE_SWITCH) {
    CERTIFICATE_SWITCH = -c
}
DEFINES += CERTIFICATE_SWITCH=\\\"$$CERTIFICATE_SWITCH\\\"

isEmpty(PORT_SWITCH) {
    PORT_SWITCH = -p
}
DEFINES += PORT_SWITCH=\\\"$$PORT_SWITCH\\\"

isEmpty(USER_SWITCH) {
    USER_SWITCH = -u
}
DEFINES += USER_SWITCH=\\\"$$USER_SWITCH\\\"

isEmpty(TMP_DIR) {
    TMP_DIR = /tmp
}
DEFINES += TMP_DIR=\\\"$$TMP_DIR\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qsnx
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

DEFINES += QAPPLICATION_CLASS=QCoreApplication
DEFINES += QT_DEPRECATED_WARNINGS

SO_BIN1=$$INSTALL_PREFIX/lib/libsetbuf.so
SO_BIN2=$$OUT_PWD/../setbuf/libsetbuf.so
DEFINES += SO_BIN1=\\\"$$SO_BIN1\\\"
DEFINES += SO_BIN2=\\\"$$SO_BIN2\\\"

#SNX_PATH should be a full path to snx executable
isEmpty(SNX_PATH) {
    SNX_PATH = /usr/bin/snx
}
DEFINES += SNX_PATH=\\\"$$SNX_PATH\\\"

DBUS_ADAPTORS = ../qsnx.xml

SOURCES += \
        main.cpp \
        qsnxservice.cpp \
        sigwatch.cpp \
        singleapplication.cpp \
        singleapplication_p.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    qsnxservice.h \
    sigwatch.h \
    singleapplication.h \
    singleapplication_p.h

LANGUAGES = ru be

defineReplace(prependAll) {
 for(a,$$1):result += $${2}_$${a}$$3
 return($$result)
}

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/$$TARGET, .ts)

LRELEASE_COMMAND =
LRELEASE_TARGET =
LRELEASE = $$[QT_INSTALL_BINS]/lrelease
for(tsfile, TRANSLATIONS) {
    qmfile = $$basename(tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$OUT_PWD/translations
    qmfile = $$qmdir/$$qmfile
    !exists($$qmdir) {
        system($${QMAKE_MKDIR} \"$$qmdir\")
    }
    LRELEASE_TARGET += $$qmfile
    LRELEASE_COMMAND += $$LRELEASE $$tsfile -qm $$qmfile;
}

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
updatets.target = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/service.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

fileschange1.target = com.alexl.qt.QSNX.service
fileschange1.depends = FORCE
fileschange1.commands = cd $$PWD; cat com.alexl.qt.QSNX.service.orig | awk -v f="/usr/bin/" -v t="$$INSTALL_PREFIX/bin/" \'{if (substr(t,1,2) == \"//\") {t=substr(t,2)}; gsub(f,t);print \$0}\' > com.alexl.qt.QSNX.service
PRE_TARGETDEPS += com.alexl.qt.QSNX.service
QMAKE_EXTRA_TARGETS += fileschange1
fileschange2.target = qsnx.service
fileschange2.depends = FORCE
fileschange2.commands = cd $$PWD; cat qsnx.service.orig | awk -v f="/usr/bin/" -v t="$$INSTALL_PREFIX/bin/" \'{if (substr(t,1,2) == \"//\") {t=substr(t,2)}; gsub(f,t);print \$0}\' > qsnx.service
PRE_TARGETDEPS += qsnx.service
QMAKE_EXTRA_TARGETS += fileschange2

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target transinstall

DISTFILES += \
    com.alexl.qt.QSNX.conf \
    com.alexl.qt.QSNX.service \
    qsnx.service

dbus_config.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/dbus-1/system.d/
dbus_config.files = com.alexl.qt.QSNX.conf

dbus_service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/dbus-1/system-services/
dbus_service.files = com.alexl.qt.QSNX.service

systemd_service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/
systemd_service.files = qsnx.service

INSTALLS += dbus_config dbus_service systemd_service
