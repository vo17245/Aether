#include "Component.h"
#include "../Core/Config.h"
#include <filesystem>
#include "../Render/Transform.h"


void Aether::TransformComponent::CalculateMatrix()
{
	matrix = Transform::Translation(position)*Transform::Rotation(rotation)*Transform::Scale(scaling);
}
