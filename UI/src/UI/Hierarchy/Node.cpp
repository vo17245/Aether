#include "Node.h"
#include "Hierarchy.h"
namespace Aether::UI
{
    void Node::Remove()
    {
        hierarchy->DestroyNode(this);
    }
    
}