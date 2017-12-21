#include "precomp.h" 

static Camera camera;
static Scene scene;
static Renderer *renderer;
static BVH *bvh;
#if MEASURE_PERFORMANCE
static timer t;

// time counters
static float bvhConstuction;
static float bvhTraverse;
static float bvhAll = 0.0f;
static int frame = 0;
#endif
#if OPTIMIZE
static JobManager *jm;
static RenderParallel *renderJob[NUM_OF_THREADS];
#endif

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init() {
	// initialize camera
	camera.Initialize(vec3(0, 0, 0), vec3(0, 0, -1), 60.0f);
	// initialize custom scene
	scene.Initialize();
	// initialize renderer with camera and scene
	renderer = new Renderer(&camera, &scene, screen);
	//renderer.Initialize(&camera, &scene, screen);

#if USE_BVH
	bvh = new BVH();	
	bvh->SetSplitMethod(BVH::SplitMethod::ExhaustiveSAH);
	bvh->ConstructBVH(renderer->scene->GetPrimitives(), renderer->scene->GetNumberOfPrimitives() - renderer->scene->GetNumberOfLights());
	renderer->SetBVH(bvh);
#endif

#if OPTIMIZE
	JobManager::CreateJobManager(NUM_OF_THREADS);
	jm = JobManager::GetJobManager();
	int sy = 0;
	int ey = SCRHEIGHT / NUM_OF_THREADS;
	for (int i = 0; i < NUM_OF_THREADS; i++) {
		renderJob[i] = new RenderParallel(i, sy, ey, *renderer);
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
	case SDL_SCANCODE_I:
		printf("Avg time BVH ALL: %.3lf ms; Time of BVH traverse: %.3lf ms; Time of BVH construction: %.3lf ms; Speed up: %.3lfx \nNum of threads: %d\n",
			   bvhAll / (++frame), bvhTraverse, bvhConstuction, REF_SPEED_TREE / (bvhTraverse + bvhConstuction), NUM_OF_THREADS);
		toggleInfoView = !toggleInfoView;
		break;
	case SDL_SCANCODE_1:
		renderer->toggleRenderView = (renderer->toggleRenderView + 1) % 3;
		break;
	case SDL_SCANCODE_2:
		renderer->toggleSplitMethod = (renderer->toggleSplitMethod + 1) % 2;
		switch (renderer->toggleSplitMethod) {
		case 0:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::Median);
			break;
		case 1:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::ExhaustiveSAH);
			break;
		case 2:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::Median);
			break;
		default:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::Median);
			break;
		}
		break;
	case SDL_SCANCODE_3:
		renderer->enableBvhReconstruction ^= 1;
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
	// construct BVH when necessary
	bvhConstuction = 0.0f;
	if (renderer->enableBvhReconstruction) {
		t.reset();
		bvh->ReconstructBVH(renderer->scene->GetPrimitives(), renderer->scene->GetNumberOfPrimitives() - renderer->scene->GetNumberOfLights());
		renderer->SetBVH(bvh);
		bvhConstuction = t.elapsed();
	}
#if OPTIMIZE
	RenderParallel::BalanceWorkload(renderJob, jm->GetNumThreads());
	for (int i = 0; i < NUM_OF_THREADS; i++) jm->AddJob2(renderJob[i]);
#if MEASURE_PERFORMANCE
	t.reset();
	jm->RunJobs();
	bvhTraverse = t.elapsed();
	bvhAll += bvhTraverse + bvhConstuction;
	++frame;
	if (toggleInfoView) {
		int offset = 2;
		char buff[100];
		screen->Print("Whitted ray tracer v1.00 Beta", 2, offset, 0xffffffff);
		sprintf(buff, "Num of primitives: %d", scene.GetNumberOfPrimitives());
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
		sprintf(buff, "Time of BVH construction: %.3lf ms", bvhConstuction);
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
		sprintf(buff, "Time of BVH traverse: %.3lf ms", bvhTraverse);
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
		float currAll = bvhAll / (++frame);
		sprintf(buff, "Avg time BVH ALL: %.3lf ms", currAll);
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
		sprintf(buff, "Speed up: %.2lfx ", REF_SPEED_TREE / currAll);
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
		sprintf(buff, "Num of threads: %d", NUM_OF_THREADS);
		offset += 7;
		screen->Print(buff, 2, offset, 0xffffffff);
	}
#else
	jm->RunJobs();
#endif
#else
	// Sim whole screen
	renderer.Sim(0, SCRHEIGHT);
#endif
}