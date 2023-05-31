#pragma once

#include <system/application.h>
#include <graphics/sprite.h>
#include "GameState.h"
#include "Game.h"

namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class Mesh;
	class RenderTarget;
	class TextureVita;
	class InputManager;
	class SonyControllerInputManager;
}

class MainMenu
{
public:
	MainMenu();
	~MainMenu();

	void Init(gef::Platform& platform);
	void CleanUp();
	bool Update(float frame_time);
	void Render(gef::SpriteRenderer& spriteRenderer, gef::Renderer3D& renderer3D);
	void Input(gef::SonyControllerInputManager* controller, GameState& gameSate, Game& game);
	void DrawHUD(gef::Font* font, gef::SpriteRenderer* spriteRenderer, GameState* gameState);

private:

};

