/*

Modified by Wagner May 25, 2013

LCD4Bit v0.1 16/Oct/2006 neillzero http://abstractplain.net

What is this?
An arduino library for comms with HD44780-compatible LCD, in 4-bit mode (saves pins)

Sources:
- The original "LiquidCrystal" 8-bit library and tutorial
	http://www.arduino.cc/en/uploads/Tutorial/LiquidCrystal.zip
	http://www.arduino.cc/en/Tutorial/LCDLibrary
- DEM 16216 datasheet http://www.maplin.co.uk/Media/PDFs/N27AZ.pdf
- Massimo's suggested 4-bit code (I took initialization from here) http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1144924220/8
See also:
- glasspusher's code (probably more correct): http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1160586800/0#0

Tested only with a DEM 16216 (maplin "N27AZ" - http://www.maplin.co.uk/Search.aspx?criteria=N27AZ)
If you use this successfully, consider feeding back to the arduino wiki with a note of which LCD it worked on.

Usage:
see the examples folder of this library distribution.

*/

#include "Wlcd.h"
#include <string.h> //needed for strlen()
#include <inttypes.h>

//command bytes for LCD
#define CMD_CLAR 0x01
#define CMD_RGHT 0x1C
#define CMD_LEFT 0x18
#define CMD_HOME 0x02
#define CMD_BLNK 0x09
#define CMD_CTRL 0x0C


int KA = 0;
int KD0 = 11;
int KD1 = 12;

int EN = 9;
int RS = 8;
int DB[] = { 4,5,6,7 };
int KB[] = { 30,150,360,535,760 };

//--------------------------------------------------------

//how many lines has the LCD? (don't change here - specify on calling constructor)
int GnLines = 2;


//constructor.  num_lines must be 1 or 2, currently.

Wlcd::Wlcd(int nLines)
{
	GnLines = max(1, min(nLines, 2));
}


// initiatize lcd after a short pause
//while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()

void Wlcd::init()
{
	pinMode(EN, OUTPUT);
	pinMode(RS, OUTPUT);
	pinMode(DB[0], OUTPUT);
	pinMode(DB[1], OUTPUT);
	pinMode(DB[2], OUTPUT);
	pinMode(DB[3], OUTPUT);
	digitalWrite(EN, LOW);
	delay(50);
	//The first 4 nibbles and timings are not in my DEM16217 SYH datasheet, but apparently are HD44780 standard...
	commandWriteNibble(0x03);
	delay(5);
	commandWriteNibble(0x03);
	delayMicroseconds(100);
	commandWriteNibble(0x03);
	delay(5);
	// needed by the LCDs controller
	//this being 2 sets up 4-bit mode.
	commandWriteNibble(0x02);
	commandWriteNibble(0x02);
	//todo: make configurable by the user of this library.
	//NFXX where
	//N = num lines (0=1 line or 1=2 lines).
	//F= format (number of dots (0=5x7 or 1=5x10)).
	//X=don't care
	int num_lines_ptn = GnLines - 1 << 3;
	int dot_format_ptn = 0x00;      //5x7 dots.  0x04 is 5x10

	commandWriteNibble(num_lines_ptn | dot_format_ptn);
	delayMicroseconds(60);

	//The rest of the init is not specific to 4-bit mode.
	//NOTE: we're writing full bytes now, not nibbles.

	// display control: display on, cursor off, no blinking
	commandWrite(CMD_CTRL);
	delayMicroseconds(60);
	//clear display
	commandWrite(0x01);
	delay(3);

	// entry mode set: 06
	// increment automatically, display shift, entire shift off
	commandWrite(0x06);

	delay(1);//TODO: remove unnecessary delays
}


//send the clear screen command to the LCD

void Wlcd::clear()
{
	commandWrite(CMD_CLAR);
}


//write the given character at the current cursor position. overwrites, doesn't insert.

void Wlcd::write(char value)
{
	digitalWrite(RS, HIGH);                 //set the RS pin to show we're writing data
	pushByte(value);
}


//print the given string to the LCD at the current cursor position.

void Wlcd::print(char* msg)
{
	char* p;

	for (p = msg; *p; p++)
		write(*p);
}


//turns cursor blinking on/off

void Wlcd::cursor(int atv)
{
	commandWrite(CMD_CTRL | atv);
}


//move the cursor to the given absolute position.  line numbers start at 1.
//if this is not a 2-line LCD4Bit_mod instance, will always position on first line.

void Wlcd::cursorTo(int y, int x)
{
	int i;

	commandWrite(CMD_HOME);          //first, put cursor home
	y = min(y, GnLines - 1);              //if we are on a 1-line display, set line_num to 1st line, regardless of given
	if (y)
		x += 40;                          //offset 40 chars in if second line requested
	for (i = 0; i < x; i++)
		commandWrite(0x14);             //advance the cursor to the right according to position. (second line starts at position 40).
}


//creates custom character

void Wlcd::customChr(char chr, char* ptrn)
{
	int i;
	char c;

	c = 0x40 | (chr << 3);                 // baseAddress = 64 | (ascii << 3);
	commandWrite(c);
	for (i = 0; i < 8; i++)
		write(ptrn[i]);
	commandWrite(0x80);
}


void Wlcd::commandWrite(int value)
{
	digitalWrite(RS, LOW);
	pushByte(value);
	delay(1);
}


void Wlcd::commandWriteNibble(char nibble)
{
	digitalWrite(RS, LOW);
	pushNibble(nibble);
	delay(1);
}


int Wlcd::getkey(void)
{
	int a, k;

	a = analogRead(KA);
	for (k = 0; k < 5; k++)
		if (a < KB[k])
			return(k + 1);
	return(0);
}


int Wlcd::digkey(void)
{
	if (digitalRead(KD0) && digitalRead(KD1))
		return(0);
	return(1);
}


//********************** Nom Public ************************

//push a byte of data through the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.

void Wlcd::pushByte(char byte)
{
	char lower;
	char upper;

	upper = byte >> 4;
	lower = byte & 0x0F;
	pushNibble(upper);
	pushNibble(lower);
}


//push a nibble of data through the the LCD's DB4~7 pins, clocking with the Enable pin.

void Wlcd::pushNibble(char byte)
{
	int i;
	char nibble;

	nibble = byte & 0x0F;
	for (i = 0; i < 4; i++)
	{
		digitalWrite(DB[i], nibble & 01);
		nibble >>= 1;
	}
	pulseEnablePin();
}


//pulse the Enable pin high (for a microsecond).
//This clocks whatever command or data is in DB4~7 into the LCD controller.

void Wlcd::pulseEnablePin()
{
	digitalWrite(EN, HIGH);
	delayMicroseconds(1);
	digitalWrite(EN, LOW);
	delay(1);                    // pause 1 ms.  TODO: what delay, if any, is necessary here?
}

