.SILENT:

PACKAGE = PS1_HDMI_FIX
EE_BIN = $(PACKAGE).ELF
EE_OBJS := gsm_engine.o gsm_api.o
EE_LDFLAGS =  -nostartfiles -Tlinkfile	
EE_INCS +=  -I$(PS2SDK)/ee/include -I$(PS2SDK)/iop/include -I$(PS2SDK)/ports/include -I$(GSKIT)/include -I$(GSKIT)/ee/dma/include -I$(GSKIT)/ee/gs/include -I$(GSKIT)/ee/toolkit/include

EE_LIBS = -lmc -lpad -lfileXio -lpatches -ldebug -lc -lkernel -L$(GSKIT)/lib -lgskit -ldmakit

EE_LDFLAGS =  -nostartfiles -Tlinkfile -L$(PS2SDK)/ee/lib -L$(PS2SDK)/sbv/lib -s

#EE_LDFLAGS += -Xlinker -Map -Xlinker 'uncompressed $(PACKAGE).map'

all: $(EE_BIN)
	 rm -f 'uncompressed $(PACKAGE).ELF'
	 mv $(PACKAGE).ELF 'uncompressed $(PACKAGE).ELF'
	 ee-strip 'uncompressed $(PACKAGE).ELF'
	 ps2-packer 'uncompressed $(PACKAGE).ELF' $(PACKAGE).ELF > /dev/null

dump:
	ee-objdump -D 'uncompressed $(PACKAGE).ELF' > $(PACKAGE).dump
	ps2client netdump

test:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:'uncompressed $(PACKAGE).ELF'

run:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:$(PACKAGE).ELF

line:
	ee-addr2line -e 'uncompressed $(PACKAGE).ELF' $(ADDR)

reset:
	ps2client -h $(PS2_IP) reset

clean:
	rm -f *.ELF *.o *.a *.s *.i *.map

rebuild:clean all

release:rebuild
	rm -f 'uncompressed $(PACKAGE).ELF' *.o *.a *.s *.i *.map

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
