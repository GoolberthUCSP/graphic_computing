#ifndef LIGHTHOUSE_H
#define LIGHTHOUSE_H

#include <map>
#include <vector>
#include "obj_model.h"

vector<Vec4f> colors{
    Vec4f(1.0f, 1.0f, 0.0f, 1.0f), // Yellow
    Vec4f(0.5f, 0.5f, 0.5f, 1.0f), // Gray
    Vec4f(1.0f, 0.0f, 0.0f, 1.0f), // Red
    Vec4f(1.0f, 1.0f, 1.0f, 1.0f), // White
    Vec4f(1.0f, 0.0f, 0.0f, 1.0f), // Red
    Vec4f(1.0f, 0.5f, 0.0f, 1.0f) // Orange
};

class Lighthouse : public ObjModel{
    Vec4f light_direction;
public:
    Lighthouse();

    void rotate_lantern(float angle);
    Vec4f get_light_direction(){ return light_direction; }
    Vec4f get_lantern_position(){ return drawables["lantern"]->get_centroid(); }

    void draw(GLuint &color_location);
    void calc_centroid();
};

Lighthouse::Lighthouse() : ObjModel(colors, "lighthouse.obj"),
    light_direction(Vec4f(-1.0f, 0.0f, 0.0f, 0.0f)){
    this->scaling(0.1f, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
}

void Lighthouse::draw(GLuint &color_location){
    ObjModel::draw(color_location);
    Vec4f a = drawables["light"]->get_centroid();
    Vec4f b = a + light_direction;
    a(3) = 1.0f; b(3) = 1.0f;
    Drawable light_vector({a, b}, {{GL_LINES, {0, 1}}}, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    light_vector.draw(color_location, GL_LINES);
}

void Lighthouse::rotate_lantern(float angle){
    Mtx4f transform = rotation_general(Vec4f(0.0f, 1.0f, 0.0f, 0.0f), angle);
    drawables["lantern"]->transform_mtx_centroid(transform);
    drawables["light"]->transform_mtx_point(transform, drawables["lantern"]->get_centroid());
    light_direction = (transform * light_direction).unit();
}

void Lighthouse::calc_centroid(){
    this->centroid = drawables["body_white"]->get_centroid();
    this->centroid = this->centroid + drawables["body_red"]->get_centroid();
    this->centroid = this->centroid / 2.0f;
    this->centroid(3) = 0.0f;
}

#endif