OBJECTS	=	builtin_print_int.o \
		builtin_print_float.o

TARGET	=	builtin.a

all: $(TARGET)

clean:
	-$(RM) $(OBJECTS)

distclean: clean
	-$(RM) $(TARGET)

%.o: %.s
	gcc -c -o $@ $<

$(TARGET) : $(TARGET)($(OBJECTS))
