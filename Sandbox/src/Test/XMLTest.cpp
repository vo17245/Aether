#include "XMLTest.h"

#include "Aether/IO/XML/tinyxml2.h"
#include "TestRegister.h"
#include <iostream>
#include <memory>
#include <optional>
namespace Aether {
namespace Test {
REGISTER_TEST(XMLTest);

XMLTest::XMLTest()
{
    std::string path = "../../AetherEditor/page/example.xml";
    auto xml = XML::LoadFile(path);
    for (auto element : xml->EachElement())
    {
        std::cout << element.GetName() << std::endl;
        std::cout << "Attributes:" << std::endl;
        for (auto attribute : element.EachAttribute())
        {
            std::cout << attribute.GetName() << ":" << attribute.GetValue() << std::endl;
        }
    }
}
XMLTest::~XMLTest()
{
}

}
} // namespace Aether::Test