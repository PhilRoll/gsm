PACKAGE = PS1_HDMI_FIX
BUILD_DIR = build
EE_BIN = $(BUILD_DIR)/$(PACKAGE).ELF

EE_OBJS := gsm_engine.o main.o

EE_INCS += -I$(GSKIT)

EE_LIBS = -ldebug -lcdvd -lpadx -lmc -lhdd -lmf -lfileXio -lpatches -lpoweroff

EE_LDFLAGS =  -nostartfiles -Tlinkfile

all: $(BUILD_DIR) $(EE_BIN)
	rm -f '$(BUILD_DIR)/uncompressed_$(PACKAGE).ELF'
	mv $(BUILD_DIR)/$(PACKAGE).ELF $(BUILD_DIR)/uncompressed_$(PACKAGE).ELF
	ee-strip $(BUILD_DIR)/uncompressed_$(PACKAGE).ELF
	ps2-packer $(BUILD_DIR)/uncompressed_$(PACKAGE).ELF $(BUILD_DIR)/$(PACKAGE).ELF > /dev/null

dump:
	ee-objdump -D '$(BUILD_DIR)/uncompressed_$(PACKAGE).ELF' > $(BUILD_DIR)/$(PACKAGE).dump
	ps2client netdump

test:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:$(BUILD_DIR)/uncompressed_$(PACKAGE).ELF

run:
	ps2client -h $(PS2_IP) reset
	ps2client -h $(PS2_IP) execee host:$(BUILD_DIR)/$(PACKAGE).ELF

line:
	ee-addr2line -e  $(BUILD_DIR)/$(PACKAGE).ELF $(ADDR)

reset:
	ps2client -h $(PS2_IP) reset

clean:
	rm -f $(BUILD_DIR)/*.ELF *.o *.a *.s *.i *.map

rebuild:clean all

release:rebuild
	rm -f '$(BUILD_DIR)/uncompressed_$(PACKAGE).ELF' *.o *.a *.s *.i *.map

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
