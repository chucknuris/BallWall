#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "camera.h"

void normalize3d(double *v) {
	double n = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	if (n != 0.0) {
		n = 1/sqrt(n);
		for (int i = 0; i != 3; i++)
			v[i] *= n;
	}
}

void crossproduct(double *prod, double *v1, double *v2) {
	prod[0] = v1[1]*v2[2] - v1[2]*v2[1];
	prod[1] = v1[2]*v2[0] - v1[0]*v2[2];
	prod[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

// set the projection matrix (Mp)
void camera::perspective(double fovy, double ratiohw, double cnear, double cfar) {
	double _t = 1/tan(fovy*(M_PI/360)); // tan of half fovy
	Mp[0][0] =  ratiohw*_t;
	Mp[1][1] =  _t;
	Mp[2][2] =  (cfar + cnear)/(cnear - cfar);
	Mp[2][3] =  2*cfar*cnear/(cnear - cfar);
	Mp[3][2] = -1;
	Mp[0][1] = Mp[0][2] = Mp[0][3] = Mp[1][0] = 0;
	Mp[1][2] = Mp[1][3] = Mp[2][0] = Mp[2][1] = 0;
	Mp[3][0] = Mp[3][1] = Mp[3][3] = 0;
}


void camera::LookAt(double x, double y, double z, double cx, double cy, double cz, double ux, double uy, double uz) {
	double u[3] = {ux, uy, uz};
	pos[0] = x; pos[1] = y; pos[2] = z;
	fwd[0] = cx - x; fwd[1] = cy - y; fwd[2] = cz - z;
	normalize3d(fwd);
	crossproduct(right, fwd, u);
	normalize3d(right);
	crossproduct(up, right, fwd);
	calcMatrixModelView();
}


void camera::calcMatrixModelView() {
	for (int i = 0; i != 3; i++) {
		Mn[0][i] = Mv[0][i] = right[i];
		Mn[1][i] = Mv[1][i] = up[i];
		Mn[2][i] = Mv[2][i] = -fwd[i];
		Mv[3][i] = 0;
	}
	Mv[0][3] = -right[0]*pos[0] - right[1]*pos[1] - right[2]*pos[2];
	Mv[1][3] = -up[0]*pos[0] - up[1]*pos[1] - up[2]*pos[2];
	Mv[2][3] = fwd[0]*pos[0] + fwd[1]*pos[1] + fwd[2]*pos[2];
	Mv[3][3] = 1;
	calcMatrixModelViewProjection();
}


void camera::calcMatrixModelViewProjection() {
	for (int j = 0; j != 4; j++)
		for (int i = 0; i != 4; i++) {
			Mvp[j][i] = 0;
			for (int k = 0; k != 4; k++)
				Mvp[j][i] += Mp[j][k]*Mv[k][i];
		}
}

float *camera::getMatrixModelViewProjection() {return Mvp[0];}
float *camera::getMatrixNormal() {return Mn[0];}
float *camera::getMatrixModelView() {return Mv[0];}

