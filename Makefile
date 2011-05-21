CXX = g++

ifdef COMSPEC
RM = del
else
RM = rm
endif

SOURCES		=	src/main.cpp \
			src/common.cpp \
			src/cli.cpp \
			src/prettyprinting.cpp \
			src/scanner.cpp \
			src/parser.cpp \
			src/codegen.cpp \
			src/optimization.cpp \
			src/expressions.cpp \
			src/statements.cpp \
			src/symbols.cpp 

BIN_DIR		=	bin/

BUILTIN_DIR	=	builtin/

TARGET		=	$(BIN_DIR)ncc

CFLAGS		:=	$(CFLAGS) -Iinclude


all: $(BIN_DIR) $(SOURCES)
	$(CXX) -o $(TARGET) $(CFLAGS) $(SOURCES)
	$(MAKE) -C $(BUILTIN_DIR)

debug: CFLAGS += -g3
debug: all

strict: CFLAGS += -Wall
strict: all

tags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

distclean:
	-$(RM) -r $(BIN_DIR)
	-$(RM) -r tests/*/output/
	-$(RM) -r tests/codegen/optimized-output/
	$(MAKE) -C $(BUILTIN_DIR) distclean

$(BIN_DIR):
	mkdir $(BIN_DIR)
