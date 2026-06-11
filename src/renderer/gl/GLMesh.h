// OpenGL 메시 버퍼와 드로우 래퍼를 선언합니다.
#pragma once
#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include <glad/glad.h>
#endif
#include "resource/ObjLoader.h"

class GLMesh {
public:
    GLMesh() = default;
    ~GLMesh();

    GLMesh(const GLMesh&)            = delete;
    GLMesh& operator=(const GLMesh&) = delete;

    GLMesh(GLMesh&& o) noexcept
        : vao(o.vao), vbo(o.vbo), ebo(o.ebo), indexCount(o.indexCount)
    { o.vao = o.vbo = o.ebo = 0; o.indexCount = 0; }

    GLMesh& operator=(GLMesh&& o) noexcept {
        if (this != &o) {
            if (vao) glDeleteVertexArrays(1, &vao);
            if (vbo) glDeleteBuffers(1, &vbo);
            if (ebo) glDeleteBuffers(1, &ebo);
            vao = o.vao; vbo = o.vbo; ebo = o.ebo; indexCount = o.indexCount;
            o.vao = o.vbo = o.ebo = 0; o.indexCount = 0;
        }
        return *this;
    }

    void Upload(const Mesh& mesh);
    void Draw() const;

    bool IsValid() const { return vao != 0 && indexCount > 0; }

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
};
