#pragma once
#include "../Core/Core.h"
#include <string>
#include "../Asset/Image.h"
AETHER_NAMESPACE_BEGIN
class Texture2D
{
public:
	Texture2D(const std::string& path,uint32_t slot);
	Texture2D(const Image& image, uint32_t slot);
	~Texture2D();
	void Bind();
	void Unbind();
	void SetSlot(uint32_t slot);
	inline const uint32_t GetSlot()const { return m_Slot; }
	inline const std::string& GetTypeName()const { return m_TypeName; }
	inline void SetTypeName(const std::string& typeName) { m_TypeName = typeName; }
	inline uint32_t GetRendererId() { return m_RendererId; }
private:
	uint32_t m_RendererId;
	uint32_t m_Slot;
	std::string m_TypeName;
};
AETHER_NAMESPACE_END