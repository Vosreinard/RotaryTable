/***************************************************************/
/*                                                             */
/*                          Wstepper                           */
/*                   Wagner Mello June/2013                    */
/*                                                             */
/***************************************************************/

// Wstepper.h

class WStepper
{
public:
	WStepper(void);
	void Init(float, float);
	void Set(int, unsigned long);
	int Run(void);
	void Stop(void);
	long Gone(void);
private:
	void Step(void);
	void Calc(void);

	float Accel;             // Acceleration              (st/s�)
	float Speed;             // Current Speed             (st/s)

	unsigned long StepsToGo; // Steps to run              (st)
	unsigned long StepsGone; // Steps run                 (st)

	long StepsToDc;          // Steps to decelerate       (st)
	long StepsRun;           // Steps run / to decelerate (st)

	unsigned long LastTime;  // Last step time            (탎)
	unsigned long NextTime;  // Next step time            (탎)
	unsigned long StepTime;  // Step delay time           (탎)

	float MxvlTime;          // Step time at max speed    (탎)
	float InitTime;          // Initial step time         (탎)
	float CalcTime;          // Step delay time           (탎)

};
