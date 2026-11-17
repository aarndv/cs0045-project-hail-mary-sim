#include "scene.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>

namespace scene {

GLuint loadTexture(const std::filesystem::path& path) {
    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 3);
    if (!pixels) throw std::runtime_error("Cannot load Adrian texture: " + path.string());
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    return texture;
}

void initializeSceneTextures() {
    stbi_set_flip_vertically_on_load(1);
    adrianTexture = loadTexture(textureFileDirectory / "adrian_texture_map.jpg");
    redAdrianTexture = loadTexture(textureFileDirectory / "red_adrian_texture.png");
}

} // namespace scene
