TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    src/main.cpp \
    src/Cpu.cpp \
    src/Memory.cpp \
    src/Ppu.cpp \
    src/Mapper.cpp

HEADERS += \
    include/Cpu.h \
    include/Memory.h \
    include/Ppu.h \
    include/Mapper.h \
    include/mappers/NRom128.h \
    include/mappers/NRom256.h \
    include/mappers/MMC1.h

OTHER_FILES += \
    DKrom.nes\
    nestest.log\
    info.log

LIBS += -L/usr/local/lib -lSDL -ldl -lpthread
INCLUDES += /usr/local/include
