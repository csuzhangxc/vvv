SHELL	= sh

PRGS	= raw2pvm pvm2raw pvm2pvm pvminfo pvmplay dti2pvm rgb2hsv

LIBS	= -L.. -lv3 -lGL -lGLU -lm

SRCS	= $(PRGS:=.cpp)
OBJS	= $(PRGS:=.o)

CP	= cp -rf
RM	= rm -rf

.SUFFIXES: .cpp

.cpp.o:
	$(COMPILER) $(OPTS) -c -D$(TARGET) -I.. $*.cpp
	$(COMPILER) $(OPTS) -o $* $*.o $(LINK) $(LIBS) $(LINK)
	@-if [ -e $* ]; then strip $*; fi

trap:

all:	$(OBJS)

depend:
	@-$(MAKEDEPEND) -D$(TARGET) -I.. $(SRCS) >Makefile.d 2>/dev/null

clean:
	@-$(RM) $(OBJS) ii_files

tidy:	clean
	@-$(RM) *~ Makefile.d Debug Release *.ncb *.opt *.plg
	@-$(RM) $(PRGS)

sinclude Makefile.d
