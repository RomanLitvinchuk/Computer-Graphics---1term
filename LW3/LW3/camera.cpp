#include "camera.h"
#include "geometry.h"
Matrix Camera::lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix res = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

Matrix Camera::projection(Vec3f eye, Vec3f center) {
    Matrix Projection = Matrix::identity();
    Projection[3][2] = -1.f / (eye - center).norm();
    return Projection;
}

Matrix Camera::viewport(int x, int y, int w, int h, int depth) {
    Matrix m = Matrix::identity();
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}