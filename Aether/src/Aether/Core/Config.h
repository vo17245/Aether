#pragma once
namespace Aether
{
	class Config
	{
	public:
		
		const char* resource_path;
		const char* log_name;
		const char* app_name;
		const char* shader_dir;
		Config() = default;
		Config(const Config&) = default;
		Config(Config&&) = default;
	};
	extern Config& GetConfig();//define in Aether App
}
