QT += core quick network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

RESOURCES += \
        res.qrc

OTHER_FILES += \
        main.qml \
        gui\MainWindow.qml

# \表示开始和分隔，/表示文件路径，/会引入文件夹，\则可以不引入文件夹
# 用\找到文件则表示找到特定文件，忽略文件夹
# 官方的结构是用/分隔，用\表示路径，如现在RESOURCES的情况

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

UI_DIR=./UI # 解决修改qml后每次都需重新编译的问题

# 子模块
include($$PWD/backend/adb/adb.pri)
include($$PWD/backend/server/server.pri)
include($$PWD/backend/decoder/decoder.pri)
include($$PWD/backend/common/common.pri)
include($$PWD/backend/render/render.pri)
include($$PWD/backend/access/access.pri)
include($$PWD/backend/android/android.pri)
include($$PWD/backend/inputcontrol/inputcontrol.pri)

# 包含目录
INCLUDEPATH += \
        $$PWD/backend/adb \
        $$PWD/backend/server \
        $$PWD/backend/decoder \
        $$PWD/backend/common \
        $$PWD/backend/render \
        $$PWD/backend/access \
        $$PWD/backend/android \
        $$PWD/backend/inputcontrol \
        $$PWD/lib/thrid_party/ffmpeg/include

# 依赖模块
LIBS += \
        -L$$PWD/lib/thrid_party/ffmpeg/lib -lavformat \
        -L$$PWD/lib/thrid_party/ffmpeg/lib -lavcodec \
        -L$$PWD/lib/thrid_party/ffmpeg/lib -lavutil \
        -L$$PWD/lib/thrid_party/ffmpeg/lib -lswscale

HEADERS +=
