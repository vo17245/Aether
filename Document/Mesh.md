## attribute location

attribute使用固定的location，如果一个mesh只有texcoord，texcoord的location也是2

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