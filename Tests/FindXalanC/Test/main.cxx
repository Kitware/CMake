#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

int main()
{
  xercesc::XMLPlatformUtils::Initialize();
  xalanc::XalanTransformer::initialize();
  xalanc::XalanTransformer::terminate();
  xercesc::XMLPlatformUtils::Terminate();
}
