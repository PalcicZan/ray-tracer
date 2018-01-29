#include "precomp.h" 

static Camera camera;
static Scene scene;
static Renderer *renderer;
static BVH *bvh;
static vec3 accumulatedEnergy;
static int nSample = 0;
#if OPTIMIZE
static JobManager *jm;
static RenderParallel *renderJob[NUM_OF_THREADS];
#endif

#if MEASURE_PERFORMANCE
// time counters
static timer t;
static float bvhConstuctionTime;
static float bvhTraverseTime;
static float bvhAllTime = 0.0f;
static int nFrame = 0;
#endif

/*===================================================*/
/*|	Initialize the application						|*/
/*===================================================*/
void Game::Init() {
	// initialize custom scene
	scene.Initialize();
	screen->Clear(0);
	// initialize camera
	//camera.Initialize(vec3(0, 0, 0), vec3(0, 0, -1), 60.0f);
	camera.Initialize(vec3(0, 0, 500), vec3(0, 0, -1), 60.0f);
	camera.SetMoveStep(20.0f);
	// initialize renderer with camera and scene
	renderer = new Renderer(&camera, &scene, screen, &nSample, &nFrame);
	//renderer.Initialize(&camera, &scene, screen);

#if USE_BVH
	bvh = new BVH(renderer->scene->GetPrimitives(), renderer->scene->GetNumberOfPrimitives());// - renderer->scene->GetNumberOfLights());
	bvh->SetSplitMethod(BVH::SplitMethod::ExhaustiveSAH);
	bvh->ConstructBVH(renderer->scene->GetPrimitives(), renderer->scene->GetNumberOfPrimitives());// - renderer->scene->GetNumberOfLights());
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

void Game::Shutdown() {
}

void Game::MouseMove(int x, int y) {
	if (handleCameraRotation) {
		camera.LookAt(vec3((float)x, 0.0f, (float)y), this->deltaTime);
		nSample = 0;
	}
}

void Game::KeyUp(int key) {
	switch (key) {
	case SDL_SCANCODE_W:
	case SDL_SCANCODE_S:
	case SDL_SCANCODE_A:
	case SDL_SCANCODE_D:
	case SDL_SCANCODE_E:
	case SDL_SCANCODE_Q:
		renderer->moving = false;
		break;
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
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_S:
		camera.Move(Camera::Direction::BACKWARD, deltaTime);
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_A:
		camera.Move(Camera::Direction::LEFT, deltaTime);
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_D:
		camera.Move(Camera::Direction::RIGHT, deltaTime);
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_Q:
		camera.Move(Camera::Direction::UP, deltaTime);
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_E:
		camera.Move(Camera::Direction::DOWN, deltaTime);
		nSample = 0;
		renderer->moving = true;
		break;
	case SDL_SCANCODE_KP_PLUS:
		camera.moveStep += 0.5;
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
			   bvhAllTime / (nFrame), bvhTraverseTime, bvhConstuctionTime, REF_SPEED_TREE / (bvhTraverseTime + bvhConstuctionTime), NUM_OF_THREADS);
		toggleInfoView = !toggleInfoView;
		break;
	case SDL_SCANCODE_1:
		renderer->toggleRenderView = (renderer->toggleRenderView + 1) % 3;
		break;
	case SDL_SCANCODE_2:
		renderer->toggleSplitMethod = (renderer->toggleSplitMethod + 1) % 3;
		switch (renderer->toggleSplitMethod) {
		case 0:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::Median);
			break;
		case 1:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::ExhaustiveSAH);
			break;
		case 2:
			renderer->bvh->SetSplitMethod(BVH::SplitMethod::BinnedSAH);
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

void Game::ShowInfo() {
	int offset = 2;
	char buff[100];
	screen->Print(RAY_TRACER_VERSION, 2, offset, 0xffffffff);
	sprintf(buff, "Num of primitives: %d", scene.GetNumberOfPrimitives());
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
#if USE_BVH
	sprintf(buff, "Time of BVH traverse: %.3lf ms", bvhTraverseTime);
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
#if SCENE == 2
	sprintf(buff, "Speed up: %.2lfx ", REF_SPEED_TREE / currAll);
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
#endif
#endif
	sprintf(buff, "Num of threads: %d", NUM_OF_THREADS);
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
	accumulatedEnergy = accumulatedEnergy * (1.0f / (SCRHEIGHT*SCRWIDTH));
	sprintf(buff, "Accumulated energy: %.5lf %.5lf %.5lf", accumulatedEnergy.x, accumulatedEnergy.y, accumulatedEnergy.z);
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
#if SCENE == TEST_SCENE
#if VARIANCE_REDUCTION == COSINE
	sprintf(buff, "Ref energy (SCENE): %.5lf %.5lf %.5lf", R_REF, G_REF, B_REF);
#else
	sprintf(buff, "Ref energy (SCENE): %.5lf %.5lf %.5lf", R_REF_H, G_REF_H, B_REF_H);
#endif
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
#endif
	sprintf(buff, "Sample: %d", nSample);
	offset += 7;
	screen->Print(buff, 2, offset, 0xffffffff);
}

/*===================================================*/
/*|	Main application tick function					|*/
/*===================================================*/
void Game::Tick(float deltaTime) {
	this->deltaTime = deltaTime;
	// construct BVH on demand
#if USE_BVH
	if (renderer->enableBvhReconstruction) {
		t.reset();
		bvh->ReconstructBVH(renderer->scene->GetPrimitives(), renderer->scene->GetNumberOfPrimitives());
		bvhConstuctionTime = t.elapsed();
	}
#endif
#if OPTIMIZE
	//renderer->moving |= handleCameraRotation;
	nFrame++;
	nSample++;
	renderer->UpdateSeed();
	RenderParallel::BalanceWorkload(renderJob, NUM_OF_THREADS);
	for (int i = 0; i < NUM_OF_THREADS; i++) {
		jm->AddJob2(renderJob[i]);
	}
#if MEASURE_PERFORMANCE
	// traverse
	t.reset();
	jm->RunJobs();
	bvhTraverseTime = t.elapsed();
	bvhAllTime += bvhTraverseTime + bvhConstuctionTime;
	// calculate accmulated energy
	accumulatedEnergy = vec3(0.0f);
	for (int i = 0; i < NUM_OF_THREADS; i++) accumulatedEnergy += renderJob[i]->accColor;
	// show info panel
	if (toggleInfoView) ShowInfo();
#if PATH_TRACER == SIMPLE
	if (nSample == 100)
		printf("%lf, %lf, %lf\n", accumulatedEnergy.x, accumulatedEnergy.y, accumulatedEnergy.z);
#endif
	//	printf("%lf, %lf, %lf\n", abs(R_REF - accumulatedEnergy.x)/R_REF*100.f, abs(G_REF - accumulatedEnergy.y) / G_REF * 100.f, abs(B_REF - accumulatedEnergy.z) / B_REF * 100.f);
#else
	jm->RunJobs();
#endif
#else
	// Sim whole screen
	renderer.Sim(0, SCRHEIGHT);
#endif
}