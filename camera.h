
// class camera definition

class camera {
	float Mv[4][4], Mp[4][4]; // model view matrix, projection matrix
	float Mvp[4][4], Mn[3][3]; // model view projection matrix, normal matrix
	double up[3], right[3], fwd[3], pos[3]; // aux vectors and camera position
	void calcMatrixModelView();
public:
	void calcMatrixModelViewProjection();
	void perspective(double fovy, double ratiohw, double cnear, double cfar);
	float *getMatrixModelViewProjection();
	float *getMatrixNormal();
	float *getMatrixModelView();
	void LookAt(double x, double y, double z, double cx, double cy, double cz, double ux, double uy, double uz);
};

