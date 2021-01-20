#ifndef FC_PHYSICS_H
#define FC_PHYSICS_H 

void initPhysicsVars(void);
void makeInertiaTensor(int n_vertexs); 
void simulatePhysics(int plane_up, int plane_down, int plane_inclleft, int plane_inclright, float h, double g);
double bounceAirplane(double rx, double ry, double rz,
					  double nx, double ny, double nz, double e);
void updateTorque(int plane_up, int plane_down, int plane_inclleft, int plane_inclright);
#endif
