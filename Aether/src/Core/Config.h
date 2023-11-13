#pragma once
namespace Aether
{
	class Config
	{
	public:
		
		const char* resource_path;
		const char* log_name;
		const char* app_name;
	};
	extern Config& GetConfig();//define in Aether Application
}
