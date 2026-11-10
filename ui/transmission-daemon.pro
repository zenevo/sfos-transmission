QT += quick network
CONFIG += sailfishapp c++11
TARGET = transmission-daemon-ui
SOURCES += src/main.cpp src/backend.cpp
HEADERS += src/backend.h
RESOURCES += qml.qrc

qmlroot.path = /usr/share/transmission-daemon-ui/qml
qmlroot.files = qml/main.qml

qmlpages.path = /usr/share/transmission-daemon-ui/qml/pages
qmlpages.files = qml/pages/FirstPage.qml

qmlcover.path = /usr/share/transmission-daemon-ui/qml/cover
qmlcover.files = qml/cover/CoverPage.qml

INSTALLS += qmlroot qmlpages qmlcover
