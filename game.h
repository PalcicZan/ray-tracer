#pragma once

namespace Tmpl8 {
class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp(int button) { handleCameraRotation = false; };
	void MouseDown(int button) { handleCameraRotation = true; };
	void MouseMove(int x, int y);
	void KeyUp(int key);
	void KeyDown(int key);
private:
	bool handleCameraRotation;
	uint toggleInfoView;
	float deltaTime;
	Surface* screen;
};

}; // namespace Tmpl8