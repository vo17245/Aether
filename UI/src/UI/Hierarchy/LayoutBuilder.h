#pragma once
#include "Core/Math/Def.h"
#include <list>
#include <new>
namespace Aether::UI
{
struct LayoutBuilder
{
    /**
     * @param size ,  box(id=0) size
     */
    void Begin(Vec2f size)
    {
        boxes.clear();
        boxes.push_back({size, Vec2f(0, 0)});
        boxes[0].availableQuads.push_back({size, Vec2f(0, 0)});
    }
    /**
     * @return position
     */
    Vec2f PushBox(Vec2f size, size_t containerId)
    {
        auto& container = boxes[containerId];
        auto& availableBoxes = container.availableQuads;
        Vec2f pos=container.quad.position;
        for (auto iter = availableBoxes.begin(); iter != availableBoxes.end(); iter++)
        {
            auto& availableBox = *iter;
            if (availableBox.size.x() < size.x() && availableBox.size.y() < size.y())
            {
                continue;
            }
            if (availableBox.size.x() == size.x() && availableBox.size.y() == size.y())
            {
                pos = availableBox.position;
                availableBoxes.erase(iter);
                break;
            }
            if (availableBox.size.x() == size.x())
            {
                pos = availableBox.position;
                auto newQuad = Quad{Vec2f(availableBox.size.x(), availableBox.size.y() - size.y()),
                                  Vec2f(availableBox.position.x(), availableBox.position.y() + size.y())};
                *iter= newQuad;
                break;
            }
            if(availableBox.size.y()==size.y())
            {
                pos = availableBox.position;
                auto newBox = Quad{Vec2f(availableBox.size.x() - size.x(), availableBox.size.y()),
                                  Vec2f(availableBox.position.x() + size.x(), availableBox.position.y())};
                *iter= newBox;
                break;
            }
            pos=availableBox.position;
            auto newQuad1=Quad{Vec2f(availableBox.size.x()-size.x(),size.y()),
                             Vec2f(availableBox.position.x()+size.x(),availableBox.position.y())};
            auto newQuad2=Quad{Vec2f(size.x(),availableBox.size.y()-size.y()),
                                Vec2f(availableBox.position.x(),availableBox.position.y()+size.y())};
            *iter= newQuad1;
            availableBoxes.push_back(newQuad2);
            break;
        }
        boxes.emplace_back(Quad{size, pos},std::list<Quad>{Quad{size, pos}});
        return pos;

    }
    //@brief 创建一个脱离流的盒子，这个盒子自己不会参与布局，他的子节点会在这个盒子中布局
    void PlaceBox(const Vec2f& size,const Vec2f& position)
    {
        boxes.emplace_back(Quad{size, position}, std::list<Quad>{Quad{size, position}});
    }
    void End()
    { /*do nothing now*/
    }
    struct Quad
    {
        Vec2f size;
        Vec2f position;
    };
    struct Box
    {
        Quad quad;
        std::list<Quad> availableQuads;
    };
    std::vector<Box> boxes;
};
} // namespace Aether::UI