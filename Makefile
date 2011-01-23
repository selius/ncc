CXX = g++

ifdef COMSPEC
RM = del
else
RM = rm
endif

SOURCES		=	src/main.cpp \
			src/common.cpp \
			src/cli.cpp \
			src/scanner.cpp \
			src/parser.cpp \
			src/logger.cpp

TARGET		=	bin/ncc

CFLAGS		:=	$(CFLAGS) -Iinclude


all: $(SOURCES)
	$(CXX) -o $(TARGET) $(CFLAGS) $(SOURCES)

debug: CFLAGS += -g3
debug: all

strict: CFLAGS += -Wall
strict: all

tags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

distclean:
	-$(RM) $(TARGET)
