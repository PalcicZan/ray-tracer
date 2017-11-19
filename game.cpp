#include "precomp.h" 

static Camera camera;
static Scene scene;
static Renderer renderer;
static int frame = 0;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	// initialize camera
	camera.Initialize(vec3(0, 0, 0), vec3(0, 0, -4), 60.0f);
	// initialize custom scene
	scene.Initialize();
	scene.SetBackground(vec3(0.529f, 0.808f, 0.980f));
	// initialize renderer with camera and scene
	renderer.Initialize(camera, scene);
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
	printf("Frame %d.\n", ++frame);
	// go through all pixels
	for (int i = 0; i < SCRHEIGHT; i++)
	{
		for (int j = 0; j < SCRWIDTH; j++)
		{
			Ray r = camera.CastRayGeneral(j, i);
			vec3 color = renderer.Trace(r, 0);
			screen->Plot(j, i, SetPixelColor(color));
		}
	}
	screen->Print("Whitted ray tracer v0.5", 2, 2, 0xffffffff);
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
