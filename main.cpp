//
// Created by wtl on 2024/7/21.
//
#include "tgaimage.h"
#include "model.h"
#include <iostream>

const static TGAColor WHITE{255, 255, 255, 255};
const static TGAColor BLUE{255, 0, 0, 255};
const static TGAColor RED{0, 0, 255, 255};
const static TGAColor GREEN{0, 255, 0, 255};
const static int WIDTH = 800;
const static int HEIGHT = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, const TGAColor &color) {
    bool steep = false;
    if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror = std::abs(dy) * 2;
    int error = 0;
    for (int x = x0, y = y0; x <= x1; ++x) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error += derror;
        if (error > dx) {
            y += (y1 > y0 ? 1 : -1);
            error -= dx * 2;
        }
    }
}

/*
 * pts: A, B, C
 * P = iA +jB + (1-i-j)C
 * if i < 0 || b < 0 || 1-i-j < 0 -> false
 * else -> true
 *
 * calc i, j:
 * https://blog.csdn.net/wangjiangrong/article/details/115326930
 */
bool is_in_triangle(const Vec2i (&pts)[3], const Vec2i &P) {
    const Vec2i &A = pts[0];
    const Vec2i &B = pts[1];
    const Vec2i &C = pts[2];

    int i_up = -(P.x - B.x) * (C.y - B.y) + (P.y - B.y) * (C.x - B.x);
    int i_down = -(A.x - B.x) * (C.y - B.y) + (A.y - B.y) * (C.x - B.x);
    int j_up = -(P.x - C.x) * (A.y - C.y) + (P.y - C.y) * (A.x - C.x);
    int j_down = -(B.x - C.x) * (A.y - C.y) + (B.y - C.y) * (A.x - C.x);
    int k_up = i_down * j_down - i_up * j_down - i_down * j_up;
    int k_down = i_down * j_down;

    if ((i_up ^ i_down) < 0 || (j_up ^ j_down) < 0 || (k_up ^ k_down) < 0) {
        return false;
    }

    return true;
}

Vec3f barycentric(const Vec3i (&pts)[3], const Vec3i &P) {
    Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^
              Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
    /* `pts` and `P` has integer value as coordinates, that is, u has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z) < 1) return {-1, 1, 1};
    return {1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z};
}

void triangle(Vec3i (&pts)[3], float *z_buffer, TGAImage &image, const TGAColor &color) {
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);

    for (int i = 0; i < 3; ++i) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
            Vec3f bc_screen = barycentric(pts, P);  // 重心坐标
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            float Pz = pts[0].z * bc_screen.x + pts[1].z * bc_screen.y + pts[2].z * bc_screen.z;
            int idx = P.x + P.y * WIDTH;
            if (z_buffer[idx] < Pz) {
                z_buffer[idx] = Pz;
                image.set(P.x, P.y, color);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "param error: require an obj file" << std::endl;
        return -1;
    }

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
    Vec3f light_dir(0, 0, -1);
    float *z_buffer = new float[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        z_buffer[i] = -std::numeric_limits<float>::max();
    }

    Model *model = new Model(argv[1]);

    for (int i = 0; i < model->nfaces(); ++i) {
        std::vector<int> face = model->face(i);

        Vec3f world_coords[3];
        Vec3i screen_coords[3];
        for (int j = 0; j < 3; ++j) {
            world_coords[j] = model->vert(face[j]);

            screen_coords[j].x = (1 + world_coords[j].x) * WIDTH / 2;
            screen_coords[j].y = (1 + world_coords[j].y) * HEIGHT / 2;
        }
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;

        if (intensity > 0) {
            triangle(screen_coords, z_buffer, image,
                     {uint8_t(intensity * 255), uint8_t(intensity * 255), uint8_t(intensity * 255), 255});
        }
    }

    delete[] z_buffer;
    delete model;
    image.write_tga_file("output.tga");

    return 0;
}