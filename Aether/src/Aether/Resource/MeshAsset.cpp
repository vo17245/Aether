#include "MeshAsset.h"
#include <limits>
#undef max
#undef min
namespace Aether
{

static void ReplaceMinOrMax(float& min, float& max, float val)
{
	if (val < min)
	{
		min = val;
		return;
	}
	if (max < val)
	{
		max = val;
		return;
	}
}
void MeshAsset::CalculateBoundingBox()
{
	Eigen::Vector3f max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	Eigen::Vector3f min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	for (auto& vertex : Vertices)
	{
		ReplaceMinOrMax(max.x(), min.x(), vertex.Position.x());
		ReplaceMinOrMax(max.y(), min.y(), vertex.Position.y());
		ReplaceMinOrMax(max.z(), min.z(), vertex.Position.z());
	}
	PositionMax = max;
	PositionMin = min;
}
}//namespace Aether
