QT       += core gui dbus network widgets
TARGET = qsnx_client

CONFIG += c++11
DEFINES += QAPPLICATION_CLASS=QApplication
DEFINES += VERSION=\\\"1.1\\\"

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}
DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qsnx
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DBUS_INTERFACES = ../qsnx.xml

SOURCES += \
    ../service/singleapplication.cpp \
    ../service/singleapplication_p.cpp \
    confreader.cpp \
    main.cpp \
    profiledialog.cpp \
    qsnxclient.cpp \
    qsnxwindow.cpp \
    simplecrypt.cpp \
    windowcenterer.cpp

HEADERS += \
    ../service/singleapplication.h \
    ../service/singleapplication_p.h \
    confreader.h \
    profiledialog.h \
    qsnxclient.h \
    qsnxwindow.h \
    simplecrypt.h \
    windowcenterer.h

FORMS += \
    profiledialog.ui \
    qsnxwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qsnx.qrc

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
updatets.commands = $$LUPDATE $$PWD/client.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

icon.files = $$PWD/pics/key.svg
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

desktop.files = $$PWD/qsnx.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

metainfo.files = $$PWD/qsnx.metainfo.xml
metainfo.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/metainfo/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target transinstall icon desktop metainfo
