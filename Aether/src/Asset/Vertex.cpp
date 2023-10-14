#include "Vertex.h"
AETHER_NAMESPACE_BEGIN
VertexBufferLayout& Vertex::GetVertexBufferLayout()
{
    static VertexBufferLayout vbl;
    vbl.Push<float>(3);
    vbl.Push<float>(3);
    vbl.Push<float>(2);
    return vbl;
}
AETHER_NAMESPACE_END