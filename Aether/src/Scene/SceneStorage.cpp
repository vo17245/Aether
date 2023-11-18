#include "SceneStorage.h"

Aether::TagComponentStorage::TagComponentStorage(const TagComponent& tc)
	:buffer(tc.tag.size())
{
	header.bufferLength = tc.tag.size();
	memcpy(buffer.data(), tc.tag.data(), tc.tag.size());
}

Aether::VisualComponentStorage::VisualComponentStorage(const VisualComponent& vc)
	:buffer(vc.model->GetPath().string().size())
{
	header.bufferLength = buffer.size();
	memcpy(buffer.data(), vc.model->GetPath().string().data(), buffer.size());
}

Aether::PointLightComponentStorage::PointLightComponentStorage(const PointLightComponent& plc)
	:color(plc.light.GetColor()),position(plc.light.GetPosition())
{
}
