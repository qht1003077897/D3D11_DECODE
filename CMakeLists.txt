cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("D3D11_DECODE")

add_executable (D3D11_DECODE WIN32 "D3D1_DECODE.cpp" "D3D1_DECODE.h" "KMTInterceptor.h" "KMTInterceptor.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET D3D11_DECODE PROPERTY CXX_STANDARD 20)
endif()

# 添加包含目录
target_include_directories(D3D11_DECODE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include D:/workspace/msvc/include)

# 链接 Windows D3D10 运行时库
target_link_libraries(D3D11_DECODE PRIVATE D:/workspace/msvc/lib/x64/libavutild.lib D:/workspace/msvc/lib/x64/libavformatd.lib D:/workspace/msvc/lib/x64/libswresampled.lib  D:/workspace/msvc/lib/x64/libavcodecd.lib  d3d10 d3dcompiler dxgi ${CMAKE_CURRENT_SOURCE_DIR}/lib/detours.lib gdi32.lib user32.lib)

# 设置运行时输出目录
set_target_properties(D3D11_DECODE PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# TODO: 如有需要，请添加测试并安装目标。
