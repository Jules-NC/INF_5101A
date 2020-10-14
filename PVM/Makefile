#
# minimum Makefile for PVM at ESIEE
#

# include local arch. particularities

include $(PVM_ROOT)/conf/$(PVM_ARCH).def

#CFLAGS	=	-I$(INC) $(ARCHCFLAGS)

LIBS	=	-lpvm3 -lgpvm3 $(ARCHLIB) -lm
#LIBS	=	-L$(PVM3) -L$(PVM3G) $(ARCHLIB) -lm
LFLAGS	=	-L$(PVM_ROOT)/lib/$(PVM_ARCH)
LDFLAGS	=	$(LFLAGS) $(LIBS)
PVM3 = /usr/lib/x86_64-linux-gnu/libpvm3.so.3
PVM3G = /usr/lib/x86_64-linux-gnu/libgpvm3.so.3
INC=    /home/jules/pvm3/include/
# dependances go here

.c:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	sudo mv $@ $(PVM_ROOT)/bin/$(PVM_ARCH)

#hello: $$@.c $$@.h
#	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)
#	mv $@ bin/$(PVM_ARCH)
