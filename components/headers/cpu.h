#pragma one

extern unsigned char memory[4096];
extern bool drawFlag;
extern unsigned char gfx[];

void initializeCpu();
void emulateCycle();
void updateTimers();