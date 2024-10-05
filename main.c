#include <libcdvd.h>      // For CD/DVD functions such as sceCdDiskReady, sceCdGetDiskType, sceCdInit, etc.
#include <loadfile.h>     // For functions like SifLoadElf, ExecPS2, etc.
#include <kernel.h>       // For kernel management functions like sceKernel...
#include <fileio.h>       // For file I/O operations
#include <sifrpc.h>       // For Remote Procedure Call (RPC)
#include <osd_config.h>   // For OSD (On-Screen Display) configurations
#include <debug.h>        // For debug functions like scr_printf, scr_setXY
#include <stdio.h>        // For standard input/output functions
#include <stdlib.h>       // For standard library functions
#include <string.h>       // For string manipulation functions
#include <malloc.h>       // For dynamic memory allocation functions
#include <errno.h>        // For error handling


/*
GSM Fork https://github.com/PhilRoll/gsm
*/

// For a quick PS1 PAL/NTSC GSM selection:
#define PAL_PREDEF_VMODE_IDX  1
#define PAL_X_OFFSET -88
#define PAL_Y_OFFSET 14
#define NTSC_PREDEF_VMODE_IDX 0
#define NTSC_X_OFFSET -48
#define NTSC_Y_OFFSET 4



#define make_display_magic_number(dh, dw, magv, magh, dy, dx)      \
	(((u64)(dh) << 44) | ((u64)(dw) << 32) | ((u64)(magv) << 27) | \
	 ((u64)(magh) << 23) | ((u64)(dy) << 12) | ((u64)(dx) << 0))

#define MAKE_J(func) (u32)((0x02 << 26) | (((u32)func) / 4)) // Jump (MIPS instruction)

// GS Registers
#define GS_BGCOLOUR *((volatile unsigned long int *)0x120000E0)

// VMODE TYPES
#define PS1_VMODE 1
#define HDTV_VMODE 3

/// Non-Interlaced Mode
#define GS_NONINTERLACED 0x00
/// Interlaced Mode
#define GS_INTERLACED 0x01

/// Field Mode - Reads Every Other Line
#define GS_FIELD 0x00
/// Frame Mode - Reads Every Line
#define GS_FRAME 0x01

/// DTV 480 Progressive Scan (720x480)
#define GS_MODE_DTV_480P 0x50
/// DTV 576 Progressive Scan (720x576)
#define GS_MODE_DTV_576P 0x53


// Prototypes for External Functions
#define _GSM_ENGINE_ __attribute__((section(".gsm_engine"))) // Resident section

extern void *Old_SetGsCrt _GSM_ENGINE_;

extern u32 Target_INTERLACE _GSM_ENGINE_;
extern u32 Target_MODE _GSM_ENGINE_;
extern u32 Target_FFMD _GSM_ENGINE_;

extern u64 Target_SMODE2 _GSM_ENGINE_;
extern u64 Target_DISPLAY1 _GSM_ENGINE_;
extern u64 Target_DISPLAY2 _GSM_ENGINE_;
extern u64 Target_SYNCV _GSM_ENGINE_;

extern u8 automatic_adaptation _GSM_ENGINE_;
extern u8 DISPLAY_fix _GSM_ENGINE_;
extern u8 SMODE2_fix _GSM_ENGINE_;
extern u8 SYNCV_fix _GSM_ENGINE_;
extern u8 skip_videos_fix _GSM_ENGINE_;

extern u32 X_offset _GSM_ENGINE_;
extern u32 Y_offset _GSM_ENGINE_;

extern void Hook_SetGsCrt() _GSM_ENGINE_;
extern void GSHandler() _GSM_ENGINE_;

typedef struct predef_vmode_struct
{
	u8 category;
	char desc[34];
	u8 interlace;
	u8 mode;
	u8 ffmd;
	u64 display;
	u64 syncv;
} predef_vmode_struct;

// Pre-defined vmodes
// Some of following vmodes gives BSOD and/or freezing, depending on the console BIOS version, TV/Monitor set, PS2 cable (composite, component, VGA, ...)
// Therefore there are many variables involved here that can lead us to success or fail depending on the circumstances above mentioned.
//
//	category	description								interlace			mode			 	ffmd	   	display							dh		dw		magv	magh	dy		dx		syncv
//	--------	-----------								---------			----			 	----		----------------------------	--		--		----	----	--		--		-----
static const predef_vmode_struct predef_vmode[8] = {
	{PS1_VMODE, "PS1 NTSC (HDTV 480p @60Hz)     ", GS_NONINTERLACED, GS_MODE_DTV_480P, GS_FRAME, (u64)make_display_magic_number(255, 2559, 0, 1, 12, 736), 0x00C78C0001E00006},
	{PS1_VMODE, "PS1 PAL (HDTV 576p @50Hz)      ", GS_NONINTERLACED, GS_MODE_DTV_576P, GS_FRAME, (u64)make_display_magic_number(255, 2559, 0, 1, 23, 756), 0x00A9000002700005},
}; // ends predef_vmode definition

u32 predef_vmode_size = sizeof(predef_vmode) / sizeof(predef_vmode[0]);

#define DI2 DIntr
#define EI2 EIntr

_GSM_ENGINE_ int ee_kmode_enter2()
{
	u32 status, mask;

	__asm__ volatile(
		".set\tpush\n\t"
		".set\tnoreorder\n\t"
		"mfc0\t%0, $12\n\t"
		"li\t%1, 0xffffffe7\n\t"
		"and\t%0, %1\n\t"
		"mtc0\t%0, $12\n\t"
		"sync.p\n\t"
		".set\tpop\n\t" : "=r"(status), "=r"(mask));

	return status;
}

_GSM_ENGINE_ int ee_kmode_exit2()
{
	int status;

	__asm__ volatile(
		".set\tpush\n\t"
		".set\tnoreorder\n\t"
		"mfc0\t%0, $12\n\t"
		"ori\t%0, 0x10\n\t"
		"mtc0\t%0, $12\n\t"
		"sync.p\n\t"
		".set\tpop\n\t" : "=r"(status));

	return status;
}

_GSM_ENGINE_ void SetSyscall2(int number, void (*functionPtr)(void))
{
	__asm__ __volatile__(
		".set noreorder\n"
		".set noat\n"
		"li $3, 0x74\n"
		"add $4, $0, %0    \n" // Specify the argument #1
		"add $5, $0, %1    \n" // Specify the argument #2
		"syscall\n"
		"jr $31\n"
		"nop\n"
		".set at\n"
		".set reorder\n"
		:
		: "r"(number), "r"(functionPtr));
}

_GSM_ENGINE_ u32 *GetROMSyscallVectorTableAddress(void)
{
	// Search for Syscall Table in ROM
	u32 i;
	u32 startaddr;
	u32 *ptr;
	u32 *addr;
	startaddr = 0;
	for (i = 0x1FF00000; i < 0x1FFFFFFF; i += 4)
	{
		if (*(u32 *)(i + 0) == 0x40196800)
		{
			if (*(u32 *)(i + 4) == 0x3C1A8001)
			{
				startaddr = i - 8;
				break;
			}
		}
	}
	ptr = (u32 *)(startaddr + 0x02F0);
	addr = (u32 *)((ptr[0] << 16) | (ptr[2] & 0xFFFF));
	addr = (u32 *)((u32)addr & 0x1fffffff);
	addr = (u32 *)((u32)addr + startaddr);
	return addr;
}

_GSM_ENGINE_ void InitGSM(u32 interlace, u32 mode, u32 ffmd, u64 display, u64 syncv, u64 smode2, int dx_offset, int dy_offset, u8 skip_videos)
{

	u32 *ROMSyscallTableAddress;

	// Update GSM params
	DI2();
	ee_kmode_enter2();

	Target_INTERLACE = interlace;
	Target_MODE = mode;
	Target_FFMD = ffmd;
	Target_DISPLAY1 = display;
	Target_DISPLAY2 = display;
	Target_SYNCV = syncv;
	Target_SMODE2 = smode2;
	X_offset = dx_offset;			   // X-axis offset -> Use it only when automatic adaptations formulas don't fit into your needs
	Y_offset = dy_offset;			   // Y-axis offset -> Use it only when automatic adaptations formulas dont't fit into your needs
	skip_videos_fix = skip_videos ^ 1; // Skip Videos Fix ------------> 0 = On, 1 = Off ; Default = 0 = On

	automatic_adaptation = 0; // Automatic Adaptation -> 0 = On, 1 = Off ; Default = 0 = On
	DISPLAY_fix = 0;		  // DISPLAYx Fix ---------> 0 = On, 1 = Off ; Default = 0 = On
	SMODE2_fix = 0;			  // SMODE2 Fix -----------> 0 = On, 1 = Off ; Default = 0 = On
	SYNCV_fix = 0;			  // SYNCV Fix ------------> 0 = On, 1 = Off ; Default = 0 = On

	ee_kmode_exit2();
	EI2();

	// Hook SetGsCrt
	ROMSyscallTableAddress = GetROMSyscallVectorTableAddress();
	Old_SetGsCrt = (void *)ROMSyscallTableAddress[2];
	SetSyscall2(2, &Hook_SetGsCrt);

	// Remove all breakpoints (even when they aren't enabled)
	__asm__ __volatile__(
		".set noreorder\n"
		".set noat\n"
		"li $k0, 0x8000\n"
		"mtbpc $k0\n" // All breakpoints off (BED = 1)
		"sync.p\n"	  // Await instruction completion
		".set at\n"
		".set reorder\n");

	// Replace Core Debug Exception Handler (V_DEBUG handler) by GSHandler
	DI2();
	ee_kmode_enter2();
	*(u32 *)0x80000100 = MAKE_J((int)GSHandler);
	*(u32 *)0x80000104 = 0;
	ee_kmode_exit2();
	EI2();

	SetVCommonHandler(8, (void *)0x80000280); // TPIIG Fix
}


//Disable GSM
static inline void DeInitGSM(void)
{
	// Search for Syscall Table in ROM
	u32 i;
	u32 KernelStart;
	u32 *Pointer;
	u32 *SyscallTable;
	KernelStart = 0;
	for (i = 0x1fc00000 + 0x300000; i < 0x1fc00000 + 0x3fffff; i += 4)
	{
		if (*(u32 *)(i + 0) == 0x40196800)
		{
			if (*(u32 *)(i + 4) == 0x3c1a8001)
			{
				KernelStart = i - 8;
				break;
			}
		}
	}
	if (KernelStart == 0)
	{
		GS_BGCOLOUR = 0x00ffff; // Yellow
		while (1)
		{
			;
		}
	}
	Pointer = (u32 *)(KernelStart + 0x2f0);
	SyscallTable = (u32 *)((Pointer[0] << 16) | (Pointer[2] & 0xFFFF));
	SyscallTable = (u32 *)((u32)SyscallTable & 0x1fffffff);
	SyscallTable = (u32 *)((u32)SyscallTable + KernelStart);

	DI();
	ee_kmode_enter();
	// Restore SetGsCrt (even when it isn't hooked)
	SetSyscall(2, (void *)SyscallTable[2]);
	// Remove all breakpoints (even when they aren't enabled)
	__asm__ __volatile__(
		".set noreorder\n"
		".set noat\n"
		"li $k0, 0x8000\n"
		"mtbpc $k0\n" // All breakpoints off (BED = 1)
		"sync.p\n"	  // Await instruction completion
		".set at\n"
		".set reorder\n");
	ee_kmode_exit();
	EI();
}


//Automatically set GSM for PS1 HDMI
static inline void Set_PS1_GSM_Settings(int predef_vmode_idx, int x_offset, int y_offset)
{
	// Initialize the Graphics Synthesizer Mode
	InitGSM(predef_vmode[predef_vmode_idx].interlace,
			predef_vmode[predef_vmode_idx].mode,
			predef_vmode[predef_vmode_idx].ffmd,
			predef_vmode[predef_vmode_idx].display,
			predef_vmode[predef_vmode_idx].syncv,
			((predef_vmode[predef_vmode_idx].ffmd) << 1) | (predef_vmode[predef_vmode_idx].interlace),
			x_offset, // X Offset
			y_offset, // Y Offset
			0);		  // Skip videos

	// Call sceSetGsCrt syscall to apply the new video mode
	__asm__ __volatile__(
		"li  $3, 0x02\n"   // Syscall Number = 2 (sceGsCrt)
		"add $4, $0, %0\n" // interlace
		"add $5, $0, %1\n" // mode
		"add $6, $0, %2\n" // ffmd
		"syscall\n"		   // Perform the syscall
		"nop\n"			   // nop for Branch delay slot
		:
		: "r"(predef_vmode[predef_vmode_idx].interlace),
		  "r"(predef_vmode[predef_vmode_idx].mode),
		  "r"(predef_vmode[predef_vmode_idx].ffmd));
}




//----------------------------------------------------------------------------


/*
PS1VModeNeg by SP193 v1.10	https://www.psx-place.com/resources/ps1vmodeneg-by-sp193.853/
*/

static int file_exists(const char *file){
	int fd, result;
	if((fd=open(file, O_RDONLY))>=0){
		result=1;
		close(fd);
	}
	else result=0;

	return result;
}


static int get_CNF_string(char **CNF_p_p, char **name_p_p, char **value_p_p)
{
	char *np, *vp, *tp = *CNF_p_p;

start_line:
	while((*tp<=' ') && (*tp>'\0')) tp+=1;  //Skip leading whitespace, if any
	if(*tp=='\0') return 0;            		//but exit at EOF
	np = tp;                                //Current pos is potential name
	if(*tp<'A')                             //but may be a comment line
	{                                       //We must skip a comment line
		while((*tp!='\r')&&(*tp!='\n')&&(*tp>'\0')) tp+=1;  //Seek line end
		goto start_line;                    //Go back to try next line
	}

	while((*tp>='A')||((*tp>='0')&&(*tp<='9'))) tp+=1;  //Seek name end
	if(*tp=='\0') return 0;          		//but exit at EOF

	while((*tp<=' ') && (*tp>'\0'))
		*tp++ = '\0';                       //zero&skip post-name whitespace
	if(*tp!='=') return 0;	                //exit (syntax error) if '=' missing
	*tp++ = '\0';                           //zero '=' (possibly terminating name)

	while((*tp<=' ') && (*tp>'\0')          //Skip pre-value whitespace, if any
		&& (*tp!='\r') && (*tp!='\n')		//but do not pass the end of the line
		&& (*tp!='\7')     					//allow ctrl-G (BEL) in value
		)tp+=1;								
	if(*tp=='\0') return 0;          		//but exit at EOF
	vp = tp;                                //Current pos is potential value

	while((*tp!='\r')&&(*tp!='\n')&&(*tp!='\0')) tp+=1;  //Seek line end
	if(*tp!='\0') *tp++ = '\0';             //terminate value (passing if not EOF)
	while((*tp<=' ') && (*tp>'\0')) tp+=1;  //Skip following whitespace, if any

	*CNF_p_p = tp;                          //return new CNF file position
	*name_p_p = np;                         //return found variable name
	*value_p_p = vp;                        //return found variable value
	return 1;                           	//return control to caller
}	//Ends get_CNF_string


int Read_SYSTEM_CNF(char *boot_path, char *ver)
{
	// Returns disc type : 0 = failed; 1 = PS1; 2 = PS2;
	size_t CNF_size;
	char *RAM_p, *CNF_p, *name, *value;
	int fd = -1;	
	int Disc_Type = -1;  // -1 = Internal : Not Tested; 

	//place 3 question mark in ver string				
	strcpy(ver, "???"); 

	fd = open("cdrom0:\\SYSTEM.CNF;1", O_RDONLY);
	if (fd < 0) {
failed_load:
		if (Disc_Type == -1) { 
			// Test PS1 special cases
			if (file_exists("cdrom0:\\PSXMYST\\MYST.CCS;1")) {
				strcpy(boot_path, "SLPS_000.24");
				Disc_Type = 1;
			}
			else if (file_exists("cdrom0:\\CDROM\\LASTPHOT\\ALL_C.NBN;1")) {
				strcpy(boot_path, "SLPS_000.65");
				Disc_Type = 1;
			}
			else if (file_exists("cdrom0:\\PSX.EXE;1")) {
				//place 3 question mark in pathname
				strcpy(boot_path, "???"); 
				Disc_Type = 1;
			}
		}
		if (Disc_Type == -1) Disc_Type = 0;
		return Disc_Type;
	} // This point is only reached after succefully opening CNF	 

	Disc_Type = 0;
	
	CNF_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	RAM_p = (char*)malloc(CNF_size);
	CNF_p = RAM_p;
	if (CNF_p == NULL) {
		close(fd);
		goto failed_load;
	}
	read(fd, CNF_p, CNF_size); // Read CNF as one long string
	close(fd);
	CNF_p[CNF_size] = '\0'; // Terminate the CNF string

	strcpy(boot_path, "???"); //place 3 question mark in boot path
	
	while(get_CNF_string(&CNF_p, &name, &value)) {
		// A variable was found, now we dispose of its value.
		if (!strcmp(name,"BOOT2")) { // check for BOOT2 entry
			strcpy(boot_path, value);
			Disc_Type = 2;           // If found, PS2 disc type
			break; 
		}		
		if (!strcmp(name,"BOOT")) {  // check for BOOT entry
			strcpy(boot_path, value);
			Disc_Type = 1;			 // If found, PS1 disc type
			continue;
		}		
		if (!strcmp(name,"VER")) {   // check for VER entry
			strcpy(ver, value);
			continue;
		}		
	} // ends for
	free(RAM_p);
	return Disc_Type;
}


/* Some macros used for patching. */
#define JAL(addr)	(0x0c000000|(0x3ffffff&((addr)>>2)))
#define JMP(addr)	(0x08000000|(0x3ffffff&((addr)>>2)))
#define GETJADDR(addr)	((addr&0x03FFFFFF)<<2)

enum CONSOLE_REGIONS{
	CONSOLE_REGION_INVALID	= -1,
	CONSOLE_REGION_JAPAN	= 0,
	CONSOLE_REGION_USA,	//USA and HK/SG.
	CONSOLE_REGION_EUROPE,
	CONSOLE_REGION_CHINA,

	CONSOLE_REGION_COUNT
};

enum DISC_REGIONS{
	DISC_REGION_INVALID	= -1,
	DISC_REGION_JAPAN	= 0,
	DISC_REGION_USA,
	DISC_REGION_EUROPE,

	DISC_REGION_COUNT
};

static char ver[64], cdboot_path[256], romver[16];
static unsigned char SelectedVMode;
static void (*MainEPC)(void);
static char ConsoleRegion=CONSOLE_REGION_INVALID, DiscRegion=DISC_REGION_INVALID;


static unsigned short int GetBootROMVersion(void)
{
	int fd;
	char VerStr[5];

	fd=open("rom0:ROMVER", O_RDONLY);
	read(fd, romver, sizeof(romver));
	close(fd);
	VerStr[0]=romver[0];
	VerStr[1]=romver[1];
	VerStr[2]=romver[2];
	VerStr[3]=romver[3];
	VerStr[4]='\0';

	return((unsigned short int)strtoul(VerStr, NULL, 16));
}

static int GetConsoleRegion(void)
{
	int result;

	if((result=ConsoleRegion)<0){
		switch(romver[4]){
			case 'C':
				ConsoleRegion=CONSOLE_REGION_CHINA;
				break;
			case 'E':
				ConsoleRegion=CONSOLE_REGION_EUROPE;
				break;
			case 'H':
			case 'A':
				ConsoleRegion=CONSOLE_REGION_USA;
				break;
			case 'J':
				ConsoleRegion=CONSOLE_REGION_JAPAN;
		}

		result=ConsoleRegion;
	}

	return result;
}

static int HasValidDiscInserted(int mode)
{
	int DiscType, result;

	if(sceCdDiskReady(mode)==SCECdComplete){
		switch((DiscType=sceCdGetDiskType())){
			case SCECdDETCT:
			case SCECdDETCTCD:
			case SCECdDETCTDVDS:
			case SCECdDETCTDVDD:
				result=0;
				break;
			case SCECdUNKNOWN:
			case SCECdPSCD:
			case SCECdPSCDDA:
			case SCECdPS2CD:
			case SCECdPS2CDDA:
			case SCECdPS2DVD:
			case SCECdCDDA:
			case SCECdDVDV:
			case SCECdIllegalMedia:
				result=DiscType;
				break;
			default:
				result=-1;
		}
	}
	else{
		result=-1;
	}

	return result;
}


static void InvokeSetGsCrt(void)
{	/*	This is necessary because PS1DRV runs off the initial state that LoadExecPS2 leaves the console in.
		Therefore it is necessary to invoke SetGsCrt with the desired video mode,
		so that the registers that aren't set by PS1DRV will be in the correct state.

		For example, the PLL mode (54.0MHz for NTSC or 53.9MHz for PAL, for applicable console models that have a PLL setting)
			is only set within SetGsCrt, but not in PS1DRV. */
	SetGsCrt(1, SelectedVMode, 1);
	MainEPC();
}

static int GetDiscRegion(const char *path)
{
	int region;
	const char *name;

	region = DISC_REGION_INVALID;
	if(((name = strrchr(path, '/')) != NULL) || ((name = strrchr(path, '\\')) != NULL) || ((name = strrchr(path, ':')) != NULL))
	{	//Filename found.
		++name;
		if(strlen(name) >= 3)
		{	/*	This may not be the best way to do it, but it seems like the 3rd letter typically indicates the region:
				SxPx - Japan
				SxUx - USA
				SxEx - Europe	*/
			switch(name[2])
			{
				case 'P':
					region = DISC_REGION_JAPAN;
					break;
				case 'U':
					region = DISC_REGION_USA;
					break;
				case 'E':
					region = DISC_REGION_EUROPE;
					break;
			}
		}
	}

	return region;
}


//pretty print
const char* GetConsoleRegionString(int region) {
    switch(region) {
        case CONSOLE_REGION_CHINA:
            return "China";
        case CONSOLE_REGION_EUROPE:
            return "Europe";
        case CONSOLE_REGION_USA:
            return "USA";
        case CONSOLE_REGION_JAPAN:
            return "Japan";
        default:
            return "Unknown";
    }
}

const char* GetDiscRegionString(int region) {
    switch(region) {
        case DISC_REGION_JAPAN:
            return "Japan";
        case DISC_REGION_USA:
            return "USA";
        case DISC_REGION_EUROPE:
            return "Europe";
        default:
            return "Invalid or Unknown";
    }
}


//-------------------------------------------------------------


void run_program() {
    char *args[3];
    t_ExecData ElfData;
    int DiscType, done;
    unsigned short int ROMVersion;

    // Initialize console and screen
    init_scr();
    SifInitRpc(0);
    fioInit();
    sceCdInit(SCECdINoD);

    scr_printf("\n\n"
               "PS1 HDMI/Component FIX - by PhilRoll\n"
               "====================================\n\n");

    done = 0;
    do {
        if ((DiscType = HasValidDiscInserted(1)) >= 0) {
            if (DiscType == 0) {
                scr_printf("Reading disc...");
                while (HasValidDiscInserted(1) == 0) { 
					sleep(1);
				}
            } 
			else {
                switch (DiscType) {
                    case SCECdPSCD:
                    case SCECdPSCDDA:
                        done = 1;
                        break;
                    default:
                        scr_printf("Error! Please insert a PlayStation 1 game disc.\n");
                        sceCdStop();
                        sceCdSync(0);
                        while (HasValidDiscInserted(1) > 0) { 
							sleep(1);
						}
                }
            }
        } 
		else {
            scr_printf("No disc inserted. Please insert a PlayStation 1 game disc.\n");
            while (HasValidDiscInserted(1) < 0) { 
				sleep(1);
			}
        }
    } 
	while (!done);

	scr_clear();
	scr_printf("\n\n"
               "PS1 HDMI/Component FIX - by PhilRoll\n"
               "====================================\n\n");
    scr_printf("Reading disc...\n");

    sceCdDiskReady(0);
    DiscType = sceCdGetDiskType();

    // Check PS1 disc type and read CNF
    if (((DiscType == SCECdPSCD) || (DiscType == SCECdPSCDDA)) && (Read_SYSTEM_CNF(cdboot_path, ver) == 1)) {
        args[0] = "rom0:PS1DRV";
        args[1] = cdboot_path;  // 1st arg is main ELF path
        args[2] = ver;          // 2nd arg is version

        DiscRegion = GetDiscRegion(cdboot_path);
        ROMVersion = GetBootROMVersion();
        GetConsoleRegion();

        scr_printf("Console: %s, Disc region: %s\n", GetConsoleRegionString(ConsoleRegion), GetDiscRegionString(DiscRegion));

        sleep(1);
        scr_printf("Run game:\n");
        sleep(1);

        FlushCache(0);

        if (SifLoadElf("rom0:PS1DRV", &ElfData) == 0) {
            fioExit();
            SifExitRpc();

            MainEPC = (void*)ElfData.epc;

            FlushCache(0);
            FlushCache(2);

            // CLEAR GSM CONFIG
            DeInitGSM();
            
            // GSM SELECTION
            if (DiscRegion == DISC_REGION_EUROPE) {
                Set_PS1_GSM_Settings(PAL_PREDEF_VMODE_IDX, PAL_X_OFFSET, PAL_Y_OFFSET);
            } else if (DiscRegion == DISC_REGION_USA) {
                Set_PS1_GSM_Settings(NTSC_PREDEF_VMODE_IDX, NTSC_X_OFFSET, NTSC_Y_OFFSET);
            } else {
                // If the region is neither Europe nor USA, print an error and restart the program
                scr_printf("Error: Unsupported disc region! Please insert a valid disc.\n");
				scr_printf("Restart ELF in 3");
				sleep(1);
				scr_printf("..2");
				sleep(1);
				scr_printf("..1");
				sleep(1);
				scr_printf("..DONE");
				scr_clear();
                // Call the function to restart the program
                run_program(); // Call run_program to restart
                return; // Exit the current function
            }

            ExecPS2(&InvokeSetGsCrt, (void*)ElfData.gp, 3, args);  // Won't return from here.
        }
    }

    fioExit();
    sceCdInit(SCECdEXIT);
    SifExitRpc();
}

//-------------------------------------------------------------



//MAIN

int main(int argc, char *argv[]) {
    run_program(); // Initialize the program by calling the new function
    return 0;
}
