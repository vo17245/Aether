#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"



#include "Aether/Core/Config.h"
namespace Aether
{
    static Config CreateConfig()
    {
        Config config;
        config.resource_path = "../Resource";
        config.log_name = "Sandbox";
        config.app_name = "Sandbox";
        config.shader_dir="../../Aether/shader";
        return config;
    }
    Config& GetConfig()
    {
        static Config config=CreateConfig();
        
        return config;
    }
}