
void MotorDelayInit(void);
void MotorDelay(void);

void motors(unsigned char);

void SolenoidActivate();
void SolenoidDeactivate();

void MotorStepXY(char, char);
void MotorGotoOrigin();
void MotorBresLine(int, int);

#define X_STEPS 3660    
#define Y_STEPS 3110

extern int Ax, Ay;

