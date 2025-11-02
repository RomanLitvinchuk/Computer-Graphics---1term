#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec2f> texture_verts_;
	std::vector<std::vector<int> > faces_texture_;
public:
	TGAImage diffusemap_;
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntexture_verts();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	std::vector<int> face_texture(int idt);
	Vec2f texture_vert(int i);
	TGAColor diffuse(Vec2i uv);
	void load_texture(std::string filename, TGAImage& img);
};

#endif //__MODEL_H__
