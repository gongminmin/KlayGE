# Makefile

CXXC=g++
OBJS=gl12.o gl13.o gl14.o gl15.o gl20.o gl21.o glloader_gl.o glloader_wgl.o glloader_glx.o utils.o
LIBFILE=libglloader.a
INCDIR=./include
PREFIX=/usr

%.o: src/%.cpp
	$(CXXC) $< -c $(CXXFLAGS) -I$(INCDIR) -o $@

all: $(OBJS)
	ar rcs $(LIBFILE) $(OBJS)

install: all
	install -m 755 $(LIBFILE) $(PREFIX)/lib

clean:
	rm -f *.o
	rm -f *.a

.PHONY: all