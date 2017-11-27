#include "precomp.h" 

static Camera camera;
static Scene scene;
static Renderer renderer;
static JobManager *jm;
static RenderParallel *rendererJob[4];

static int frame = 0;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	// initialize camera
	camera.Initialize(vec3(0, 0, 0), vec3(0, 0, -1), 90.0f);
	// initialize custom scene
	scene.Initialize();
	// initialize renderer with camera and scene
	renderer.Initialize(&camera, &scene, screen);

#if OPTIMIZE
	JobManager::CreateJobManager(4);
	jm = JobManager::GetJobManager();
	rendererJob[0] = new RenderParallel(0, SCRHEIGHT / 4, &renderer);
	rendererJob[1] = new RenderParallel(SCRHEIGHT / 2, SCRHEIGHT / 4 * 3, &renderer);
	//rendererJob[0] = new RenderParallel(0, SCRHEIGHT / 4, &renderer);v
	rendererJob[2] = new RenderParallel(SCRHEIGHT / 4, SCRHEIGHT / 2, &renderer);
	//rendererJob[2] = new RenderParallel(SCRHEIGHT / 2, SCRHEIGHT / 4 * 3, &renderer);
	rendererJob[3] = new RenderParallel(SCRHEIGHT / 4 * 3, SCRHEIGHT, &renderer);
#endif
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
	// clear the graphics window
	screen->Clear(SetPixelColor(scene.GetBackground()));

#if OPTIMIZE
	jm->AddJob2(rendererJob[0]);
	jm->AddJob2(rendererJob[1]);
	jm->AddJob2(rendererJob[2]);
	jm->AddJob2(rendererJob[3]);
	jm->RunJobs();
#else
	// Sim whole screen
	renderer.Sim(0, SCRHEIGHT)
#endif
	screen->Print("Whitted ray tracer v0.5", 2, 2, 0xffffffff);
}

void Game::MouseMove(int x, int y)
{
	if (handleCameraRotation)
	{
		camera.LookAt(vec3((float)x, 0.0f, (float)y));
	}
}

void Game::KeyUp(int key)
{
}

void Game::KeyDown(int key)
{
	switch (key)
	{
	case SDL_SCANCODE_W:
		camera.Move(Camera::Direction::FORWARD);
		break;
	case SDL_SCANCODE_S:
		camera.Move(Camera::Direction::BACKWARD);
		break;
	case SDL_SCANCODE_A:
		camera.Move(Camera::Direction::LEFT);
		break;
	case SDL_SCANCODE_D:
		camera.Move(Camera::Direction::RIGHT);
		break;
	case SDL_SCANCODE_Q:
		camera.Move(Camera::Direction::UP);
		break;
	case SDL_SCANCODE_E:
		camera.Move(Camera::Direction::DOWN);
		break;
	}
}
