#ifndef FC_TERRAIN_H
#define FC_TERRAIN_H

void initTerrain(void);
void drawTerrain(void);
void loadTerrainTextures();
int loadHeightMap(char *filename); // ...description at definition
int loadMapTextureIndices(char *filename); // ...description at definition
double getTerrainHeight(struct subterrain *ite, double x, double z);
int waitMs(double tt_ms);
#endif
