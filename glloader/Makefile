# Makefile

CXXC=g++
OBJS=glloader_gl.o glloader_wgl.o glloader_glx.o utils.o
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