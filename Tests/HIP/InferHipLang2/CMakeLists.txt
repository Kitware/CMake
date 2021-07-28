cmake_minimum_required(VERSION 3.18)
project(InferHipLang C CXX HIP)

#Goal for this example:
#make sure that we understand that HIP is the correct link language
add_library(InterfaceWithHIP OBJECT)
target_sources(InterfaceWithHIP PRIVATE interface.hip main.cxx)
target_compile_features(InterfaceWithHIP INTERFACE hip_std_14)
target_compile_features(InterfaceWithHIP INTERFACE cxx_std_11)

add_executable(HIPInferHipLang2 )
target_link_libraries(HIPInferHipLang2 PRIVATE InterfaceWithHIP)
