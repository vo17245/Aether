#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "Aether/Core/Config.h"
namespace Aether
{
    Config& GetConfig()
    {
        static Config config;
        config.resource_path="../../Resource";
		config.log_name="AetherEditor";
		config.app_name="AetherEditor";
        return config;
    }
}