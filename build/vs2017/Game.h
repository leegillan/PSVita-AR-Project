#pragma once
#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <vector>
#include <graphics/mesh_instance.h>
#include <platform/vita/graphics/texture_vita.h>
#include "primitive_builder.h"
#include "primitive_renderer.h"
#include "game_object.h"
#include "GameState.h"
#include "Line.h"

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
	class Scene;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class Mesh;
	class RenderTarget;
	class TextureVita;
	class InputManager;
	class SonyControllerInputManager;
	class Material;
}

struct Reflectors
{
	game_object* object;

	int markerID;
	bool markerFound;
	gef::Matrix44 markerTransform;

	bool collision;
};

struct Walls
{
	game_object* object;

	bool hasHole;
	bool hasTarget;
	bool collision;
};

class Game
{
public:
	Game();
	~Game();

	void Init(gef::Platform& platform, PrimitiveBuilder* primitive_renderer);
	void CleanUp();
	bool Update(float frame_time, GameState& gameState);
	void Render(gef::SpriteRenderer& spriteRenderer, gef::Renderer3D& renderer3D, PrimitiveRenderer* primitiveRenderer, PrimitiveBuilder* primitiveBuilder, gef::Platform& platform);
	void Input(gef::SonyControllerInputManager* controller, GameState& gameSate);
	void DrawHUD(gef::Font* font, gef::SpriteRenderer* spriteRenderer);

	void ResetGame(GameState* gameState);

	int GetPoints() { return points; }

private:
	void UpdateLaserArea(GameState& gameState, gef::Matrix44 markerTransform);
	void UpdateReflector(int i, gef::Matrix44 markerTransfor);

	void InitCamera(gef::Platform& platform_);
	float CalcImageScaleFactor();

	void InitGameObjects(PrimitiveBuilder* primitive_renderer, gef::Platform& platform);
	void SetWallPositions(gef::Platform& platform);

	void CreateLines();
	void SetLineDescription(Lines line, std::string name, const gef::Matrix44& transform, bool xAxis, const float xSize, float xAxisOffset, bool yAxis, const float ySize, float yAxisOffset, std::string laserColour);
	
	void DrawLines(PrimitiveRenderer* primitiveRenderer, gef::Renderer3D* renderer3D, PrimitiveBuilder* primitiveBuilder, gef::Platform& platform);
	void DrawAxes(PrimitiveRenderer* primitive_renderer, const gef::Matrix44& transform, bool xAxis, const float xSize, float xAxisOffset, bool yAxis, const float ySize, float yAxisOffset, std::string laserColour);

	void LaserCollisionCheck(PrimitiveBuilder* primitiveBuilder, gef::Platform& platform);
	bool IsLineIntersecting(gef::Vector2 a, gef::Vector2 b, gef::Vector2 c, gef::Vector2 d);

	float Distance(float x1, float y1, float x2, float y2);
	float RandomFloat(float a, float b);
	int RandomInt(int max, int min);

	gef::Mesh* GetFirstMesh(gef::Scene* scene, gef::Platform& platform);

	//camera
	gef::Matrix44 scaleMat;
	gef::Matrix44 identityMat;
	gef::Matrix44 orthoMat;
	gef::Matrix44 proj3DMat;

	gef::TextureVita* camTex;
	gef::Sprite camSprite;

	//game objects
	game_object* laserBox;
	game_object* target;
	std::vector<Walls> walls;
	std::vector<Reflectors> reflectors;

	//Model variables
	gef::Scene* modelScene;
	class gef::Mesh* mesh;

	// marker variables//
	//transforms
	gef::Matrix44 laserBoxMarkerTransform;
	gef::Matrix44 targetMarkerTransform;

	//Line variables
	std::vector<Lines> lineVect;
	gef::Vector2 lineStart;
	gef::Vector2 lineEnd;

	//// Laser variables ///
	bool laserCollideTarget;

	//Laser sizes
	float laserSize;
	int maxLaserSize = 1.0f;
	float reflectorLaserSize[2];
	int maxReflectorLaserSize = 3.0f;

	//checks
	bool firstMarkerCheck;
	bool secondMarkerCheck;
	bool thirdMarkerCheck;

	bool laserMarkerFound;
	bool reflectedLine1;
	bool reflectedLine2;

	//misc variables
	int points;
	bool colTimer;
	float timer;
	float countdown;

	float deltaTime;
};

