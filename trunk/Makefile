SHELL	= sh

PRG	= v3
MODS	= ddsbase dicombase oglbase guibase tfbase tilebase

LIBS	= -lGL -lGLU -lm

SRCS	= $(PRG:=.cpp) $(MODS:=.cpp)
OBJS	= $(PRG:=.o) $(MODS:=.o)

AR	= ar -crs
RM	= rm -rf

.SUFFIXES: .cpp

.cpp.o:
	$(COMPILER) $(OPTS) -c -D$(TARGET) $*.cpp

trap:

all:	$(OBJS)
	@-$(AR) libv3.a $(MODS:=.o)
	@-(cd tools; make all)
	$(COMPILER) $(OPTS) -o $(PRG) $(OBJS) $(LINK) $(LIBS) $(LINK)
	@-if [ -e $(PRG) ]; then strip $(PRG); fi

tools:
	@-(cd tools; make all)

depend:
	@-(cd tools; make depend)
	@-$(MAKEDEPEND) -D$(TARGET) $(SRCS) >Makefile.d 2>/dev/null

clean:
	@-(cd tools; make clean)
	@-$(RM) $(OBJS) ii_files

tidy:	clean
	@-(cd tools; make tidy)
	@-$(RM) *~ Makefile.d Debug Release *.ncb *.opt *.plg
	@-$(RM) $(PRG) libv3.a libv3

sinclude Makefile.d
