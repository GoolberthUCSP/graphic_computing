#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#include <map>
#include <vector>
#include <filesystem>
#include "drawable.h"

string obj_path = std::filesystem::absolute(__FILE__).string() + "/../../objects/";

class ObjModel{
protected:
    map<string, Drawable*> drawables;
    Vec4f centroid;
public:
    ObjModel(vector<Vec4f> colors, string filename);
    void scaling(float scale);
    void scaling(float scale, Vec4f point);
    void translation(Vec4f direction, float distance);
    void translation(Vec4f origin, Vec4f destiny);
    void translation(Vec4f destiny);
    void rotation_centroid(Vec4f axis, float angle);
    void rotation_point(Vec4f axis, float angle, Vec4f point);
    void draw(GLuint &color_location);
    void calc_centroid();
    Vec4f get_centroid(){ return centroid; }
};

ObjModel::ObjModel(vector<Vec4f> colors, string filename) {
    ifstream file(obj_path + filename);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo OBJ." << endl;
        return;
    }
    int color_idx = 0;
    unsigned int last_vertex = 0;
    string line, name;
    vector<Vec4f> vertices;
    map<int, vector<unsigned int>> indices;
    while (getline(file, line)){
        if (line.substr(0, 2) == "o ") {
            name = line.substr(2);
            break;
        }
    }
    while (getline(file, line)) {
        istringstream iss(line);
        string token;
        iss >> token;
        if (token == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Vec4f(x, -z, y, 1.0f));
        } else if (token == "f") {
            unsigned int v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            indices[GL_TRIANGLES].push_back(v1 - 1 - last_vertex);
            indices[GL_TRIANGLES].push_back(v2 - 1 - last_vertex);
            indices[GL_TRIANGLES].push_back(v3 - 1 - last_vertex);
        } else if (token == "o") {
            last_vertex += vertices.size();
            drawables[name] = new Drawable(vertices, indices, colors[color_idx++]);
            vertices.resize(0);
            indices[GL_TRIANGLES].resize(0);
            vertices.shrink_to_fit();
            indices[GL_TRIANGLES].shrink_to_fit();
            iss >> name;
        }
    }
    file.close();
    drawables[name] = new Drawable(vertices, indices, colors[color_idx]);

    calc_centroid();
}

void ObjModel::scaling(float scale){
    for (auto &drawable : drawables){
        drawable.second->scaling(Vec4f(scale, scale, scale, 1.0f), centroid);
    }
}

void ObjModel::scaling(float scale, Vec4f point){
    for (auto &drawable : drawables){
        drawable.second->scaling(Vec4f(scale, scale, scale, 1.0f), point);
    }
    calc_centroid();
}

void ObjModel::translation(Vec4f direction, float distance){
    for (auto &drawable : drawables){
        drawable.second->translation(direction, distance);
    }
    calc_centroid();
}

void ObjModel::translation(Vec4f origin, Vec4f destiny){
    for (auto &drawable : drawables){
        drawable.second->translation(origin, destiny);
    }
    calc_centroid();
}

void ObjModel::translation(Vec4f destiny){
    for (auto &drawable : drawables){
        drawable.second->translation(destiny);
    }
    calc_centroid();
}

void ObjModel::rotation_centroid(Vec4f axis, float angle){
    for (auto &drawable : drawables){
        drawable.second->rotation_point(axis, angle, centroid);
    }
    calc_centroid();
}

void ObjModel::rotation_point(Vec4f axis, float angle, Vec4f point){
    for (auto &drawable : drawables){
        drawable.second->rotation_point(axis, angle, point);
    }
    calc_centroid();
}

void ObjModel::draw(GLuint &color_location){
    for (auto &drawable : drawables){
        drawable.second->draw(color_location, GL_TRIANGLES);
    }
}

void ObjModel::calc_centroid(){
    centroid = Vec4f::zeros();
    for (auto &drawable : drawables){
        centroid = centroid + drawable.second->get_centroid();
    }
    centroid = centroid / drawables.size();
    centroid(3) = 0.0f;
}

#endif