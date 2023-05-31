#include "MainMenu.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>

#include <input/sony_controller_input_manager.h>
#include <sony_sample_framework.h>

MainMenu::MainMenu()
{
}


MainMenu::~MainMenu()
{
}

void MainMenu::Init(gef::Platform & platform)
{
}

void MainMenu::CleanUp()
{
}

bool MainMenu::Update(float frame_time)
{
	return false;
}

void MainMenu::Render(gef::SpriteRenderer& spriteRenderer, gef::Renderer3D & renderer3D)
{
	spriteRenderer.Begin(true);

	spriteRenderer.End();
}

void MainMenu::Input(gef::SonyControllerInputManager * controller, GameState& gameState, Game& game)
{
	if (controller)
	{
		if (controller->GetController(0)->buttons_pressed() & gef_SONY_CTRL_CROSS)
		{
			game.ResetGame(&gameState);

			gameState.SetGameState(GAME);
		}

		if (controller->GetController(0)->buttons_pressed() & gef_SONY_CTRL_R2)
		{
			if (gameState.GetGameDifficulty() == EASY)
			{
				gameState.SetGameDifficulty(MEDIUM);
			}
			else if (gameState.GetGameDifficulty() == MEDIUM)
			{
				gameState.SetGameDifficulty(HARD);
			}
			else if(gameState.GetGameDifficulty() == HARD)
			{
				gameState.SetGameDifficulty(EASY);
			}
		}

		if (controller->GetController(0)->buttons_pressed() & gef_SONY_CTRL_L2)
		{
			if (gameState.GetGameDifficulty() == EASY)
			{
				gameState.SetGameDifficulty(HARD);
			}
			else if (gameState.GetGameDifficulty() == HARD)
			{
				gameState.SetGameDifficulty(MEDIUM);
			}
			else if (gameState.GetGameDifficulty() == MEDIUM)
			{
				gameState.SetGameDifficulty(EASY);
			}
		}
	}
}

void MainMenu::DrawHUD(gef::Font* font, gef::SpriteRenderer * spriteRenderer, GameState* gameState)
{
	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.35f, (float)DISPLAY_HEIGHT * 0.2f, -0.9f), 2.0f, 0xffffffff, gef::TJ_LEFT, "Laser Practice");

	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.36f, (float)DISPLAY_HEIGHT * 0.5f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Press X to Start Game");

	if (gameState->GetGameDifficulty() == EASY)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.7f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Difficulty: Easy");
	}
	else if (gameState->GetGameDifficulty() == MEDIUM)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.7f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Difficulty: Medium");
	}
	else if (gameState->GetGameDifficulty() == HARD)
	{
		font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.4f, (float)DISPLAY_HEIGHT * 0.7f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Difficulty: Hard");
	}
	
	font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3f, (float)DISPLAY_HEIGHT * 0.8f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Press L2/R2  to change difficulty");
}
