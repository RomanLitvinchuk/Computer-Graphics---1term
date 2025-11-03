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
Vec3f light_dir(0, 0, -1);
Vec3f eye(3, 1, 3);
Vec3f center(0, 0, 0);

Matrix viewport2(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

Matrix lookat2(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}


void triangle(Vec3i t0, Vec2i uv0, Vec3i t1, Vec2i uv1, Vec3i t2, Vec2i uv2, TGAImage& image, int intensity, int* zbuffer) {
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
        Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
        Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
        if (A.x > B.x) std::swap(A, B); std::swap(uvA, uvB);
        for (int j = A.x; j <= B.x; j++) {
            float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
            Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
            Vec2i uvP = uvA + (uvB - uvA) * phi;
            int idx = P.x + P.y * width;
            if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                color.r = color.r;
                color.g = color.g;
                //color.b *= intensity;
                image.set(P.x, P.y, color);
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

        Matrix Projection2 = Matrix::identity(4);
        Matrix ModelView2 = lookat2(eye, center, Vec3f(0, 1, 0));
        Matrix ViewPort2 = viewport2(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        Projection[3][2] = -1.f / (eye - center).norm();

        std::cerr << ModelView << std::endl;
        std::cerr << Projection << std::endl;
        std::cerr << ViewPort << std::endl;
        Matrix z = (ViewPort * Projection * ModelView);
        std::cerr << z << std::endl;


        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            std::vector<int> face_texture = model->face_texture(i);
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            Vec2f texture_coords[3];
            Vec2i uv[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(face[j]);
                texture_coords[j] = model->texture_vert(face_texture[j]);
                uv[j] = Vec2i(texture_coords[j].x * model->diffusemap_.get_width(), texture_coords[j].y * model->diffusemap_.get_height());
                screen_coords[j] = m2v(ViewPort * Projection * ModelView * v2m(v));
                world_coords[j] = v;
            }
            Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
            n.normalize();
            float intensity = n * light_dir;
            //if (intensity > 0) {
            triangle(screen_coords[0], uv[0], screen_coords[1], uv[1], screen_coords[2], uv[2], image, intensity, zbuffer);
            //}
        }

        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    {
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete[] zbuffer;
    return 0;
}