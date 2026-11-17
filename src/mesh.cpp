#include "scene.h"
#include "mesh_data.h"
#include "adrian_data.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace scene {

const GLfloat adrianModelZOffset = adrian_data::modelZOffset;
constexpr double adrianScale = 1.2;
constexpr double adrianDropFraction = 0.10;

Vec3 adrianSceneCenter() {
    const Vec3 original{adrian_data::centerX, adrian_data::centerY, adrian_data::centerZ};
    const Vec3 fromShip{original.x, original.y, original.z - adrian_data::modelZOffset};
    const double distance = std::sqrt(fromShip.x * fromShip.x + fromShip.y * fromShip.y +
                                      fromShip.z * fromShip.z);
    const double shift = (adrianScale - 1.0) * adrian_data::radius / distance;
    const Vec3 placed{original.x + fromShip.x * shift,
                      original.y + fromShip.y * shift,
                      original.z + fromShip.z * shift};
    const double centerDistance = distance + (adrianScale - 1.0) * adrian_data::radius;
    const double loweredY = placed.y - adrianDropFraction *
        (2.0 * adrian_data::radius * adrianScale);
    const double zFromShip = placed.z - adrian_data::modelZOffset;
    const double xFromShip = std::copysign(std::sqrt(std::max(0.0,
        centerDistance * centerDistance - loweredY * loweredY - zFromShip * zFromShip)), placed.x);
    return {xFromShip, loweredY, placed.z};
}

void uploadMesh(const Mesh& mesh, RenderMesh& target) {
    const std::size_t vertexBytes = mesh.vertices.size() * sizeof(Vertex);
    const std::size_t indexBytes = mesh.indices.size() * sizeof(GLuint);
    if (vertexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) ||
        indexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) ||
        mesh.indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Mesh is too large for GPU buffers");
    }
    glGenBuffers(1, &target.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, target.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexBytes), mesh.vertices.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &target.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexBytes), mesh.indices.data(), GL_STATIC_DRAW);
    target.indexCount = static_cast<GLsizei>(mesh.indices.size());
}

void initializeSceneMeshes() {
    Mesh model = loadEmbeddedMesh();
    Mesh adrian = loadAdrianMesh();
    const Vec3 placed = adrianSceneCenter();
    for (Vertex& vertex : adrian.vertices) {
        vertex.x = static_cast<GLfloat>(placed.x +
            (vertex.x - adrian_data::centerX) * adrianScale);
        vertex.y = static_cast<GLfloat>(placed.y +
            (vertex.y - adrian_data::centerY) * adrianScale);
        vertex.z = static_cast<GLfloat>(placed.z +
            (vertex.z - adrian_data::centerZ) * adrianScale);
    }
    std::cout << "Embedded scene: " << model.indices.size() / 3 << " model triangles and "
              << adrian.indices.size() / 3 << " Adrian triangles\n";

    uploadMesh(model, modelMesh);
    uploadMesh(adrian, adrianMesh);
}

} // namespace scene
