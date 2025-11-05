#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "camera.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;
Vec3f light_dir = Vec3f(1, -1, 1).normalize();
Vec3f eye(3, 1, 3);
Vec3f center(0, 0, 0);

void triangle(Vec3i t0, Vec2i uv0, Vec3i t1, Vec2i uv1, Vec3i t2, Vec2i uv2, TGAImage& image, float it0, float it1, float it2, int* zbuffer) {
    if (t0.y == t1.y && t0.y == t2.y) return;
    if (t0.y > t1.y) std::swap(t0, t1); std::swap(uv0, uv1);
    if (t0.y > t2.y) std::swap(t0, t2); std::swap(uv0, uv2);
    if (t1.y > t2.y) std::swap(t1, t2); std::swap(uv1, uv2);
    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
        Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
        Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
        float ityA = it0 + (it2 - it0) * alpha;
        Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
        Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
        float ityB = second_half ? it1 + (it2 - it1) * beta : it0 + (it1 - it0) * beta;
        if (A.x > B.x) std::swap(A, B); std::swap(uvA, uvB); std::swap(ityA, ityB);
        for (int j = A.x; j <= B.x; j++) {
            float phi = B.x == A.x ? 1. : (float)(j - A.x)/(B.x - A.x);
            Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
            Vec2i uvP = uvA + (uvB - uvA) * phi;
            float ityP = ityA + (ityB - ityA) * phi;
            int idx = P.x + P.y * width;
            if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, color*ityP);
            }
        }
    }
}

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    zbuffer = new int[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = std::numeric_limits<int>::min();
    }

    {
        Camera camera(eye, center);
        Matrix Projection = camera.projection(eye, center);
        Matrix ViewPort = camera.viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4, depth);
        Matrix ModelView = camera.lookat(eye, center, Vec3f(0, 1, 0));

        std::cerr << ModelView << std::endl;
        std::cerr << Projection << std::endl;
        std::cerr << ViewPort << std::endl;


        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            float intensity[3];
            Vec2i uv[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(face[j]);
                screen_coords[j] = m2v(ViewPort * Projection * ModelView * v2m(v));
                world_coords[j] = v;
                intensity[j] = model->norm(i, j) * light_dir;
                uv[j] = model->uv(i, j);
            }
            triangle(screen_coords[0], uv[0], screen_coords[1], uv[1], screen_coords[2], uv[2], image, intensity[0], intensity[1], intensity[2], zbuffer);
        }

        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    {
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete[] zbuffer;
    return 0;
}