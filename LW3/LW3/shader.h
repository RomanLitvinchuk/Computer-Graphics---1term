#ifndef __SHADER_H__
#define __SHADER_H__

#include "tgaimage.h"
#include "geometry.h"

struct OShader {
    virtual ~OShader() {};
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

#endif //__SHADER_H__
