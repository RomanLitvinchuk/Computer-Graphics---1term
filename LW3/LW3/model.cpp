#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

void Model::load_texture(std::string filename, TGAImage& img) {
    std::cerr << "texture file " << filename << " loading " << (img.read_tga_file(filename.c_str()) ? "ok" : "failed") << std::endl;
    img.flip_vertically();
}

Model::Model(const char* filename) : verts_(), faces_(), texture_verts_(), faces_texture_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Failed to open file" << std::endl;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> ft;
            int itrash, idx, idt;
            iss >> trash;
            while (iss >> idx >> trash >> idt >> trash >> itrash) {
                idx--; 
                idt--;
                f.push_back(idx);
                ft.push_back(idt);
            }
            faces_.push_back(f);
            faces_texture_.push_back(ft);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash;
            iss >> trash;
            Vec2f vt;
            for (int i = 0; i < 2; i++) iss >> vt[i];
            texture_verts_.push_back(vt);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << "# vt# " << texture_verts_.size() << "# ft# " << faces_texture_.size() << std::endl;
    load_texture("obj/african_head_diffuse.tga", diffusemap_);
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntexture_verts() {
    return (int)texture_verts_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::face_texture(int idt) {
    return faces_texture_[idt];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::texture_vert(int i) {
    return texture_verts_[i];
}

TGAColor Model::diffuse(Vec2i uv) {
    return diffusemap_.get(uv.x, uv.y);
}