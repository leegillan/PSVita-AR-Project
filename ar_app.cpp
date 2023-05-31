#include "ar_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <assets/png_loader.h>
#include <graphics/image_data.h>
#include <graphics/font.h>
#include <input/touch_input_manager.h>
#include <input/keyboard.h>
#include <input/sony_controller_input_manager.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/render_target.h>
#include <sstream>

#include <sony_sample_framework.h>
#include <sony_tracking.h>

ARApp::ARApp(gef::Platform& platform) :
	Application(platform),
	input_manager_(NULL),
	sprite_renderer_(NULL),
	font_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	primitive_renderer_(NULL)
{
}

void ARApp::Init()
{
	input_manager_ = gef::InputManager::Create(platform_);
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	renderer_3d_ = gef::Renderer3D::Create(platform_);
	primitive_builder_ = new PrimitiveBuilder(platform_);
	primitive_renderer_ = new PrimitiveRenderer(platform_);

	InitFont();
	SetupLights();

	//Initialisations
	gameState.Init();
	mainMenu.Init(platform_);
	game.Init(platform_, primitive_builder_);
	endScreen.Init(platform_);

	deltaTime = 0;
}

void ARApp::CleanUp()
{
	mainMenu.CleanUp();
	game.CleanUp();
	endScreen.CleanUp();

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete primitive_renderer_;
	primitive_renderer_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;
}

bool ARApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	Input();

	//Game Logic
	switch (gameState.GetGameState())
	{
		case MAINMENU:
			mainMenu.Update(frame_time);
			break;

		case GAME:
			game.Update(frame_time, gameState);
			break;

		case PAUSE:
			break;

		case ENDSCREEN:
			endScreen.Update(frame_time);
			break;
	}

	return true;
}

void ARApp::Input()
{
	if (input_manager_)
	{
		input_manager_->Update();

		gef::SonyControllerInputManager* controller = input_manager_->controller_input();

		//Game Logic
		switch (gameState.GetGameState())
		{
		case MAINMENU:
			mainMenu.Input(controller, gameState, game);
			break;

		case GAME:
			game.Input(controller, gameState);
			break;

		case PAUSE:
			game.Input(controller, gameState);
			break;

		case ENDSCREEN:
			endScreen.Input(controller, gameState, game);
			break;
		}
	}
}

void ARApp::Render()
{
	//Game Logic
	switch (gameState.GetGameState())
	{
	case MAINMENU:
		mainMenu.Render(*sprite_renderer_, *renderer_3d_);
		break;

	case GAME:
		game.Render(*sprite_renderer_, *renderer_3d_, primitive_renderer_, primitive_builder_, platform_);
		break;

	case PAUSE:
		break;

	case ENDSCREEN:
		endScreen.Render(*sprite_renderer_, *renderer_3d_);
		break;
	}

	RenderOverlay();
}

void ARApp::RenderOverlay()
{
	//
	// render 2d hud on top
	//
	gef::Matrix44 proj_matrix2d;

	proj_matrix2d = platform_.OrthographicFrustum(0.0f, platform_.width(), 0.0f, platform_.height(), -1.0f, 1.0f);
	sprite_renderer_->set_projection_matrix(proj_matrix2d);
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}


void ARApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void ARApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void ARApp::DrawHUD()
{
	if (font_)
	{
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);

		//Game Logic
		switch (gameState.GetGameState())
		{
		case MAINMENU:
			mainMenu.DrawHUD(font_, sprite_renderer_, &gameState);
			break;

		case GAME:
			game.DrawHUD(font_, sprite_renderer_);
			break;

		case PAUSE:
			game.DrawHUD(font_, sprite_renderer_);
			break;

		case ENDSCREEN:
			endScreen.DrawHUD(font_, sprite_renderer_, game.GetPoints(), &gameState);
			break;
		}

	}
}

void ARApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}