QT += core gui widgets

CONFIG += c++11
CONFIG += warn_on
CONFIG += exceptions
CONFIG += silent
CONFIG += lrelease embed_translations       # embeds ":/i18n/gui_*.qm"

include(thirdparty/qtsingleapplication/src/qtsingleapplication.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

GIT_VER = $$system(git --work-tree $$PWD rev-parse --short HEAD)
isEmpty(GIT_VER) {
}
APP_VER=$$(APP_VER)
isEmpty(APP_VER) {
    APP_VER=$$GIT_VER
}

DEFINES += APP_VER=\\\"$$APP_VER\\\"


SOURCES += \
    SettingsDialog.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h \
    SettingsDialog.h

FORMS += \
    MainWindow.ui \
    SettingsDialog.ui

TRANSLATIONS += \
    softfm-gui_tr_TR.ts
CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += \
    resources/images/images.qrc \
    thirdparty/QDarkStyleSheet/qdarkstyle/style.qrc



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
