CXX = g++

ifdef COMSPEC
RM = del
else
RM = rm
endif

SOURCES		=	src/main.cpp

TARGET		=	bin/ncc


all: $(SOURCES)
	$(CXX) -o $(TARGET) $(CFLAGS) $(SOURCES)

debug: CFLAGS += -g3
debug: all

strict: CFLAGS += -Wall
strict: all

distclean:
	-$(RM) $(TARGET)
