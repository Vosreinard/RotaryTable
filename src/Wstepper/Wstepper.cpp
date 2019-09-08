/***************************************************************/
/*                                                             */
/*                          Wstepper                           */
/*                   Wagner Mello June/2013                    */
/*                Max Speed achieved: 9400 st/s                */
/*                                                             */
/*                    Based on AccelStepper                     */
/*                                                             */
/***************************************************************/


#include <Wstepper.h>

#define PDIR   2                   // Direction pin
#define PSTP   3                   // Step pin
#define CW     0                   // CW pin level
#define CCW    1                   // CCW pin level
#define PULSE  0                   // Active pulse
#define IDDLE  1                   // Iddle pulse
#define PULSEW 1                   // Pulse width 탎

WStepper::WStepper(void)
{
	pinMode(PDIR, OUTPUT);
	pinMode(PSTP, OUTPUT);
	digitalWrite(PDIR, CW);
	digitalWrite(PSTP, IDDLE);
}

void WStepper::Init(float speed, float accel)
{
	Speed = speed;
	Accel = accel;                                  // Acceleration           (st/s�)
	MxvlTime = 1E6 / speed;                           // Step time at max speed    (탎)
	InitTime = sqrt(2.0 / Accel) * 1E6 * 0.6763162;       // Initial step time         (탎)
}

void WStepper::Set(int dir, unsigned long steps)
{
	StepsToGo = steps;                              // Steps to run              (st)
	StepsGone = 0;                                  // Steps run                 (st)
	StepsRun = 0;                                   // Steps run / to decelerate (st)
	LastTime = 0;                                   // Last step time            (탎)
	NextTime = 0;                                   // Next step time            (탎)
	StepsToDc = (long)((Speed * Speed) / (2.0 * Accel));  // Steps to deceleration     (st)
	StepsToDc = min(StepsToDc, steps / 2);
	digitalWrite(PDIR, (dir) ? CCW : CW);              // Set direction pin
}

int WStepper::Run(void)
{
	unsigned long time;
	if (StepsToGo)
	{
		time = micros();
		if (((NextTime >= LastTime) && ((time >= NextTime) || (time < LastTime))) || ((NextTime < LastTime) && ((time >= NextTime) && (time < LastTime))))
		{
			Step();
			Calc();
			StepsToGo--;
			StepsGone++;
			StepsRun++;
			LastTime = time;
			NextTime = time + StepTime;
		}
		return(1);
	}
	return(0);
}





void WStepper::Stop()
{
	if (StepsToGo)
	{
		StepsToDc = min(StepsToDc, StepsGone);
		StepsToGo = StepsToDc + 1;                      // Steps to stop with deceleration
		StepsRun = -StepsToGo;
	}
}

void WStepper::Calc()
{
	if (StepsGone)
	{
		if ((StepsRun > 0) && (StepsToDc >= StepsToGo))          // Time to decelerate
			StepsRun = -StepsToDc;                             // Starts deceleration
		CalcTime -= ((2.0 * CalcTime) / ((4.0 * StepsRun) + 1));    // New step delay time
		CalcTime = max(CalcTime, MxvlTime);                  // Up to max velocity
	}
	else
		CalcTime = InitTime;                                 // First step
	StepTime = (long)CalcTime;
}

void WStepper::Step(void)
{
	digitalWrite(PSTP, PULSE);
	delayMicroseconds(PULSEW);
	digitalWrite(PSTP, IDDLE);
}

long WStepper::Gone(void)
{
	return(StepsGone);
}
