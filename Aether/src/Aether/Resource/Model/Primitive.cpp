#include "Primitive.h"

namespace Aether
{
    void Primitive::Bind()
    {

    }
    void Primitive::Unbind()
    {
        ibo.reset();
        vbo.reset();
        vao.reset();
    }
}