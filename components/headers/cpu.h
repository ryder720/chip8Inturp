#pragma one

extern unsigned char memory[4096];
extern bool drawFlag;
extern unsigned char gfx[];
extern unsigned char dt;
extern unsigned char st;

void initializeCpu();
void emulateCycle();
void updateTimers();
void keyUp(int);
void keyDown(int);
void clearKeys();