#include "Aether/Core/Core.h"
#include "Test.h"
#include <string>
#include "Aether/Asset/ModelAsset.h"
namespace Test
{
class ShowModelTest:public Test
{
public:
	ShowModelTest();
	ShowModelTest(const std::string& modelPath);
	ShowModelTest(Aether::Ref<Aether::ModelAsset> modelAsset);
	~ShowModelTest();
	void OnRender() override;
	void OnImGuiRender() override;
	void OnEvent(Aether::Event& event) override;
	void OnUpdate(float sec) override;
private:
	Aether::Ref<Aether::ModelAsset> m_ModelAsset;
};
}