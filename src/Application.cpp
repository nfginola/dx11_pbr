#include "pch.h"
#include "Application.h"
#include "Engine.h"
#include "Input.h"
#include "Scene.h"
#include "Timer.h"

namespace Gino
{
	Application::Application(Settings& settings) :
		m_appIsAlive(true)
	{
		InitWindow(settings);
		InitConsoleCommands();
		
		// Other settings such as Shadow Map resolution, and other misc. will go through here
		Engine::Settings engineSettings
		{
			.hwnd = m_mainWindow->GetHWND(),
			.vsync = true,
			
			.resolutionWidth = settings.windowWidth,
			.resolutionHeight = settings.windowHeight
		};

		m_engine = std::make_unique<Engine>(engineSettings);
		m_scene = std::make_unique<Scene>(m_engine.get());
    }

    Application::~Application()
    {


    }

	void Application::Run()
	{
		// Loop
		float dt = 0.f;
		while (m_appIsAlive)
		{
			Timer frameTimer;
			m_mainWindow->PumpMessages();

			m_engine->SimulateAndRender(dt);
			dt = frameTimer.TimeElapsed();

			/*


			REMINDER ============== CTRL + F and look for "NOTE" to read your notes!!

			Engine->SimulateAndRender();

			Typical functionality we need inside our Engine
				NOTE: Do we want ECS again? How do we move around "WorldObjects" which all have transforms?
						What kind of separation do we do with Lights/Models? (They are both world objects, but have different structures)
						Look at ArcaneEngine for reference and maybe decide (currently leaning towards using EnTT)

				NOTE: Do we want to directly store with ID3D11ConstantBuffer and other primitives? Or create simple wrappers such as: "Texture" and "Buffer"?
						We can easily unwrap them for simultaneous API binds if they are designed with getters (and not some .Bind() functionality)

				- Model (We should go with our typical RenderUnit setup)
				- LoadModel (Assimp)
				- LoadTexture (stb image)

				-- We will split our Renderer into different submodules
				-- NOTE: Where will light live?
				- MasterRenderer (Should have a toggle to switch between Forward, Deferred and Forward+ later on)

				++++ NOTE: We WILL pass native Device and DeviceContext to various submodules! We are doing this IN Direct3D11 with no consideration for porting the code
				++++	This constraint will help us minimize complexity! We want to focus on implementing techniques and not on future proofing.
				++++	Reason: DX11 is still industry standard!
				++++	We keep this in "functional style" on the surface (some submodulse returning relevant data, e.g ShadowMapResult, to pass on to other submodules that need it)
				++++	Our reusability constraint will be "reusable within a DX11 application", meaning submodules such as "ShadowMapper" can be used in both Forward, Deferred and Forward+!

					- SkyboxRenderer	--> Takes in skybox texture data and renders skybox

						(We dont necessarily have to implement all three, just a modelling suggestion to get a better picture)
					- ShadowMapperDirectional		--> Takes in meshes without material and renders shadow map data
					- ShadowMapperPointLight		--> Takes in meshes without material and renders shadow map data
					- ShadowMapperSpotlight			--> Takes in meshes without material and renders shadow map data

					- MeshRenderer		--> Takes in meshes with material and draws the meshes with material
										--> This may take in more.. such as light data, and shadow map data (e.g Forward) - Look at ArcaneEngine for reference on how to tackle this!

					-..(Future).. TerrainRenderer (will probably employ Tesselation)
					-..(Future).. WaterRenderer (will probably employ Tesselation)

					(this is made up, it could very well be integrated as an instanced ver. of Mesh Renderer)
					-..(Future).. TreeRenderer (special instanced with regards to Terrain data)

					-..(Future).. ParticleRenderer (will probably employ Compute Shader and other stages for geometry generation perhaps)

					- PostProcessor		--> Takes in final framebuffer and applies post processing
					--> After all that, COPY the final texture into the swapchain image

					NOTE: We may want to take a look at VQEngine's usage of RenderPass.
							Above idea "can be" simplified into passes, but it may be overcomplicating the problem!
							for example:
								- shadowPass { render meshes no mat },
								- geometryPass { render mesh, terrain, water, tree with mat }
								- postProcPass { take framebuffer, apply postproc, copy to texture }

					(Functions)
				- MasterRenderer
					- Render()

					- SubmitModelOpaque(Model)					[0, n]
					- SubmitModelTransparent(model)				[0, n]
					-..(Future).. SubmitTerrain(terrain)		[0, n] // Preferrably one?
					-..(Future).. SubmitWaterBody(waterBody)	[0, n]
					-..(Future).. SubmitParticleSystem(system)	[0, n]

					--> If we want to implement CPU side frustum culling, we want to do it outside of MasterRenderer.
					--> We can then employ a RenderAndFlush method and only submit models to the MasterRenderer that are visible

					- SetSkybox(skyboxData)						[1]

					(Below is a suggestion for Light data interface >> Strongly typed structs)
						(We will have max amount of point lights and spotlights)
							(Changing max amount on the fly would probably require some #defines and we can look into that much later since its not super relevant)
							(https://consent.youtube.com/m?continue=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DnyItqF3sM84&gl=SE&m=0&pc=yt&uxe=23983172&hl=en&src=1)
								(In that video, he updates the point light amount, try to find out how he does it later down the line)

					- SetDirectionalLightParameters(param)		[1]
					- SetPointLightParameters(params)			[0, n]
					- SetSpotlightParameters(params)			[0, n]



					..
					..
					- Functions to change settings for the various submodules
					..
					..

			*/
		}
	}

	bool Application::IsAlive() const
	{
		return m_appIsAlive;
	}

	void Application::ParseConsoleInput(std::string input)
	{
		// Convert input to lowercase
		std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); });

		// Alternatively, we can do this without needing locks for individual action
		// We can instead simply push a function to a ParsedCommand Queue
		// which the main Application thread can take care of at any time within the frame (e.g begin or end)
		// The only lock needed will be on popping the queue and pushing to the queue.
		// This means we would need to make a SharedQueue structure which lets us push and pop in a thread-safe manner
		// If we do want some function to happen asynchronously (e.g Model Loading), then we can dispatch threads for that and handle that specifically
		auto it = std::find_if(m_consoleCommands.begin(), m_consoleCommands.end(), [&input](const auto& command) { return command.first == input; });
		if (it != m_consoleCommands.end())
		{
			std::cout << "Command: '" << input << "' success!\n";
			auto& action = ((*it).second);
			action();
		}
		else
		{
			std::cout << "Command: '" << input << "' is an invalid command!\n";		
		}
		
		std::cout << std::endl;
	}

	void Application::InitWindow(Settings& settings)
	{
		// Init window
		auto mainWindowProc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->MainWindowProc(hwnd, uMsg, wParam, lParam); };
		m_mainWindow = std::make_unique<Window>(settings.hInstance, mainWindowProc, settings.windowWidth, settings.windowHeight);

		// Set fullscreen on start if specified
		if (!m_mainWindow->IsFullscreen() && settings.fullscreenOnStart)
		{
			m_mainWindow->SetFullscreen(true);
		}
	}

	void Application::InitConsoleCommands()
	{
		// Init functions
		auto appKillCommand = [this]() { KillApp(); };

		// Assign functions
		m_consoleCommands.insert({ "q", appKillCommand });
		m_consoleCommands.insert({ "quit", appKillCommand });
	}

	void Application::KillApp()
	{
		// Can be killed from console or app
		std::lock_guard<std::mutex> guard(m_appKillMutex);
		m_appIsAlive = false;
	}

	LRESULT Application::MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
		if (m_appIsAlive && m_engine)
		{
			auto imGuiFunc = m_engine->GetImGuiHook();
			if (imGuiFunc)
			{
				imGuiFunc(hwnd, uMsg, wParam, lParam);
			}
		}

		switch (uMsg)
		{
		// DirectXTK Mouse and Keyboard (Input)
		case WM_ACTIVATEAPP:
		{
			if (m_engine && m_engine->GetInput())
			{
				m_engine->GetInput()->ProcessKeyboard(uMsg, wParam, lParam);
				m_engine->GetInput()->ProcessMouse(uMsg, wParam, lParam);
			}
			break;
		}
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
		{
			if (m_engine && m_engine->GetInput())
			{
				m_engine->GetInput()->ProcessMouse(uMsg, wParam, lParam);
			}
			break;
		}

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				KillApp();
				break;
			}
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (m_engine && m_engine->GetInput())
			{
				m_engine->GetInput()->ProcessKeyboard(uMsg, wParam, lParam);
			}
			break;

		// Universal quit message
		case WM_CLOSE:
		{
			KillApp();
			break;
		}

		// Resize message
		case WM_SIZE:
		{
			// NOTE: We may want to turn off so that we can scale freely in both dimensions.
			// Because we would like to keep the aspect ratio of the initial resolution! To not handle it, lets just turn off free scaling and resize only on Fullscreen enter/exit

			// We want to hook this to ImGui viewport later
			//if (m_resizeCallback)
			//	m_resizeCallback(LOWORD(lParam), HIWORD(lParam));
			break;
		}

		case WM_EXITSIZEMOVE:
		{
			//std::cout << "Should resize\n";
			break;
		}

		// Input message
		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
			{
				// Custom Alt + Enter to toggle windowed borderless
				m_mainWindow->SetFullscreen(!m_mainWindow->IsFullscreen());
				// Resizing will be handled through WM_SIZE through a subsequent WM
			}
			break;
		}

		default:
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		}
		
		return 0;
    }

}
