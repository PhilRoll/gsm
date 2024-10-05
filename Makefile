PACKAGE = PS1_HDMI_FIX
EE_BIN = $(PACKAGE).ELF

EE_OBJS := gsm_engine.o main.o

EE_INCS += -I$(GSKIT)

EE_LIBS = -ldebug -lcdvd -lpadx -lmc -lhdd -lmf -lfileXio -lpatches -lpoweroff

EE_LDFLAGS =  -nostartfiles -Tlinkfile

#EE_LDFLAGS += -Xlinker -Map -Xlinker 'uncompressed $(PACKAGE).map'

all: $(EE_BIN)
	 rm -f 'uncompressed $(PACKAGE).ELF'
	 mv $(PACKAGE).ELF 'uncompressed_$(PACKAGE).ELF'
	 ee-strip 'uncompressed_$(PACKAGE).ELF'
	 ps2-packer 'uncompressed_$(PACKAGE).ELF' $(PACKAGE).ELF > /dev/null

dump:
	ee-objdump -D 'uncompressed_$(PACKAGE).ELF' > $(PACKAGE).dump
	ps2client netdump

test:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:'uncompressed_$(PACKAGE).ELF'

run:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:$(PACKAGE).ELF

line:
	ee-addr2line -e  $(PACKAGE).ELF' $(ADDR)

reset:
	ps2client -h $(PS2_IP) reset

clean:
	rm -f *.ELF *.o *.a *.s *.i *.map

rebuild:clean all

release:rebuild
	rm -f 'uncompressed_$(PACKAGE).ELF' *.o *.a *.s *.i *.map

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal