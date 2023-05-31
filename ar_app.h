#ifndef _RENDER_TARGET_APP_H
#define _RENDER_TARGET_APP_H

#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <vector>
#include <graphics/mesh_instance.h>
#include <platform/vita/graphics/texture_vita.h>
#include "build/vs2017/primitive_builder.h"
#include "build/vs2017/primitive_renderer.h"

#include "build/vs2017/game_object.h"
#include "build/vs2017/GameState.h"
#include "build/vs2017/Game.h"
#include "build/vs2017/EndScreen.h"
#include "build/vs2017/MainMenu.h"
#include "build/vs2017/Line.h"

// Vita AR includes
#include <camera.h>
#include <gxm.h>
#include <motion.h>
#include <libdbg.h>
#include <libsmart.h>

// FRAMEWORK FORWARD DECLARATIONS
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
}

class ARApp : public gef::Application
{
public:
	ARApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
	void Input();

private:
	void InitFont();
	void CleanUpFont();
	void DrawHUD();

	void RenderOverlay();
	void SetupLights();

	class gef::Renderer3D* renderer_3d_;
	gef::InputManager* input_manager_;
	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	PrimitiveBuilder* primitive_builder_;
	PrimitiveRenderer* primitive_renderer_;
	
	//Class Instances
	GameState gameState;
	Game game;
	EndScreen endScreen;
	MainMenu mainMenu;

	float fps_;
	float deltaTime;
};

#endif // _RENDER_TARGET_APP_H