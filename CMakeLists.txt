cmake_minimum_required(VERSION 2.8)
project(FakeGLRenderWindowRelease)

set(CMAKE_CXX_STANDARD 11)

# 添加头文件
set(GLEW_H /usr/local/Cellar/glew/2.2.0_1/include/GL)
set(GLFW_H /usr/local/Cellar/glfw/3.3.4/include/GLFW)
set(GLAD_H /usr/local/include/)
set(GLAD_H /usr/local/include/glad)
set(KH_H /usr/local/include/KHR)
#包含上述头文件所在目录
include_directories(${GLEW_H} ${GLFW_H} ${GLAD_H} ${KH_H})

# 添加目标链接
set(GLEW_LINK /usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.0.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3.4/lib/libglfw.3.3.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK})


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/usr/local/Cellar/qt@5/5.15.2")



add_executable(FakeGLRenderWindowRelease
        ArcBall.cpp
        ArcBall.h
        ArcBallWidget.cpp
        ArcBallWidget.h
        Cartesian3.cpp
        Cartesian3.h
        FakeGL.cpp
        FakeGL.h
        FakeGLRenderWidget.cpp
        FakeGLRenderWidget.h
        Homogeneous4.cpp
        Homogeneous4.h
        main.cpp
        Matrix4.cpp
        Matrix4.h
        Quaternion.cpp
        Quaternion.h
        RenderController.cpp
        RenderController.h
        RenderParameters.h
        RenderWidget.cpp
        RenderWidget.h
        RenderWindow.cpp
        RenderWindow.h
        RGBAImage.cpp
        RGBAImage.h
        RGBAValue.cpp
        RGBAValue.h
        TexturedObject.cpp
        TexturedObject.h src/glad.c)


find_package(Qt5 COMPONENTS
        Core
        REQUIRED)

find_package(Qt5Widgets REQUIRED)

# mac下这步很重要
if (APPLE)
    target_link_libraries(FakeGLRenderWindowRelease "-framework OpenGL")
    target_link_libraries(FakeGLRenderWindowRelease "-framework GLUT")
endif()

target_link_libraries(FakeGLRenderWindowRelease
        Qt5::Core
        )
target_link_libraries(FakeGLRenderWindowRelease
        Qt5::Widgets
        )

target_link_libraries(FakeGLRenderWindowRelease
        Qt5::Gui
        )