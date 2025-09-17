## attribute location

attribute按顺序分配location
比如,如果一个mesh只有POSITION和TEXCOORD，POSITION的location是0,texcoord的location是1


```cpp
std::unordered_map<Mesh::Attribute, uint32_t> attributeLocation = {
            {Mesh::Attribute::POSITION, 0},
            {Mesh::Attribute::NORMAL, 1},
            {Mesh::Attribute::TEXCOORD, 2},
            {Mesh::Attribute::TANGENT, 3},
            {Mesh::Attribute::COLOR, 4},
            {Mesh::Attribute::BITANGENT, 5},
        };
```