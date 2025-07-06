#pragma once
namespace Aether::UI
{
    // 拥有这个组件的节点，当processed为true时，
    // 会把这个节点以及这个节点的所有子节点的可渲染组件(text quad)中的visible设置为这个组件的visible值
    // 然后会把这个组件的processed设置为false
    // 用于发起一次设置可见性请求
    // 注意: 这个操作在ecs中性能开销比较大，只适合用于响应用户操作
    //       目前被用于弹出式ui的实现
    struct VisibilityRequestComponent
    {
        bool visible=true;
        bool processed = true;
    };
}