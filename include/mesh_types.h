#pragma once

#include <GL/glew.h>

#include <vector>

struct Vertex { GLfloat x, y, z, nx, ny, nz, u, v; };
struct Mesh { std::vector<Vertex> vertices; std::vector<GLuint> indices; };
