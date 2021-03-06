/*
 * Copyright (c) 2020 Anthony Beaucamp.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented * you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 */

#include "unity.h"

#ifdef __APPLE2__
  #pragma code-name("LC")
#endif

#if (defined __APPLE2__) || (defined __ORIC__)
  extern unsigned char sprDrawn[];
  void Scroll(void);
#elif defined __LYNX__	
  extern unsigned char charFlags[];
#endif

extern unsigned char inkColor;

unsigned char charmapX, charmapY, charmapWidth, charmapHeight;
unsigned char scrollCol1 = 0, scrollCol2 = CHR_COLS;
unsigned char scrollRow1 = 0, scrollRow2 = CHR_ROWS;

// Initialize Charmap Mode
void InitCharmap() 
{	
#if (defined __APPLE2__) || (defined __ORIC__)
	InitBitmap();
	
#elif defined __LYNX__
	InitDisplay();
#endif		
}

// Switch from Text mode to Charmap mode
void EnterCharmapMode()
{		
#if defined __APPLE2__	
	EnterBitmapMode();
	
#elif defined __ATARI__	
	// Setup Charmap DLIST
	CharmapDLIST();

	// Page to character set
	POKE(0x02f4, 0xa0);	
	
	// DLI parameters
	charmapDLI = 1; StartDLI();
	charsetPage1 = 0xa0;
	charsetPage2 = 0xa4;
	
#elif defined __CBM__	
	// Setup VIC2 (memory bank and multicolor mode)
	SetupVIC2();

#elif defined __LYNX__
	videoMode = MODE_CHARMAP;	
#endif	

	// Reset scroll coords (out of range)
	charmapX = 255; charmapY = 255;
}

// Switch back to Text mode
void ExitCharmapMode()
{
#if defined __APPLE2__	
	ExitBitmapMode();
	
#elif defined __CBM__	
	ResetVIC2();
#endif	
}

// Load charset and associated attributes / flags
void LoadCharset(char* filename)
{
#if defined __APPLE2__
	FILE* fp = fopen(filename, "rb");
	fread((char*)CHARSETRAM, 1, 0x0880, fp);
	fclose(fp);		
	
#elif defined __ATARI__
	if (FileOpen(filename)) {
		FileRead((char*)CHARSETRAM, 0x0800);
		FileRead((char*)CHARATRRAM, 0x0100);
	}
	
	// Set Colors
	POKE(0x02c8, 0x00);
	POKE(0x02c4, 0x0c);	
	POKE(0x02c5, 0x78);
	POKE(0x02c6, 0x62);
	POKE(0x02c7, 0x12);	// WARNING: This color is shared with sprite 5!
	
#elif defined __CBM__	
	FILE* fp = fopen(filename, "rb");	
	fread((char*)CHARSETRAM, 1, 0x0800, fp);
	fread((char*)CHARATRRAM, 1, 0x0100, fp);
	fclose(fp);	
	
	// Set Colors
	POKE(0xd021, BLACK);	// BG Color
	POKE(0xd022, WHITE); 	// MC1
	POKE(0xd023, LBLUE);	// MC2	
	
#elif defined __ORIC__
	FileRead(filename, (void*)CHARSETRAM);
#endif			
}

// Clear entire screen
void ClearCharmap()
{
#if (defined __APPLE2__) || (defined __ORIC__)
	ClearBitmap();

#elif defined __ATARI__
	bzero((char*)SCREENRAM, 1000);
	
#elif defined __CBM__
	bzero((char*)SCREENRAM, 1000);
	bzero((char*)COLORRAM,  1000);
	
#elif defined __LYNX__
	bzero((char*)SCREENRAM, 680);
	
#endif
}

// Load charmap from file
void LoadCharmap(char *filename) 
{
#if defined __ATARI__
	if (FileOpen(filename))
		FileRead((char*)CHARMAPRAM, 0x2000);
	
#elif (defined __APPLE2__) || (defined __CBM__)
	FILE* fp = fopen(filename, "rb");	
	fread((char*)CHARMAPRAM, 1, 0x2000, fp);
	fclose(fp);
	
#elif defined __LYNX__
	FileRead(filename);	
	
#elif defined __ORIC__
	FileRead(filename, (void*)CHARMAPRAM);	
#endif
	// Set Dimensions
	charmapWidth = 96;
	charmapHeight = 64;
}

unsigned char GetCharFlags(unsigned char x, unsigned char y)
{
	// Get flags of specified tile
	unsigned char chr;
	chr = PEEK(CHARMAPRAM + charmapWidth*y + x);
#if defined __LYNX__
	return charFlags[chr];
#else
	return PEEK(CHARFLGRAM + chr);
#endif
}

#if defined __APPLE2__
  #if defined __DHR__
    void __fastcall__ BlitDHR(void);
  #else
    void __fastcall__ BlitSHR(void);
  #endif
#endif

void ScrollCharmap(unsigned char x, unsigned char y)
{
	unsigned int src, dst, col;
	unsigned char i, j, k, l, chr;
	
	// Check if map was moved?
	if (x == charmapX && y == charmapY)
		return;
	charmapX = x;
	charmapY = y;

	// Address of first character
	src = CHARMAPRAM + charmapWidth*y + (x - scrollCol1);

#if defined __APPLE2__
	// Reset sprite background
	for (i=0; i<SPRITE_NUM; i++)
		sprDrawn[i] = 0;
	
	// Prepare blitter
	POKE(0xE3, 1);	// Bytes per line (x2 for MAIN/AUX)
	POKE(0xEB, 8);	// Number of lines
	POKEW(0xEE, 0);	// Address for copying Hires > Output
	
	// Blit characters to screen
	for (j=scrollRow1; j<scrollRow2; j++) {
		k = j*8;
		for (i=scrollCol1; i<scrollCol2; i+=2) {
			chr = PEEK(src+i);
			POKE(0xEC, i);  // Hires Offset X
			POKE(0xED, k);  // Hires Offset Y
			POKEW(0xFA, (char*)CHARSETRAM+chr*8);	// Address for copying Input > Hires
		  #if defined __DHR__	
			BlitDHR();
		  #else
			BlitSHR();  
		  #endif
			chr = PEEK(src+i+1);
			POKE(0xEC, i+1);  // Hires Offset X
			POKE(0xED, k);  // Hires Offset Y
			POKEW(0xFA, (char*)CHARSETRAM+1024+chr*8);	// Address for copying Input > Hires
		  #if defined __DHR__	
			BlitDHR();
		  #else
			BlitSHR();  
		  #endif
		}
		src += charmapWidth;
	}
	
#elif defined __ATARI__	
	dst = SCREENRAM + scrollRow1*40;
	for (j=scrollRow1; j<scrollRow2; j++) {
		for (i=scrollCol1; i<scrollCol2; i++) {
			chr = PEEK(src+i);
			k = PEEK(CHARATRRAM+chr);	// 0 or 128
			POKE(dst+i, chr+k);
		}
		src += charmapWidth;
		dst += 40;
	}
	
#elif defined __CBM__	
	// Switch Frame
	//charmapFrame ^= 1;
	dst = SCREENRAM + scrollRow1*40; // + 0x0400*charmapFrame;
	col = COLORRAM + scrollRow1*40;
	for (j=scrollRow1; j<scrollRow2; j++) {
		for (i=scrollCol1; i<scrollCol2; i++) {
			chr = PEEK(src+i);
			POKE(dst+i, chr);
			POKE(col+i, PEEK(CHARATRRAM+chr));
		}
		src += charmapWidth;
		dst += 40;
		col += 40;
	}
	
	// Update frame address (and sprite pointers)
	//POKE(0xD018, charmapFrame*16 + BITMAPLOC);
	//memcpy(SPRITEPTR + 0x0400*charmapFrame, spritePtr, 8);
	//spritePtr = SPRITEPTR + 0x0400*charmapFrame;
	
#elif defined __LYNX__	
	dst = SCREENRAM + scrollRow1*40;
	for (j=scrollRow1; j<scrollRow2; j++) {
		for (i=scrollCol1; i<scrollCol2; i++) {
			chr = PEEK(src+i);
			POKE(dst+i, chr);
		}
		src += charmapWidth;
		dst += 40;
	}
	
#elif defined __ORIC__	
	// Reset sprite background
	for (i=0; i<SPRITE_NUM; i++)
		sprDrawn[i] = 0;

	POKE(0xb0, scrollRow1); 		
	POKE(0xb1, scrollRow2);		
	POKE(0xb2, scrollCol1);		
	POKE(0xb3, scrollCol2);		
	POKE(0xb4, charmapWidth); 	// Offset between source lines
	POKEW(0xb5, src);			// Address of first charmap line
	POKEW(0xb7, BITMAPRAM + scrollRow1*320 + 1);	// Address of first bitmap line
	Scroll();	
	
	// Copy Map Section
/*	dst = BITMAPRAM + scrollRow1*320 + 1;
	for (j=scrollRow1; j<scrollRow2; j++) {
		col = CHARSETRAM;
		for (k=0; k<8; k++) {
			for (i=scrollCol1; i<scrollCol2; i++) {
				chr = PEEK(src+i);
				POKE(dst+i, PEEK(col+chr));
			}
			col += 128;
			dst += 40;
		}
		src += charmapWidth;
	}*/
#endif
}

void PrintCharmap(unsigned char x, unsigned char y, unsigned char* str)
{
	unsigned char i, chr;
	unsigned int addr1;
	
#if (defined __APPLE2__) || (defined __ORIC__)
	PrintStr(x, y, str);	
	
#elif (defined __ATARI__)
	addr1 = SCREENRAM + 40*y + x;
	for (i=0; i<strlen(str); i++) {
		if (str[i] > 96) 
			chr = str[i];		// Lower case
		else
			chr = str[i]+32;	// Upper case
		POKE(addr1++, chr);
	}
	
#elif defined __CBM__	
	unsigned int addr2, addr3;
	addr1 = SCREENRAM + 40*y + x;
	addr2 = SCREENRAM + 0x0400 + 40*y + x;
	addr3 = COLORRAM + 40*y + x;
	for (i=0; i<strlen(str); i++) {
		if (str[i] > 192)
			chr = str[i]+32;	// Upper case (petscii)
		else if (str[i] > 96) 
			chr = str[i]+128;	// Lower case (ascii)
		else
			chr = str[i]+160;	// Lower case (petscii)
		POKE(addr1++, chr);
		POKE(addr2++, chr);
		POKE(addr3++, inkColor);
	}
	
#elif defined __LYNX__
	addr1 = SCREENRAM + 40*y + x;
	for (i=0; i<strlen(str); i++) {
		if (str[i] > 96) 
			chr = str[i]+128;	// Lower case
		else
			chr = str[i]+160;	// Upper case
		POKE(addr1++, chr);
	}

#endif
}
