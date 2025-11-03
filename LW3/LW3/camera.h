#ifndef CAMERA_H__
#define CAMERA_H__

#include "geometry.h"


class Camera 
{
private:
	Vec3f eye_;
	Vec3f center_;
public:
	Camera(Vec3f eye, Vec3f center) : eye_(eye), center_(center){}
	Matrix viewport(int x, int y, int w, int h, int depth);
	Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);
	Matrix projection(Vec3f eye, Vec3f center);
};


#endif CAMERA_H__