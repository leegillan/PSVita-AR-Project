#include "EndScreen.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>

#include <input/sony_controller_input_manager.h>
#include <sony_sample_framework.h>

EndScreen::EndScreen()
{
}


EndScreen::~EndScreen()
{
}

void EndScreen::Init(gef::Platform & platform)
{
}

void EndScreen::CleanUp()
{
}

bool EndScreen::Update(float frame_time)
{
	return false;
}

void EndScreen::Render(gef::SpriteRenderer & spriteRenderer, gef::Renderer3D & renderer3D)
{
	spriteRenderer.Begin(true);

	spriteRenderer.End();
}

void EndScreen::Input(gef::SonyControllerInputManager* controller, GameState& gameState, Game& game)
{
	if (controller)
	{
		if (controller->GetController(0)->buttons_pressed() & gef_SONY_CTRL_CROSS)
		{
			game.ResetGame(&gameState);

			gameState.SetGameState(MAINMENU);
		}
	}
}

void EndScreen::DrawHUD(gef::Font * font, gef::SpriteRenderer * spriteRenderer, int points, GameState* gameState)
{
	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.45f, (float)DISPLAY_HEIGHT * 0.3f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "Well done!");

	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3f, (float)DISPLAY_HEIGHT * 0.4f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "You managed to get %d points!", points);

	if (gameState->GetGameDifficulty() == EASY)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.5f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "Difficulty: Easy");
	}
	else if (gameState->GetGameDifficulty() == MEDIUM)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.5f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "Difficulty: Medium");
	}
	else if (gameState->GetGameDifficulty() == HARD)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.5f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "Game difficulty: Hard");
	}

	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3f, (float)DISPLAY_HEIGHT * 0.7f, -0.9f), 1.3f, 0xffffffff, gef::TJ_LEFT, "Press X to go back to main menu!");
}