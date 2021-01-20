#ifndef FC_SPCL_EFFECTS
#define FC_SPCL_EFFECTS

// basic special effects in videogames
// add new explosion or just process those already started 
void addExplosionAtPoint(float x0, float y0, float z0, double dft, int option);

// add new explosion or just process those already started 
void addSmokeAtPoint(double x0, double y0, double z0, double dft, int option);

void launchProjectiles(float x, float y, float z, float vx, float vy, float vz, double dft, int do_add);
#endif