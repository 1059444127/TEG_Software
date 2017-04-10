#-------------------------------------------------
#
# Project created by QtCreator 2015-12-05T15:39:37
#
#-------------------------------------------------


QT       += core gui serialport  printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TEG-Beta01
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    commandedit.cpp \
    commandpanel.cpp \
    commandwidget.cpp \
    dataformatpanel.cpp \
    hidabletabwidget.cpp \
    plotsnapshotoverlay.cpp \
    portcontrol.cpp \
    portlist.cpp \
    scalepicker.cpp \
    scalezoomer.cpp \
    snapshot.cpp \
    snapshotmanager.cpp \
    snapshotview.cpp \
    tooltipfilter.cpp \
    presetdlg.cpp \
    plot.cpp \
    signaldata.cpp \
    maintenancedlg.cpp \
    setelasticdlg.cpp \
    zoomer.cpp \
    scanbarcodedlg.cpp \
    resultshowdlg.cpp

HEADERS  += mainwindow.h \
    commandedit.h \
    commandpanel.h \
    commandwidget.h \
    dataformatpanel.h \
    floatswap.h \
    hidabletabwidget.h \
    plotsnapshotoverlay.h \
    portcontrol.h \
    portlist.h \
    scalepicker.h \
    scalezoomer.h \
    snapshot.h \
    snapshotmanager.h \
    snapshotview.h \
    tooltipfilter.h \
    utils.h \
    zoomer.h \
    presetdlg.h \
    plot.h \
    signaldata.h \
    maintenancedlg.h \
    setelasticdlg.h \
    scanbarcodedlg.h \
    resultshowdlg.h

FORMS    += mainwindow.ui \
    about_dialog.ui \
    commandpanel.ui \
    commandwidget.ui \
    dataformatpanel.ui \
    portcontrol.ui \
    presetdlg.ui \
    snapshotview.ui \
    maintenancedlg.ui \
    setelasticdlg.ui \
    scanbarcodedlg.ui \
    resultshowdlg.ui

CONFIG += c++11

RESOURCES += \
    toolbar.qrc \
    appicon.qrc


LIBS +=  -lqwtd
INCLUDEPATH += C:\Qt\Qt5.5.1\5.5\mingw492_32\include\QtQwt


RC_FILE += loginico.rc
OTHER_FILES += \
    loginico.rc




