# Makefile

CXXC=g++
OBJS=GLLoader_GL.o GLLoader_WGL.o GLLoader_GLX.o GLLoader.o Utils.o
LIBFILE=libGLLoader.a
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