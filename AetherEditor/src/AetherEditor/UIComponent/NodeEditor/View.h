#pragma once
#include <ImGui/ImGui.h>
namespace AetherEditor::ImGuiComponent
{
enum class ViewType
{
    FloatInput,
    FloatSlider,
    Float2Input,
    Float2Slider,
    Float3Input,
    Float3Slider,
    Float4Input,
    Float4Slider,
    Color3,
    Color4,
    TextInput,
    Texture2D,
};
class View
{
public:
    virtual ~View() = default;
    View(ViewType type) : m_Type(type)
    {
    }
    virtual void OnDraw()=0;
    virtual void OnUpdate(float deltaSec)=0;
    ViewType GetType() const
    {
        return m_Type;
    }
private:
    ViewType m_Type;
};
class FloatInputView:public View
{
public:
    FloatInputView(float* value):View(ViewType::FloatInput),m_Value(value)
    {
    }
    void OnDraw() override
    {
        ImGui::InputFloat("##float",m_Value);
        ImGuiEx::BlockPushLast();
    }
    void OnUpdate(float deltaSec) override
    {
    }
private:
    float* m_Value;
};
class FloatSliderView:public View
{
public:
    FloatSliderView(float* value,float min,float max):View(ViewType::FloatSlider),m_Value(value),m_Min(min),m_Max(max)
    {
    }
    void OnDraw() override
    {
        ImGui::SliderFloat("##float",m_Value,m_Min,m_Max);
        ImGuiEx::BlockPushLast();
    }
    void OnUpdate(float deltaSec) override
    {
    }
private:
    float* m_Value;
    float m_Min;
    float m_Max;
};
class Color3View:public View
{
public:
    /**
     * @param value Pointer to an array of 3 floats representing the color (RGB).
    */
    Color3View(float* value):View(ViewType::Color3),m_Value(value)
    {
    }
    void OnDraw() override
    {
        ImGui::ColorEdit3("##color3",m_Value,ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_NoLabel);
        ImGuiEx::BlockPushLast();
    }
    void OnUpdate(float deltaSec) override
    {
    }
private:
    float* m_Value;
};
class TextInputView:public View
{
public:
    TextInputView(char* buffer,size_t bufferSize):View(ViewType::TextInput),m_Buffer(buffer),m_BufferSize(bufferSize)
    {
    }
    void SetWidth(float width)
    {
        m_Width=width;
    }
    void OnDraw() override
    {
        // set input text width
        ImGui::SetNextItemWidth(m_Width);
        ImGui::InputText("##text",m_Buffer,m_BufferSize);
        ImGuiEx::BlockPushLast();
    }
    void OnUpdate(float deltaSec) override
    {
    }
private:
    char* m_Buffer;
    size_t m_BufferSize;
    float m_Width=200.0f;
};
} // namespace Aether::ImGuiComponent