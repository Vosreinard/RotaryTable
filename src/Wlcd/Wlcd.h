// Wlcd.h

#pragma once
class Wlcd
{
public:
	Wlcd(int);
	void init();
	void clear();
	void write(char);
	void print(char*);
	void cursor(int);
	void cursorTo(int, int);
	void customChr(char, char*);
	void commandWrite(int);
	void commandWriteNibble(char);
	int getkey(void);
	int digkey(void);
private:
	void pulseEnablePin();
	void pushNibble(char);
	void pushByte(char);
};

