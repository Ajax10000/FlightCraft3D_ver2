#ifndef FC_AIRPLANE_H
#define FC_AIRPLANE_H

void initPoints(void);
void initAirplaneColors(void);
void loadAirplaneModel(void);
// polyhedron definition importing from simple text file containing list of coordinate triplets. 
// does same for face definition and colors and texture orderting if needed.
void importAirplanePolyhedron(void); 
// misc for import 3D models.
long int countNumbersInFile(char filename[]);
void getFloatsInFile(char filename[], float floatsRead[], long int maxsize);

void checkForPlaneCollision(void);
void drawAirplane(float h);
#endif