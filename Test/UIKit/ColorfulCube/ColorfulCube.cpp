#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "Renderer/Camera.h"
#include "Renderer/GpuBuffer.h"
#include "Renderer/GraphUtils/ParamHelper.h"
#include "Renderer/GraphUtils/PSO.h"
#include "Renderer/GraphUtils/Shader.h"
#include "Renderer/Renderer.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/CheckBox.h"
#include "UIKit/GridLayout.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/RawTextBox.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/ScenePanel.h"

using namespace d14engine;
using namespace d14engine::renderer;
using namespace d14engine::uikit;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

D14_SET_APP_ENTRY(mainColorfulCube)
{
    Application::CreateInfo info = {};
    if (argc >= 2 && lstrcmp(argv[1], L"HighDPI") == 0)
    {
        info.dpi = 192.0f;
    }
    else info.dpi = 96.0f;
    info.win32WindowRect = { 0, 0, 1000, 600 };

    BitmapObject::g_interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

    return Application(argc, argv, info).run([&](Application* app)
    {
        app->increaseAnimationCount(); // enable renderer updating

        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - ColorfulCube @ UIKit");
        {
            ui_mainWindow->moveTopmost();
            ui_mainWindow->isMaximizeEnabled = false;

            ui_mainWindow->caption()->transform(300.0f, 0.0f, 376.0f, 32.0f);
        }
        auto ui_darkModeLabel = makeRootUIObject<Label>(L"Dark Mode");
        auto ui_darkModeSwitch = makeRootUIObject<OnOffSwitch>();
        {
            ui_darkModeLabel->moveTopmost();
            ui_darkModeLabel->transform(10.0f, 0.0f, 120.0f, 32.0f);

            ui_darkModeSwitch->moveTopmost();
            ui_darkModeSwitch->move(130.0f, 4.0f);

            if (app->systemThemeStyle().mode == Application::ThemeStyle::Mode::Light)
            {
                ui_darkModeSwitch->setOnOffState(OnOffSwitch::OFF);
            }
            else ui_darkModeSwitch->setOnOffState(OnOffSwitch::ON);

            app->customThemeStyle = app->systemThemeStyle();
            app->f_onSystemThemeStyleChange = [app]
            {
                app->customThemeStyle.value().color = app->systemThemeStyle().color;
                app->changeTheme(app->currThemeName());
            };
            ui_darkModeSwitch->f_onStateChange = [app]
            (OnOffSwitch::StatefulObject* obj, OnOffSwitch::StatefulObject::Event& e)
            {
                auto& customThemeStyle = app->customThemeStyle.value();
                if (e.on())
                {
                    customThemeStyle.mode = Application::ThemeStyle::Mode::Dark;
                    app->changeTheme(L"Dark");
                }
                else if (e.off())
                {
                    customThemeStyle.mode = Application::ThemeStyle::Mode::Light;
                    app->changeTheme(L"Light");
                }
            };
        }
        auto ui_screenshot = makeRootUIObject<OutlinedButton>(L"Screenshot");
        {
            ui_screenshot->moveTopmost();
            ui_screenshot->transform(200.0f, 4.0f, 100.0f, 24.0f);
            ui_screenshot->content()->label()->setTextFormat(D14_FONT(L"Default/Normal/12"));

            ui_screenshot->f_onMouseButtonRelease = []
            (ClickablePanel* clkp, ClickablePanel::Event& e)
            {
                auto image = Application::g_app->screenshot();
                CreateDirectory(L"Screenshots", nullptr);
                bitmap_utils::saveBitmap(image.Get(), L"Screenshots/ColorfulCube.png");
            };
        }
        auto ui_clientArea = makeUIObject<Panel>();
        {
            ui_mainWindow->setCenterUIObject(ui_clientArea);
        }
        auto ui_scenePanel = makeManagedUIObject<ScenePanel>(ui_clientArea);
        auto wk_scenePanel = (WeakPtr<ScenePanel>)ui_scenePanel;
        {
            ui_scenePanel->setUIObjectPriority(0);
            ui_scenePanel->transform(0.0f, 0.0f, 564.0f, 564.0f);

            ui_scenePanel->f_onChangeTheme = [](Panel* p, WstrParam themeName)
            {
                auto sp = (ScenePanel*)p;
                if (themeName == L"Light") sp->setClearColor(Colors::White);
                else if (themeName == L"Dark") sp->setClearColor(Colors::Black);
            };
            ui_scenePanel->f_onChangeTheme(ui_scenePanel.get(), app->currThemeName());
        }
        // Initialize necessary graphics structures of the colorful cube.

        auto rndr = app->dxRenderer();
        auto device = rndr->d3d12Device();
        auto cmdList = rndr->cmdList();

        using D3D12Target = Renderer::CommandLayer::D3D12Target;
        auto& target = std::get<D3D12Target>(ui_scenePanel->cmdLayer()->drawTarget);
        auto& objset = target.at(ui_scenePanel->primaryLayer());

        auto prepare = std::make_shared<ScenePanel::Object>();
        {
            // Create root signature.
            CD3DX12_ROOT_PARAMETER1 rootParams[2] = {};

            // camera data
            rootParams[0].InitAsConstantBufferView(0);
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

            // world matrix
            rootParams[1].InitAsConstants(16, 1);
            rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

            D3D12_ROOT_SIGNATURE_FLAGS rootFlags =
            (
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            );
            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.Init_1_1(NUM_ARR_ARGS(rootParams), 0, nullptr, rootFlags);

            auto& maxVersion = rndr->d3d12DeviceInfo().feature.rootSignature.HighestVersion;

            ComPtr<ID3DBlob> rootBlob;
            THROW_IF_ERROR(D3DX12SerializeVersionedRootSignature(&rootDesc, maxVersion, &rootBlob, &error));

            ComPtr<ID3D12RootSignature> rootSignature;
            THROW_IF_FAILED(rndr->d3d12Device()->CreateRootSignature(
                0, BLB_PSZ_ARGS(rootBlob), IID_PPV_ARGS(&rootSignature)));

            // Create pipeline state.
#ifdef _DEBUG
            Wstring shaderPath = L"Test/UIKit/ColorfulCube/ColorfulCube.hlsl";
#else
            Wstring shaderPath = L"ColorfulCube.hlsl";
#endif
            auto vs = graph_utils::shader::compile(shaderPath.c_str(), L"VS", L"vs_6_0");
            auto ps = graph_utils::shader::compile(shaderPath.c_str(), L"PS", L"ps_6_0");

            D3D12_INPUT_ELEMENT_DESC inputElemDescs[] =
            {
                {
                    /* SemanticName         */ "POSITION",
                    /* SemanticIndex        */ 0,
                    /* Format               */ DXGI_FORMAT_R32G32B32_FLOAT,
                    /* InputSlot            */ 0,
                    /* AlignedByteOffset    */ 0,
                    /* InputSlotClass       */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    /* InstanceDataStepRate */ 0
                },
                {
                    /* SemanticName         */ "COLOR",
                    /* SemanticIndex        */ 0,
                    /* Format               */ DXGI_FORMAT_R32G32B32A32_FLOAT,
                    /* InputSlot            */ 0,
                    /* AlignedByteOffset    */ D3D12_APPEND_ALIGNED_ELEMENT,
                    /* InputSlotClass       */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    /* InstanceDataStepRate */ 0
                }
            };
            auto psoDesc = graph_utils::graphicsPipelineStateDescTemplate();

            psoDesc.pRootSignature = rootSignature.Get();
            psoDesc.VS = { BLB_PSZ_ARGS(vs) };
            psoDesc.PS = { BLB_PSZ_ARGS(ps) };
            psoDesc.InputLayout = { ARR_NUM_ARGS(inputElemDescs) };

            ComPtr<ID3D12PipelineState> pipelineState;
            THROW_IF_FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

            psoDesc.SampleDesc.Count = 4;
            auto level = rndr->d3d12DeviceInfo().feature.queryMsaaQualityLevel(4);
            if (level.has_value()) psoDesc.SampleDesc.Quality = level.value();

            ComPtr<ID3D12PipelineState> pipelineStateMsaa;
            THROW_IF_FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateMsaa)));

            prepare->f_onRendererDrawD3d12Object = [=](ScenePanel::Object* obj, Renderer* rndr)
            {
                rndr->cmdList()->SetGraphicsRootSignature(rootSignature.Get());

                if (!wk_scenePanel.expired() && wk_scenePanel.lock()->msaaEnabled())
                {
                    rndr->cmdList()->SetPipelineState(pipelineStateMsaa.Get());
                }
                else rndr->cmdList()->SetPipelineState(pipelineState.Get());
            };
        }
        prepare->setPriority(0);
        objset.insert(prepare);

        auto camera = std::make_shared<Camera>(device);
        {
            camera->eyePos = { -2.0f, +2.0f, -2.0f };
            camera->eyeDir = { +1.0f, -1.0f, +1.0f };
            camera->updateViewMatrix();

            camera->nearZ = 0.1f; camera->farZ = 100.0f;
            // proj-mat will be updated in onViewResize

            auto viewSize = math_utils::roundu(ui_scenePanel->size());
            camera->onViewResize(viewSize.width, viewSize.height);
        }
        camera->setPriority(1);
        objset.insert(camera);

        auto worldMatrix = std::make_shared<XMFLOAT4X4>(math_utils::identityFloat4x4());

        auto cubeobj = std::make_shared<ScenePanel::Object>();
        {
            Vertex vertices[] =
            {
                { {+0.5f, +0.5f, +0.5f}, XMFLOAT4{Colors::Magenta} }, // 0
                { {-0.5f, +0.5f, +0.5f}, XMFLOAT4{Colors::Red    } }, // 1
                { {+0.5f, -0.5f, +0.5f}, XMFLOAT4{Colors::Orange } }, // 2
                { {+0.5f, +0.5f, -0.5f}, XMFLOAT4{Colors::Yellow } }, // 3
                { {-0.5f, -0.5f, +0.5f}, XMFLOAT4{Colors::Green  } }, // 4
                { {-0.5f, +0.5f, -0.5f}, XMFLOAT4{Colors::Cyan   } }, // 5
                { {+0.5f, -0.5f, -0.5f}, XMFLOAT4{Colors::Blue   } }, // 6
                { {-0.5f, -0.5f, -0.5f}, XMFLOAT4{Colors::Purple } }  // 7
            };
            UINT32 indices[] =
            {
                // front face
                5, 3, 7,
                6, 7, 3,
                // back face
                0, 1, 2,
                4, 2, 1,
                // left face
                4, 1, 7,
                5, 7, 1,
                // right face
                0, 2, 3,
                6, 3, 2,
                // top face
                0, 3, 1,
                5, 1, 3,
                // bottom face
                4, 7, 2,
                6, 2, 7
            };
            auto vertexBuffer = std::make_shared<DefaultBuffer>(device, sizeof(vertices));
            auto indexBuffer = std::make_shared<DefaultBuffer>(device, sizeof(indices));

            rndr->beginGpuCommand();

            vertexBuffer->uploadData(cmdList, vertices, sizeof(vertices));
            indexBuffer->uploadData(cmdList, indices, sizeof(indices));

            rndr->endGpuCommand();

            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
            vertexBufferView.BufferLocation = vertexBuffer->resource()->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = sizeof(vertices);
            vertexBufferView.StrideInBytes = sizeof(Vertex);

            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
            indexBufferView.BufferLocation = indexBuffer->resource()->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = sizeof(indices);
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;

            cubeobj->f_onRendererDrawD3d12Object =
            [worldMatrix, vertexBuffer, indexBuffer, vertexBufferView, indexBufferView]
            (ScenePanel::Object* obj, Renderer* rndr)
            {
                rndr->cmdList()->SetGraphicsRoot32BitConstants(1, 16, worldMatrix.get(), 0);

                rndr->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                rndr->cmdList()->IASetVertexBuffers(0, 1, &vertexBufferView);
                rndr->cmdList()->IASetIndexBuffer(&indexBufferView);

                rndr->cmdList()->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
            };
        }
        cubeobj->setPriority(2);
        objset.insert(cubeobj);

        auto ui_sideLayout = makeManagedUIObject<GridLayout>(ui_clientArea);
        {
            ui_sideLayout->setUIObjectPriority(1);
            ui_sideLayout->transform(564.0f, 0.0f, 436.0f, 564.0f);
            ui_sideLayout->getAppearance().background.opacity = 1.0f;
            ui_sideLayout->setCellCount(8, 10);
            ui_sideLayout->setMargin(10.0f, 0.0f);
        }
        auto ui_graphTitle = makeUIObject<Label>(L"-------------- Graph --------------");
        {
            THROW_IF_FAILED(ui_graphTitle->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = false;
            geoInfo.axis.x = { 0, 8 };
            geoInfo.axis.y = { 0, 1 };
            ui_sideLayout->addElement(ui_graphTitle, geoInfo);
        }
        auto ui_graphMsaaChecker = makeUIObject<CheckBox>();
        {
            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_graphMsaaChecker, geoInfo1);

            auto level = rndr->d3d12DeviceInfo().feature.queryMsaaQualityLevel(4);

            Wstring levelStr = {};
            if (level.has_value())
            {
                levelStr = L"(max quality-level: " + std::to_wstring(level.value()) + L")";
            }
            else levelStr = L"(not supported)";

            auto ui_label = makeUIObject<Label>(L"Enable 4x MSAA " + levelStr);

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 7 };
            geoInfo2.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_graphMsaaChecker->f_onStateChange = [=]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_scenePanel.expired())
                {
                    auto sh_scenePanel = wk_scenePanel.lock();
                    if (e.checked()) sh_scenePanel->setMultiSample(4);
                    else if (e.unchecked()) sh_scenePanel->setMultiSample(0);
                }
            };
            if (!level.has_value())
            {
                ui_graphMsaaChecker->setEnabled(false);
                ui_label->setEnabled(false);
            }
        }
        // Press TAB to move focus between the group members.
        using FocusGroup = std::list<SharedPtr<Panel>>;
        auto generateTabLink = [](const std::list<SharedPtr<Panel>>& group)
        {
            for (auto itor = group.begin(); itor != group.end(); ++itor)
            {
                auto nextItor = std::next(itor);
                if (nextItor == group.end())
                {
                    nextItor = group.begin();
                }
                (*itor)->f_onKeyboard = [wk_next = (WeakPtr<Panel>)(*nextItor)](Panel* p, KeyboardEvent& e)
                {
                    if (e.vkey == VK_TAB && e.state.pressed() && !wk_next.expired())
                    {
                        auto sh_next = wk_next.lock();
                        Application::g_app->focusUIObject(sh_next);

                        auto sh_input = std::dynamic_pointer_cast<RawTextBox>(sh_next);
                        if (sh_input != nullptr)
                        {
                            sh_input->setHiliteRange({ 0, sh_input->text().size() });
                        }
                    }
                };
            };
        };
        auto ui_cameraTitle = makeUIObject<Label>(L"-------------- Camera --------------");
        {
            THROW_IF_FAILED(ui_cameraTitle->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = false;
            geoInfo.axis.x = { 0, 8 };
            geoInfo.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_cameraTitle, geoInfo);
        }
        FocusGroup cameraDataFocusGroup = {};
        auto createCameraDataEditor = [&](WstrParam name, size_t offsetY, wchar_t which, OptParam<XMFLOAT3> invalid)
        {
            auto ui_cameraData = makeUIObject<Label>(name);
            {
                THROW_IF_FAILED(ui_cameraData->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

                GridLayout::GeometryInfo geoInfo = {};
                geoInfo.isFixedSize = false;
                geoInfo.axis.x = { 0, 2 };
                geoInfo.axis.y = { offsetY, 1 };
                ui_sideLayout->addElement(ui_cameraData, geoInfo);
            }
            auto updateCameraData = [=, wk_camera = (WeakPtr<Camera>)camera]
            (RawTextBox* src, wchar_t component /* select from X, Y, Z */)
            {
                if (!wk_camera.expired())
                {
                    auto sh_camera = wk_camera.lock();
                    auto value = wcstof(src->text().c_str(), nullptr);
                    XMFLOAT3* pData = nullptr;
                    switch (towupper(which))
                    {
                    case 'P': pData = &sh_camera->eyePos; break;
                    case 'D': pData = &sh_camera->eyeDir; break;
                    default: break;
                    }
                    if (pData != nullptr)
                    {
                        float tempValue = 0.0f;
                        XMFLOAT3 tempData = *pData;
                        switch (towupper(component))
                        {
                        case 'X': tempValue = tempData.x; tempData.x = value; break;
                        case 'Y': tempValue = tempData.y; tempData.y = value; break;
                        case 'Z': tempValue = tempData.z; tempData.z = value; break;
                        default: break;
                        }
                        if (invalid.has_value())
                        {
                            auto& invalidValue = invalid.value();
                            if (tempData.x == invalidValue.x &&
                                tempData.y == invalidValue.y &&
                                tempData.z == invalidValue.z)
                            {
                                src->setText(std::to_wstring(tempValue));
                                return; // no need to do the remaining updating
                            }
                        }
                        *pData = tempData; // given value is valid, do updating

                        sh_camera->updateViewMatrix();
                        sh_camera->dirtyFrameCount = FrameResource::g_bufferCount;
                    }
                    src->setText(std::to_wstring(value));
                }
            };
            auto createCameraDataComponentEditor = [&, offsetY](size_t offsetX, wchar_t component)
            {
                auto ui_cameraData_ = makeUIObject<RawTextBox>(5.0f);
                {
                    ui_cameraData_->resize(100.0f, 40.0f);

                    XMFLOAT3* pData = nullptr;
                    switch (towupper(which))
                    {
                    case 'P': pData = &camera->eyePos; break;
                    case 'D': pData = &camera->eyeDir; break;
                    default: break;
                    }
                    if (pData != nullptr)
                    {
                        float* pValue = nullptr;
                        switch (towupper(component))
                        {
                        case 'X': pValue = &pData->x; break;
                        case 'Y': pValue = &pData->y; break;
                        case 'Z': pValue = &pData->z; break;
                        default: break;
                        }
                        if (pValue != nullptr)
                        {
                            ui_cameraData_->setText(std::to_wstring(*pValue));
                        }
                    }
                    ui_cameraData_->setTextFormat(D14_FONT(L"Default/Normal/14"));
                    ui_cameraData_->setVisibleTextRect({ 5.0f, 5.0f, 95.0f, 35.0f });

                    GridLayout::GeometryInfo geoInfo = {};
                    geoInfo.isFixedSize = true;
                    geoInfo.axis.x = { offsetX, 2 };
                    geoInfo.axis.y = { offsetY, 1 };
                    ui_sideLayout->addElement(ui_cameraData_, geoInfo);

                    ui_cameraData_->f_onLoseFocus = [=](Panel* p)
                    {
                        updateCameraData((RawTextBox*)p, component);
                    };
                }
                cameraDataFocusGroup.push_back(ui_cameraData_);
            };
            createCameraDataComponentEditor(2, 'X');
            createCameraDataComponentEditor(4, 'Y');
            createCameraDataComponentEditor(6, 'Z');
        };
        createCameraDataEditor(L"Eye Pos", 3, 'P', std::nullopt);
        createCameraDataEditor(L"Eye Dir", 4, 'D', XMFLOAT3{ 0.0f, 0.0f, 0.0f });
        generateTabLink(cameraDataFocusGroup);
    });
}
