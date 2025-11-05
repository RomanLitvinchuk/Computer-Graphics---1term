#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_;
	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
public:
	TGAImage diffusemap_;
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec2i uv(int iface, int nvert);
	Vec3f norm(int iface, int nvert);
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	TGAColor diffuse(Vec2i uv);
	void load_texture(std::string filename, TGAImage& img);
};

#endif //__MODEL_H__
