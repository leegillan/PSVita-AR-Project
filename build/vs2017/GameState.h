#pragma once

enum GAME_STATE
{
	MAINMENU,
	GAME,
	PAUSE,
	ENDSCREEN
};

enum GAME_DIFFICULTY
{
	EASY,
	MEDIUM,
	HARD,
};

class GameState
{
public:
	GameState();
	~GameState();

	void Init();

	//setters & getters
	void SetGameState(GAME_STATE gS) { gameState = gS; }
	GAME_STATE GetGameState() { return gameState; }

	void SetGameDifficulty(GAME_DIFFICULTY d) { difficulty = d; }
	GAME_DIFFICULTY GetGameDifficulty() { return difficulty; }

private:
	GAME_STATE gameState;
	GAME_DIFFICULTY difficulty;
};

