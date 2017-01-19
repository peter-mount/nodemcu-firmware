//////////////////////////////////////////////////
// rBoot open source boot loader for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifdef RBOOT_INTEGRATION
#include <rboot-integration.h>
#endif

#include "rboot-private.h"

usercode* NOINLINE load_rom(uint32 readpos) {
	
	uint8 sectcount;
	uint8 *writepos;
	uint32 remaining;
	usercode* usercode;
	
	rom_header header;
	section_header section;
	
	// read rom header
	SPIRead(readpos, &header, sizeof(rom_header));
	readpos += sizeof(rom_header);

	// create function pointer for entry point
	usercode = header.entry;
	
	// copy all the sections
	for (sectcount = header.count; sectcount > 0; sectcount--) {
		
		// read section header
		SPIRead(readpos, &section, sizeof(section_header));
		readpos += sizeof(section_header);

		// get section address and length
		writepos = section.address;
		remaining = section.length;
		
		while (remaining > 0) {
			// work out how much to read, up to 16 bytes at a time
			uint32 readlen = (remaining < 0x1000) ? remaining : 0x1000;
			// read the block
			SPIRead(readpos, writepos, readlen);
			readpos += readlen;
			writepos += readlen;
			// decrement remaining count
			remaining -= readlen;
		}
	}

	return usercode;
}

#ifdef BOOT_NO_ASM

void call_user_start(uint32 readpos) {
	usercode* user;
	user = load_rom(readpos);
	user();
}

#else

void call_user_start(uint32 readpos) {
	__asm volatile (
		"mov a15, a0\n"     // store return addr, we already splatted a15!
		"call0 load_rom\n"  // load the rom
		"mov a0, a15\n"     // restore return addr
		"jx a2\n"           // now jump to the rom code
	);
}

#endif
