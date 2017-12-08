#include <limits>
#include "common.h"
#include "trackball.h"
#include "callback.h"

void SetupTF(const void *colors, const void *opacities,
             int colorW, int colorH, int opacityW, int opacityH) {
    //! setup trasnfer function
    OSPData colorsData = ospNewData(colorW * colorH, OSP_FLOAT3, colors);
    ospCommit(colorsData);
    OSPData opacitiesData = ospNewData(opacityW * opacityH, OSP_FLOAT, opacities);
    ospCommit(opacitiesData);
    ospSetData(transferFcn, "colors", colorsData);
    ospSetData(transferFcn, "opacities", opacitiesData);
    ospSetVec2f(transferFcn, "valueRange",
                osp::vec2f{static_cast<float>(0),
                           static_cast<float>(255)});
    ospSet1i(transferFcn, "colorWidth", colorW);
    ospSet1i(transferFcn, "colorHeight", colorH);
    ospSet1i(transferFcn, "opacityWidth", opacityW);
    ospSet1i(transferFcn, "opacityHeight", opacityH);
    ospCommit(transferFcn);
    ospRelease(colorsData);
    ospRelease(opacitiesData);
}

void volume(int argc, const char **argv) {
    //! transfer function
    const std::vector<ospcommon::vec3f> colors = {
            ospcommon::vec3f(0, 0, 0.563),
            ospcommon::vec3f(0, 0, 1),
            ospcommon::vec3f(0, 1, 1),
            ospcommon::vec3f(0.5, 1, 0.5),
            ospcommon::vec3f(1, 1, 0),
            ospcommon::vec3f(1, 0, 0),
            ospcommon::vec3f(0.5, 0, 0),
    };
    const std::vector<float> opacities = {0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f};
    SetupTF(colors.data(), opacities.data(), colors.size(), 1, opacities.size(), 1);
    //! create volume
    const ospcommon::vec3i dims(20, 10, 10);
    const float coef = 1.f / (float) (dims.x - 1) * (float) M_PI / 2.0f;
    ospcommon::vec2f vRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    ospcommon::vec2f gRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    auto volumeData = new unsigned char[dims.x * dims.y * dims.z];
    auto gradientData = new float[dims.x * dims.y * dims.z];
    cleanlist.push_back([=]() { delete[] volumeData; });
    for (int x = 0; x < dims.x; ++x) {
        for (int y = 0; y < dims.y; ++y) {
            for (int z = 0; z < dims.z; ++z) {
                auto i = z * dims.y * dims.x + y * dims.x + x;
                auto v = sin(coef * x) * 255.0f;
                auto g = coef * cos(coef * x) * 255.0f;
                vRange.x = std::min(vRange.x, v);
                vRange.y = std::max(vRange.y, v);
                gRange.x = std::min(gRange.x, g);
                gRange.y = std::max(gRange.y, g);
                volumeData[i] = static_cast<unsigned char>(v);
                gradientData[i] = g;
            }
        }
    }
    //! calculate hist
    const int xDim = 256, yDim = 256;
    std::vector<float> hist(xDim * yDim, 0);
    for (int x = 0; x < dims.x; ++x) {
        for (int y = 0; y < dims.y; ++y) {
            for (int z = 0; z < dims.z; ++z) {
                const int i = z * dims.y * dims.x + y * dims.x + x;
                const float v = volumeData[i];
                const float g = gradientData[i];
                const int ix = round((xDim - 1) * (v - vRange.x) / (vRange.y - vRange.x));
                const int iy = round((yDim - 1) * (g - gRange.x) / (gRange.y - gRange.x));
                hist[iy * 256 + ix] += 1.f;
            }
        }
    }

    //! create ospray volume
    auto t1 = std::chrono::system_clock::now();
    {
        OSPVolume volume = ospNewVolume("shared_structured_volume");
        OSPData voxelData = ospNewData(dims.x * dims.y * dims.z,
                                       OSP_UCHAR, volumeData,
                                       OSP_DATA_SHARED_BUFFER);
        cleanlist.push_back([=]() {
            ospRelease(volume);
            ospRelease(voxelData);
        });
        ospSetString(volume, "voxelType", "uchar");
        ospSetVec3i(volume, "dimensions", (osp::vec3i &) dims);
        ospSetVec3f(volume, "gridOrigin", osp::vec3f{-dims.x / 2.0f, -dims.y / 2.0f, -dims.z / 2.0f});
        ospSetVec3f(volume, "gridSpacing", osp::vec3f{1.0f, 1.0f, 1.0f});
        ospSet1f(volume, "samplingRate", 9.0f);
        ospSet1i(volume, "gradientShadingEnabled", 0);
        ospSet1i(volume, "useGridAccelerator", 0);
        ospSet1i(volume, "adaptiveSampling", 0);
        ospSet1i(volume, "preIntegration", 0);
        ospSet1i(volume, "singleShade", 0);
        ospSetData(volume, "voxelData", voxelData);
        ospSetObject(volume, "transferFunction", transferFcn);
        ospCommit(volume);
        ospAddVolume(world, volume);
    }
    auto t2 = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = t2 - t1;
    std::cout << "finish commits " << dur.count() << " seconds" << std::endl;
}

int main(int argc, const char **argv) {
    //---------------------------------------------------------------------------------------//
    // OpenGL Setup
    //---------------------------------------------------------------------------------------//
    // Create Context
    GLFWwindow *window = InitWindow();
    check_error_gl("Initialized OpenGL");

    //---------------------------------------------------------------------------------------//
    // OSPRay Setup
    //---------------------------------------------------------------------------------------//
    ospInit(&argc, argv);
    ospLoadModule("tfn");

    //! Init camera and framebuffer
    camera.Init();
    framebuffer.Init(camera.CameraWidth(), camera.CameraHeight());

    //! create world and renderer
    world = ospNewModel();
    renderer = ospNewRenderer("scivis");

    //! setup volume/geometry
    transferFcn = ospNewTransferFunction("piecewise_linear_2d");
    volume(argc, argv);
    ospCommit(world);

    //! lighting
    OSPLight ambient_light = ospNewLight(renderer, "AmbientLight");
    ospSet1f(ambient_light, "intensity", 0.0f);
    ospCommit(ambient_light);
    OSPLight directional_light = ospNewLight(renderer, "DirectionalLight");
    ospSet1f(directional_light, "intensity", 2.0f);
    ospSetVec3f(directional_light, "direction", osp::vec3f{20.0f, 20.0f, 20.0f});
    ospCommit(directional_light);
    std::vector<OSPLight> light_list{ambient_light, directional_light};
    OSPData lights = ospNewData(light_list.size(), OSP_OBJECT, light_list.data());
    ospCommit(lights);

    //! renderer
    ospSetVec3f(renderer, "bgColor", osp::vec3f{0.5f, 0.5f, 0.5f});
    ospSetData(renderer, "lights", lights);
    ospSetObject(renderer, "model", world);
    ospSetObject(renderer, "camera", camera.OSPRayPtr());
    ospSet1i(renderer, "shadowEnabled", 0);
    ospSet1i(renderer, "oneSidedLighting", 0);
    ospCommit(renderer);
    //---------------------------------------------------------------------------------------//
    // Render
    //---------------------------------------------------------------------------------------//
    while (!glfwWindowShouldClose(window)) {
        // clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render
        render();
        // swap frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ShutdownWindow(window);
    glfwTerminate();

    // exit
    Clean();
    return EXIT_SUCCESS;
}

