#include "precomp.h" 

static Camera camera;
static Scene scene;
static Renderer renderer;
#if OPTIMIZE
static JobManager *jm;
static RenderParallel *renderJob[NUM_OF_THREADS];
#endif
static int frame = 0;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init() {
	// initialize camera
	camera.Initialize(vec3(0, 0, 0), vec3(0, 0, -1), 60.0f);
	// initialize custom scene
	scene.Initialize();
	// initialize renderer with camera and scene
	renderer.Initialize(&camera, &scene, screen);

#if OPTIMIZE
	JobManager::CreateJobManager(NUM_OF_THREADS);
	jm = JobManager::GetJobManager();
	int sy = 0;
	int ey = SCRHEIGHT / NUM_OF_THREADS;
	for (int i = 0; i < NUM_OF_THREADS; i++) {
		renderJob[i] = new RenderParallel(i, sy, ey, &renderer);
		sy += SCRHEIGHT / NUM_OF_THREADS;
		ey += SCRHEIGHT / NUM_OF_THREADS;
	}
#endif
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown() {
}

void Game::MouseMove(int x, int y) {
	if (handleCameraRotation) {
		camera.LookAt(vec3((float)x, 0.0f, (float)y), this->deltaTime);
	}
}

void Game::KeyUp(int key) {
	switch (key) {
	case SDL_SCANCODE_LSHIFT:
		camera.moveStep = 0.05f;
		camera.rotateStep = 0.0025f;
		break;
	case SDL_SCANCODE_CAPSLOCK:
		int capsOn = SDL_GetModState() & KMOD_CAPS;
		if (capsOn) {
			camera.moveStep = 0.5f;
			camera.rotateStep = 0.005f;
		} else {
			camera.moveStep = 0.05f;
			camera.rotateStep = 0.0025f;
		}
		break;
	}
}

void Game::KeyDown(int key) {
	printf("%d \n", key);
	switch (key) {
	case SDL_SCANCODE_W:
		camera.Move(Camera::Direction::FORWARD, deltaTime);
		break;
	case SDL_SCANCODE_S:
		camera.Move(Camera::Direction::BACKWARD, deltaTime);
		break;
	case SDL_SCANCODE_A:
		camera.Move(Camera::Direction::LEFT, deltaTime);
		break;
	case SDL_SCANCODE_D:
		camera.Move(Camera::Direction::RIGHT, deltaTime);
		break;
	case SDL_SCANCODE_Q:
		camera.Move(Camera::Direction::UP, deltaTime);
		break;
	case SDL_SCANCODE_E:
		camera.Move(Camera::Direction::DOWN, deltaTime);
		break;
	case SDL_SCANCODE_LSHIFT:
		camera.moveStep = 0.5;
		camera.rotateStep = 0.005;
		break;
	case SDL_SCANCODE_X:
		camera.SetFov(camera.GetFov() + 1.0f);
		camera.SetInterpolationStep();
		camera.UpdateCamera();
		break;
	case SDL_SCANCODE_Z:
	case SDL_SCANCODE_Y:
		camera.SetFov(camera.GetFov() - 1.0f);
		camera.SetInterpolationStep();
		camera.UpdateCamera();
		break;
	}
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime) {
	this->deltaTime = deltaTime;
	// clear the graphics window
	screen->Clear(scene.GetBackgroundP());
#if OPTIMIZE
	RenderParallel::BalanceWorkload(renderJob, jm->GetNumThreads());
	for (int i = 0; i < NUM_OF_THREADS; i++) jm->AddJob2(renderJob[i]);
	jm->RunJobs();
	screen->Print("Whitted ray tracer v1.00 Beta", 2, 2, 0xffffffff);
#else
	// Sim whole screen
	renderer.Sim(0, SCRHEIGHT);
#endif
}