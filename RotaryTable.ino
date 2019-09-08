
/*****************************************************************************/
/*                                                                           */
/*                      CNC Indexer - Rotary Indexer                         */
/*                                                                           */
/*                          Wagner Mello May 2013                            */
/*                                                                           */
/*****************************************************************************/

//  v4: July 29 2013 : Bug backlash solved
//  V3: July 13 2013 : Bug newdeg%360 solved  
//  V2: July 12 2013 : Bug dJog=0 solved


#include <EEPROM.h>
#include "Wlcd.h"
#include "Wstepper.h"

Wlcd lcd = Wlcd(2);
WStepper stp = WStepper();

#define KRT  1
#define KUP  2
#define KDW  3
#define KLF  4
#define KSL  5

#define CW   0
#define CCW  1

/*****************************************************************************/

int Stt;

int Cur;
int Key;

int Div;

long pStp;                                    // Actual position (Steps)
long pDeg;                                    // Actual position (Degrees x 1000)
long cDeg;                                    // Calculated position (Degrees x 1000)

long sRot;                                    // Steps per revolution
long sBkl;                                    // Backlash (Steps)
float sVel;                                   // Velocity (Steps/Second)
float sAcc;                                   // Acceleration (Steps/Second²)

struct Cnf
{
	int Mod;                                     // Mode
	int Stp;                                     // Full steps per revolution 
	int Mst;                                     // Microstepping mode
	int Red;                                     // Reduction gears
	int Mvl;                                     // Maximum velocity (Degrees/Minute)
	int Acc;                                     // Acceleration (Degrees/Second²)
	int Bkl;                                     // Backlash (Degres x 1000)
	int nDiv;                                    // Division mode, number of divisions
	long dDeg;                                   // Degree mode, step (Degrees x 1000)
	long dJog;                                   // Jog mode step (Degrees x 1000)
}Cnf;


char mIni[8][17] =
{
 {"       ..       "},
 {"      .  .      "},
 {"     .    .     "},
 {"    .      .    "},
 {"   .        .   "},
 {"  .          .  "},
 {" .            . "},
 {".              ."}
};

char mMdd[5][12] =
{
 {"Division  "},
 {"Degree    "},
 {"Jog       "},
 {"Continuous"},
 {"Config    "}
};

char uChr[7][8] =
{
 {0x04,0x0a,0x04,0x00,0x00,0x00,0x00,0x00},                // 1 Degree
 {0x04,0x0e,0x1f,0x15,0x04,0x04,0x04,0x00},                // 2 Up Arrow
 {0x04,0x04,0x04,0x15,0x1f,0x0e,0x04,0x00},                // 3 Down Arrow
 {0x00,0x06,0x0c,0x1f,0x0c,0x06,0x00,0x00},                // 4 Left Arrow
 {0x00,0x0c,0x06,0x1f,0x06,0x0c,0x00,0x00},                // 5 Right Arrow
 {0x01,0x01,0x05,0x09,0x1f,0x08,0x04,0x00},                // 6 Return
 {0x06,0x09,0x02,0x04,0x0f,0x00,0x00,0x00}                 // 7 Power 2
};


/*****************************************************************************/


void setup()
{
	Stt = 0;
	Key = 0;
	ReadCnf();
	lcd.init();
	IniUsrChar();
	Cnf.Mod = max(0, min(Cnf.Mod, 4));
	Cnf.Stp = max(1, min(Cnf.Stp, 999));
	Cnf.Mst = max(1, min(Cnf.Mst, 999));
	Cnf.Red = max(1, min(Cnf.Red, 999));
	Cnf.Mvl = max(1, min(Cnf.Mvl, 9999));
	Cnf.Acc = max(1, min(Cnf.Acc, 999));
	Cnf.Bkl = max(0, min(Cnf.Bkl, 9999));
	Cnf.nDiv = max(1, min(Cnf.nDiv, 999));
	Cnf.dDeg = max(1, min(Cnf.dDeg, 359999));
	Cnf.dJog = max(1, min(Cnf.dJog, 100000));
	sRot = (long)Cnf.Stp * (long)Cnf.Mst * (long)Cnf.Red;
	sBkl = (lround)((float)Cnf.Bkl / 360000.0 * float(sRot));
	sVel = (float)Cnf.Mvl / 360.0 * float(sRot) / 60.0;
	sAcc = (float)Cnf.Acc / 360.0 * float(sRot);
	stp.Init(sVel, sAcc);
	f00();
}


void loop()
{
	int k;

	if ((k = GetKey()) != 0)
		switch (Stt)
		{
		case 00:f00(); break;
		case 10:f10(); break;
		case 11:f11(k); break;
		case 20:f20(); break;
		case 21:f21(k); break;
		case 22:f22(); break;
		case 23:f23(k); break;
		case 30:f30(); break;
		case 31:f31(k); break;
		case 32:f32(); break;
		case 33:f33(k); break;
		case 40:f40(); break;
		case 41:f41(k); break;
		case 50:f50(); break;
		case 51:f51(k); break;
		case 60:f60(); break;
		case 61:f61(k); break;
		case 62:f62(); break;
		case 63:f63(k); break;
		case 64:f64(); break;
		case 65:f65(k); break;
		case 66:f66(); break;
		case 67:f67(k); break;
		case 68:f68(); break;
		case 69:f69(k); break;
		case 70:f70(); break;
		case 71:f71(k); break;
		}
}


/*****************************************************************************/


void f00(void)                               // Start Screen
{
	int i, j;

	lcd.clear();
	lcd.cursor(0);
	lcd.print("  CNC  Indexer");
	for (i = 0; i < 24; i++)
	{
		j = ((i < 8) ? i : ((i < 16) ? 7 - (i - 8) : i - 16));
		lcd.cursorTo(1, 0);
		lcd.print(mIni[j]);
	}
	lcd.cursorTo(1, 0);
	lcd.print("          Start\6");
	Stt = 10;
}


void f10(void)                               // Mode Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "Mode:%s?", mMdd[Cnf.Mod]);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Bk \2Select\3 \6Ac");
	lcd.cursorTo(0, 15);
	lcd.cursor(1);
	Stt = 11;
}


void f11(int key)                            // Mode Selection
{
	switch (key)
	{
	case KUP:
		Cnf.Mod--;
		if (Cnf.Mod < 0)
			Cnf.Mod = 4;
		lcd.cursorTo(0, 5);
		lcd.print(mMdd[Cnf.Mod]);
		break;
	case KDW:
		Cnf.Mod++;
		if (Cnf.Mod > 4)
			Cnf.Mod = 0;
		lcd.cursorTo(0, 5);
		lcd.print(mMdd[Cnf.Mod]);
		break;
	case KLF:
		f00();                           // Start Screen
		break;
	case KRT:
	case KSL:
		SaveCnf();
		switch (Cnf.Mod)
		{
		case 0:f20(); break;
		case 1:f30(); break;
		case 2:f40(); break;
		case 3:f50(); break;
		case 4:f60(); break;
		}
		break;
	}
}


void f20(void)                               // Division Edit Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "nDiv:%.3d       ?", Cnf.nDiv);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 7);
	lcd.cursor(1);
	Stt = 21;
	Cur = 0;
}


void f21(int key)                            // Division Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.nDiv += 1;  break;
		case 1:Cnf.nDiv += 10; break;
		case 2:Cnf.nDiv += 100; break;
		}
		if (Cnf.nDiv >= 1000)
			Cnf.nDiv -= 1000;
		Cnf.nDiv = max(1, Cnf.nDiv);
		sprintf(buf, "%.3d", Cnf.nDiv);
		lcd.cursorTo(0, 5);
		lcd.print(buf);
		lcd.cursorTo(0, 7 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.nDiv -= 1;  break;
		case 1:Cnf.nDiv -= 10; break;
		case 2:Cnf.nDiv -= 100; break;
		}
		if (Cnf.nDiv <= 0)
			Cnf.nDiv += 1000;
		Cnf.nDiv = min(Cnf.nDiv, 999);
		sprintf(buf, "%.3d", Cnf.nDiv);
		lcd.cursorTo(0, 5);
		lcd.print(buf);
		lcd.cursorTo(0, 7 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 2)
			Cur = 0;
		lcd.cursorTo(0, 7 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 2;
		lcd.cursorTo(0, 7 - Cur);
		break;
	case KSL:
		SaveCnf();
		f22();
		break;
	}
}


void f22(void)                               // Division Exec Screen
{
	char buf[32];

	Div = 0;
	pStp = 0;
	pDeg = 0;
	cDeg = 0;
	lcd.clear();
	lcd.cursor(0);
	sprintf(buf, "Div:%d/%d", Div, Cnf.nDiv);
	lcd.print(buf);
	lcd.cursorTo(0, 13);
	lcd.print("\6Bk");
	lcd.cursorTo(1, 0);
	lcd.print("\4Index\5");
	lcd.cursorTo(1, 8);
	PrintDeg();
	Stt = 23;
}


void f23(int key)                            // Division Exec Key
{
	int f;
	char buf[16];

	f = -1;
	switch (key)
	{
	case KUP:
	case KRT:
		Div++;
		if (Div >= Cnf.nDiv)
			Div = 0;
		f = CW;
		break;
	case KDW:
	case KLF:
		Div--;
		if (Div < 0)
			Div = Cnf.nDiv - 1;
		f = CCW;
		break;
	case KSL:
		f10();
		break;
	}
	if (f >= 0)
	{
		lcd.cursorTo(0, 4);
		lcd.print(".......");
		lcd.cursorTo(1, 8);
		lcd.print("........");
		Index(Cnf.Mod, f, Div, Cnf.nDiv, Cnf.dDeg, Cnf.dJog);
		lcd.cursorTo(0, 4);
		lcd.print("       ");
		sprintf(buf, "%d/%d", Div, Cnf.nDiv);
		lcd.cursorTo(0, 4);
		lcd.print(buf);
		lcd.cursorTo(1, 8);
		PrintDeg();
	}
}


void f30(void)                               // Degree Edit Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "dDeg:%.3ld.%.3ld\1   ?", Cnf.dDeg / 1000, Cnf.dDeg % 1000);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 7);
	lcd.cursor(1);
	Stt = 31;
	Cur = 4;
}


void f31(int key)                            // Degree Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.dDeg += 1;     break;
		case 1:Cnf.dDeg += 10;    break;
		case 2:Cnf.dDeg += 100;   break;
		case 4:Cnf.dDeg += 1000;  break;
		case 5:Cnf.dDeg += 10000; break;
		case 6:Cnf.dDeg += 100000; break;
		}
		if (Cnf.dDeg >= 360000)
			Cnf.dDeg -= 360000;
		Cnf.dDeg = max(1, Cnf.dDeg);
		sprintf(buf, "%.3ld.%.3ld\1", Cnf.dDeg / 1000, Cnf.dDeg % 1000);
		lcd.cursorTo(0, 5);
		lcd.print(buf);
		lcd.cursorTo(0, 11 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.dDeg -= 1;     break;
		case 1:Cnf.dDeg -= 10;    break;
		case 2:Cnf.dDeg -= 100;   break;
		case 4:Cnf.dDeg -= 1000;  break;
		case 5:Cnf.dDeg -= 10000; break;
		case 6:Cnf.dDeg -= 100000; break;
		}
		if (Cnf.dDeg <= 0)
			Cnf.dDeg += 360000;
		Cnf.dDeg = min(Cnf.dDeg, 359999);
		sprintf(buf, "%.3ld.%.3ld\1", Cnf.dDeg / 1000, Cnf.dDeg % 1000);
		lcd.cursorTo(0, 5);
		lcd.print(buf);
		lcd.cursorTo(0, 11 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur == 3)
			Cur++;
		if (Cur > 6)
			Cur = 0;
		lcd.cursorTo(0, 11 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur == 3)
			Cur--;
		if (Cur < 0)
			Cur = 6;
		lcd.cursorTo(0, 11 - Cur);
		break;
	case KSL:
		SaveCnf();
		f32();
		break;
	}
}


void f32(void)                               // Degree Exec Screen
{
	char buf[32];

	Div = 0;
	pStp = 0;
	pDeg = 0;
	cDeg = 0;
	lcd.clear();
	lcd.cursor(0);
	sprintf(buf, "P:%dx%ld.%.3ld\1", Div, Cnf.dDeg / 1000, Cnf.dDeg % 1000);
	lcd.print(buf);
	lcd.cursorTo(0, 13);
	lcd.print("\6Bk");
	lcd.cursorTo(1, 0);
	lcd.print("\4Index\5");
	lcd.cursorTo(1, 8);
	PrintDeg();
	Stt = 33;
}


void f33(int key)                            // Degree Exec Key
{
	int f;
	char buf[16];

	f = -1;
	switch (key)
	{
	case KUP:
	case KRT:
		Div++;
		f = CW;
		break;
	case KDW:
	case KLF:
		Div--;
		f = CCW;
		break;
	case KSL:
		f10();
		break;
	}
	if (f >= 0)
	{
		lcd.cursorTo(0, 2);
		lcd.print("...........");
		lcd.cursorTo(1, 8);
		lcd.print("........");
		Index(Cnf.Mod, f, Div, Cnf.nDiv, Cnf.dDeg, Cnf.dJog);
		lcd.cursorTo(0, 2);
		lcd.print("           ");
		sprintf(buf, "%dx%ld.%.3ld\1", Div, Cnf.dDeg / 1000, Cnf.dDeg % 1000);
		lcd.cursorTo(0, 2);
		lcd.print(buf);
		lcd.cursorTo(1, 8);
		PrintDeg();
	}
}


void f40(void)                               // Jog Exec Screen
{
	char buf[32];

	Div = 0;
	pStp = 0;
	pDeg = 0;
	cDeg = 0;
	lcd.clear();
	lcd.cursor(0);
	sprintf(buf, "J\2\3:%.3ld.%.3ld\1", Cnf.dJog / 1000, Cnf.dJog % 1000);
	lcd.print(buf);
	lcd.cursorTo(0, 13);
	lcd.print("\6Bk");
	lcd.cursorTo(1, 0);
	lcd.print("\4Index\5");
	lcd.cursorTo(1, 8);
	PrintDeg();
	Stt = 41;
}


void f41(int key)                            // Jog Exec Key
{
	int f;
	char buf[16];

	f = -1;
	switch (key)
	{
	case KUP:
		Cnf.dJog *= 10;
		if (Cnf.dJog > 100000)
			Cnf.dJog = 1;
		sprintf(buf, "%.3ld.%.3ld\1", Cnf.dJog / 1000, Cnf.dJog % 1000);
		lcd.cursorTo(0, 4);
		lcd.print(buf);
		break;
	case KRT:
		Div++;
		f = CW;
		break;
	case KDW:
		Cnf.dJog /= 10;
		if (Cnf.dJog <= 0)
			Cnf.dJog = 100000;
		sprintf(buf, "%.3ld.%.3ld\1", Cnf.dJog / 1000, Cnf.dJog % 1000);
		lcd.cursorTo(0, 4);
		lcd.print(buf);
		break;
	case KLF:
		Div--;
		f = CCW;
		break;
	case KSL:
		SaveCnf();
		f10();
		break;
	}
	if (f >= 0)
	{
		lcd.cursorTo(1, 8);
		lcd.print("........");
		Index(Cnf.Mod, f, Div, Cnf.nDiv, Cnf.dDeg, Cnf.dJog);
		lcd.cursorTo(1, 8);
		PrintDeg();
	}
}


void f50(void)                               // Continuous Exec Screen
{
	char buf[32];

	Div = 0;
	pStp = 0;
	pDeg = 0;
	cDeg = 0;
	lcd.clear();
	lcd.cursor(0);
	lcd.print("Continuous");
	lcd.cursorTo(0, 13);
	lcd.print("\6Bk");
	lcd.cursorTo(1, 0);
	lcd.print("\4Index\5");
	lcd.cursorTo(1, 8);
	PrintDeg();
	Stt = 51;
}


void f51(int key)                            // Continuous Exec Key
{
	int f;
	char buf[16];

	f = -1;
	switch (key)
	{
	case KUP:
	case KRT:
		f = CW;
		break;
	case KDW:
	case KLF:
		f = CCW;
		break;
	case KSL:
		f10();
		break;
	}
	if (f >= 0)
	{
		lcd.cursorTo(1, 8);
		lcd.print("........");
		Index(Cnf.Mod, f, Div, Cnf.nDiv, Cnf.dDeg, Cnf.dJog);
		lcd.cursorTo(1, 8);
		PrintDeg();
	}
}


void f60(void)                               // Config Steps/Rev Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "Steps/Rev:%.3d  ?", Cnf.Stp);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 12);
	lcd.cursor(1);
	Stt = 61;
	Cur = 0;
}


void f61(int key)                            // Config Steps/Rev Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Stp += 1;  break;
		case 1:Cnf.Stp += 10; break;
		case 2:Cnf.Stp += 100; break;
		}
		if (Cnf.Stp >= 1000)
			Cnf.Stp -= 1000;
		Cnf.Stp = max(1, Cnf.Stp);
		sprintf(buf, "%.3d", Cnf.Stp);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Stp -= 1;  break;
		case 1:Cnf.Stp -= 10; break;
		case 2:Cnf.Stp -= 100; break;
		}
		if (Cnf.Stp <= 0)
			Cnf.Stp += 1000;
		Cnf.Stp = min(Cnf.Stp, 999);
		sprintf(buf, "%.3d", Cnf.Stp);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 2)
			Cur = 0;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 2;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KSL:
		f62();
		break;
	}
}


void f62(void)                               // Config MicroStep Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "MicroStep:%.3d  ?", Cnf.Mst);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 12);
	lcd.cursor(1);
	Stt = 63;
	Cur = 0;
}


void f63(int key)                            // Config MicroStep Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Mst += 1;  break;
		case 1:Cnf.Mst += 10; break;
		case 2:Cnf.Mst += 100; break;
		}
		if (Cnf.Mst >= 1000)
			Cnf.Mst -= 1000;
		Cnf.Mst = max(1, Cnf.Mst);
		sprintf(buf, "%.3d", Cnf.Mst);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Mst -= 1;  break;
		case 1:Cnf.Mst -= 10; break;
		case 2:Cnf.Mst -= 100; break;
		}
		if (Cnf.Mst <= 0)
			Cnf.Mst += 1000;
		Cnf.Mst = min(Cnf.Mst, 999);
		sprintf(buf, "%.3d", Cnf.Mst);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 2)
			Cur = 0;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 2;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KSL:
		f64();
		break;
	}
}


void f64(void)                               // Config Reduction Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "Reductn 1:%.3d  ?", Cnf.Red);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 12);
	lcd.cursor(1);
	Stt = 65;
	Cur = 0;
}


void f65(int key)                            // Config Reduction Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Red += 1;  break;
		case 1:Cnf.Red += 10; break;
		case 2:Cnf.Red += 100; break;
		}
		if (Cnf.Red >= 1000)
			Cnf.Red -= 1000;
		Cnf.Red = max(1, Cnf.Red);
		sprintf(buf, "%.3d", Cnf.Red);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Red -= 1;  break;
		case 1:Cnf.Red -= 10; break;
		case 2:Cnf.Red -= 100; break;
		}
		if (Cnf.Red <= 0)
			Cnf.Red += 1000;
		Cnf.Red = min(Cnf.Red, 999);
		sprintf(buf, "%.3d", Cnf.Red);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 2)
			Cur = 0;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 2;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KSL:
		f66();
		break;
	}
}


void f66(void)                               // Config MaxVel Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "MxVl(\1/M):%.4d ?", Cnf.Mvl);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 13);
	lcd.cursor(1);
	Stt = 67;
	Cur = 0;
}


void f67(int key)                            // Config MaxVel Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Mvl += 1;   break;
		case 1:Cnf.Mvl += 10;  break;
		case 2:Cnf.Mvl += 100; break;
		case 3:Cnf.Mvl += 1000; break;
		}
		if (Cnf.Mvl >= 10000)
			Cnf.Mvl -= 10000;
		Cnf.Mvl = max(1, Cnf.Mvl);
		sprintf(buf, "%.4d", Cnf.Mvl);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 13 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Mvl -= 1;   break;
		case 1:Cnf.Mvl -= 10;  break;
		case 2:Cnf.Mvl -= 100; break;
		case 3:Cnf.Mvl -= 1000; break;
		}
		if (Cnf.Mvl <= 0)
			Cnf.Mvl += 10000;
		Cnf.Mvl = min(Cnf.Mvl, 9999);
		sprintf(buf, "%.4d", Cnf.Mvl);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 13 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 3)
			Cur = 0;
		lcd.cursorTo(0, 13 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 3;
		lcd.cursorTo(0, 13 - Cur);
		break;
	case KSL:
		f68();
		break;
	}
}


void f68(void)                               // Config Acceleration Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "Acc(\1/s\7):%.3d  ?", Cnf.Acc);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 12);
	lcd.cursor(1);
	Stt = 69;
	Cur = 0;
}


void f69(int key)                            // Config Acceleration Edit Key
{
	char buf[16];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Acc += 1;   break;
		case 1:Cnf.Acc += 10;  break;
		case 2:Cnf.Acc += 100; break;
		}
		if (Cnf.Acc >= 1000)
			Cnf.Acc -= 1000;
		Cnf.Acc = max(1, Cnf.Acc);
		sprintf(buf, "%.3d", Cnf.Acc);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Acc -= 1;   break;
		case 1:Cnf.Acc -= 10;  break;
		case 2:Cnf.Acc -= 100; break;
		}
		if (Cnf.Acc <= 0)
			Cnf.Acc += 1000;
		Cnf.Acc = min(Cnf.Acc, 999);
		sprintf(buf, "%.3d", Cnf.Acc);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur > 2)
			Cur = 0;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur < 0)
			Cur = 2;
		lcd.cursorTo(0, 12 - Cur);
		break;
	case KSL:
		f70();
		break;
	}
}


void f70(void)                               // Config Backlash Screen
{
	char buf[32];

	lcd.clear();
	sprintf(buf, "Bcklsh(\1):%.1d.%.3d?", Cnf.Bkl / 1000, Cnf.Bkl % 1000);
	lcd.print(buf);
	lcd.cursorTo(1, 0);
	lcd.print("\4Sel\5 \2Edit\3 \6Ac");
	lcd.cursorTo(0, 14);
	lcd.cursor(1);
	Stt = 71;
	Cur = 0;
}


void f71(int key)                            // Config Backlash Edit Key
{
	char buf[8];

	switch (key)
	{
	case KUP:
		switch (Cur)
		{
		case 0:Cnf.Bkl += 1;   break;
		case 1:Cnf.Bkl += 10;  break;
		case 2:Cnf.Bkl += 100; break;
		case 4:Cnf.Bkl += 1000; break;
		}
		if (Cnf.Bkl >= 10000)
			Cnf.Bkl -= 10000;
		Cnf.Bkl = max(0, Cnf.Bkl);
		sprintf(buf, "%.1d.%.3d?", Cnf.Bkl / 1000, Cnf.Bkl % 1000);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 14 - Cur);
		break;
	case KDW:
		switch (Cur)
		{
		case 0:Cnf.Bkl -= 1;   break;
		case 1:Cnf.Bkl -= 10;  break;
		case 2:Cnf.Bkl -= 100; break;
		case 4:Cnf.Bkl -= 1000; break;
		}
		if (Cnf.Bkl < 0)
			Cnf.Bkl += 10000;
		Cnf.Bkl = min(Cnf.Bkl, 9999);
		sprintf(buf, "%.1d.%.3d?", Cnf.Bkl / 1000, Cnf.Bkl % 1000);
		lcd.cursorTo(0, 10);
		lcd.print(buf);
		lcd.cursorTo(0, 14 - Cur);
		break;
	case KLF:
		Cur++;
		if (Cur == 3)
			Cur++;
		if (Cur > 4)
			Cur = 0;
		lcd.cursorTo(0, 14 - Cur);
		break;
	case KRT:
		Cur--;
		if (Cur == 3)
			Cur--;
		if (Cur < 0)
			Cur = 4;
		lcd.cursorTo(0, 14 - Cur);
		break;
	case KSL:
		SaveCnf();
		sRot = (long)Cnf.Stp * (long)Cnf.Mst * (long)Cnf.Red;
		sBkl = (lround)((float)Cnf.Bkl / 360000.0 * float(sRot));
		sVel = (float)Cnf.Mvl / 360.0 * float(sRot) / 60.0;
		sAcc = (float)Cnf.Acc / 360.0 * float(sRot);
		stp.Init(sVel, sAcc);
		f10();
		break;
	}
}


/*****************************************************************************/


int GetKey(void)
{
	int k;

	k = lcd.getkey();
	if (k != Key)
	{
		delay(50);
		k = lcd.getkey();
		if (k != Key)
			return(Key = k);
	}
	return(0);
}


void IniUsrChar(void)
{
	int i;

	for (i = 0; i < 7; i++)
		lcd.customChr(i + 1, uChr[i]);
}


void PrintDeg(void)
{
	char buf[16];

	sprintf(buf, "%3ld.%.3ld\1", pDeg / 1000, pDeg % 1000);
	lcd.print(buf);
}


void ReadCnf(void)
{
	int i, n;
	unsigned char* p;

	n = sizeof(struct Cnf);
	p = (unsigned char*)& Cnf;
	for (i = 0; i < n; i++, p++)
		* p = EEPROM.read(i);
}


void SaveCnf(void)
{
	int i, n;
	unsigned char* p;

	n = sizeof(struct Cnf);
	p = (unsigned char*)& Cnf;
	for (i = 0; i < n; i++, p++)
		if (EEPROM.read(i) != *p)
			EEPROM.write(i, *p);
}


/*****************************************************************************/


void Index(int mod, int dir, int div, int ndiv, long ddeg, long djog)
{
	long stprot;
	long clcdeg, newdeg;
	long curpos, newpos;
	long stptgo, stpgon;

	stprot = sRot;
	clcdeg = cDeg;
	curpos = pStp;
	switch (mod)
	{
	case 0:newdeg = lround(360000.0 / (float)ndiv * (float)div); break;
	case 1:newdeg = ddeg * (long)div;                         break;
	case 2:newdeg = ((dir) ? (clcdeg - djog) : (clcdeg + djog));    break;
	}
	if (mod < 3)
	{
		newdeg %= 360000L;
		newpos = lround((float)newdeg / 360000.0 * (float)stprot);
	}
	if ((mod > 2) || (newpos != curpos))
	{
		if (mod < 3)
		{
			if (dir)
				stptgo = curpos - newpos;
			else
				stptgo = newpos - curpos;
			if (stptgo < 0)
				stptgo += stprot;
			if (stptgo >= stprot)
				stptgo -= stprot;
			if (dir)
				stptgo += sBkl;
		}
		else
			stptgo = 1E9;
		stp.Set(dir, stptgo);
		if (mod < 3)
			while (stp.Run());
		else
		{
			while (lcd.digkey())
				stp.Run();
			stp.Stop();
			while (stp.Run());
		}
		stpgon = stp.Gone();
		if (dir)
			if (sBkl > 0)
			{
				stptgo = min(stpgon, sBkl);
				if (stptgo)
				{
					stp.Set(0, stptgo);
					while (stp.Run());
					stpgon -= stp.Gone();
				}
			}
		if (dir)
			newpos = curpos - stpgon;
		else
			newpos = curpos + stpgon;
		if (newpos < 0)
			newpos += stprot;
		if (newpos >= stprot)
			newpos -= stprot;
		pDeg = lround((float)newpos / (float)stprot * 360000.0);
		pStp = newpos;
	}
	cDeg = newdeg;
}