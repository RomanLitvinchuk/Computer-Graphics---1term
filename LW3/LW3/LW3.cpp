#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "shader.h"
#include "camera.h"

Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;
Vec3f light_dir(1, 1, 1);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

struct Shader : public OShader {
    Vec3f varying_intensity; 
    mat<2, 3, float> varying_uv;
    Matrix VPM;

    void setVPM(const Matrix& vpm) {
        VPM = vpm;
    }

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
        gl_Vertex = VPM * gl_Vertex;     
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        float intensity = varying_intensity * bar;   
        Vec2f uv = varying_uv * bar;
        color = model->diffuse(uv) * intensity; 
        return false;                              
    }
};

void triangle(Vec4f* pts, OShader& shader, TGAImage& image, TGAImage& zbuffer) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
            if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
                
            }
        }
    }
}

void draw_background(TGAImage& image, Model* model, Matrix VPM) {
   for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
           float u = (float)x / width;
           float v = (float)y / height;
            TGAColor bg_color = model->background(u, v);
            image.set(x, y, bg_color);
        }
    }
}

void render_scene(const Vec3f& eye_position, const std::string& filename) {
    Camera camera(eye_position, center, up);
    Matrix VPM = camera.viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4, depth) *
        camera.projection(eye_position, center) *
        camera.lookat(eye_position, center, up);

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    draw_background(image, model);

    Shader shader;
    shader.setVPM(VPM);

    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file(filename.c_str());

    std::cout << "Rendered: " << filename << " from camera position ("
        << eye_position.x << ", " << eye_position.y << ", " << eye_position.z << ")" << std::endl;
}


/*int main(int argc, char** argv) {
    model = new Model("obj/african_head.obj");
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    draw_background(image, model);

    Shader shader;
    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically(); 
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}*/

int main(int argc, char** argv) {
    model = new Model("obj/african_head.obj");
    light_dir.normalize();

    std::vector<Vec3f> camera_positions = {
        Vec3f(3, 1, 3),  
        Vec3f(2, 1, 3),  
        Vec3f(1, 1, 3),  
        Vec3f(0, 1, 3)   
    };

    for (size_t i = 0; i < camera_positions.size(); i++) {
        std::string filename;
        if (i == 0) {
            filename = "output.tga";
        }
        else {
            filename = "output" + std::to_string(i + 1) + ".tga";
        }

        render_scene(camera_positions[i], filename);
    }

    delete model;
    return 0;
}