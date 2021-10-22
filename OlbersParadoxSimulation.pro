QT       += core gui 3dcore 3drender 3dinput 3dextras charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Clustering.cpp \
    DataChart.cpp \
    DataTable.cpp \
    FractalClustering.cpp \
    HalleyClustering.cpp \
    LinearizedChart.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    Clustering.h \
    DataChart.h \
    DataTable.h \
    FractalClustering.h \
    Global.h \
    HalleyClustering.h \
    LinearizedChart.h \
    MainWindow.h

FORMS += \
    DataChart.ui \
    DataTable.ui \
    LinearizedChart.ui \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
	.gitignore
