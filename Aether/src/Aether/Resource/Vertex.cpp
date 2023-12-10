#include "Vertex.h"
namespace Aether{
VertexBufferLayout Vertex::CreateVertexBufferLayout()
{
    VertexBufferLayout vbl;
    vbl.Push<float>(3);
    vbl.Push<float>(3);
    vbl.Push<float>(2);
    return vbl;
}
}//namespace Aether