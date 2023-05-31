#include "Game.h"
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
#include <graphics/scene.h>

#include "primitive_builder.h"
#include "primitive_renderer.h"

#include <sony_sample_framework.h>
#include <sony_tracking.h>

Game::Game()
{
}

Game::~Game()
{
}

void Game::Init(gef::Platform& platform_, PrimitiveBuilder* primitiveBuilder)
{
	camTex = new gef::TextureVita();

	InitCamera(platform_);

	InitGameObjects(primitiveBuilder, platform_);
	
	//set positions of walls and set target to be at one of the walls
	SetWallPositions(platform_);

	points = 0;
	timer = 0;
	countdown = 30.0f;
	laserSize = 0.5f;

	for (int i = 0; i < 2; i++)
	{
		reflectorLaserSize[i] = 0.5f;
	}
}

void Game::CleanUp()
{
	//Game Logic
	delete laserBox;
	laserBox = NULL;

	for (int i = 0; i < 2; i++)
	{
		delete reflectors[i].object;
		reflectors[i].object = NULL;
	}

	for (int i = 0; i < walls.size; i++)
	{
		delete walls[i].object;
		walls[i].object = NULL;
	}

	delete target;
	target = NULL;

	delete modelScene;
	modelScene = NULL;

	delete target;
	target = NULL;

	smartRelease();
	sampleRelease();
}

bool Game::Update(float frameTime, GameState& gameState)
{
	AppData* dat = sampleUpdateBegin();

	// use the tracking library to try and find markers
	smartUpdate(dat->currentImage);

	deltaTime = frameTime;

	for (int i = 0; i < 6; i++)
	{
		if (sampleIsMarkerFound(i))
		{
			// marker is being tracked, get its transform
			gef::Matrix44 markerTransform;
			sampleGetTransform(i, &markerTransform);

			switch (i)
			{
				case 0:
					if (firstMarkerCheck && secondMarkerCheck && thirdMarkerCheck)
					{
						UpdateLaserArea(gameState, markerTransform);
						laserMarkerFound = true;
					}
					else
					{
						firstMarkerCheck = true;
					}
					break;

				case 1:
					if (firstMarkerCheck && secondMarkerCheck && thirdMarkerCheck)
					{
					UpdateReflector(0, markerTransform);
					}
					else if(firstMarkerCheck)
					{
						secondMarkerCheck = true;
					}
					break;

				case 2:
					if (firstMarkerCheck && secondMarkerCheck && thirdMarkerCheck)
					{
					UpdateReflector(1, markerTransform);
					}
					else if (secondMarkerCheck)
					{
						thirdMarkerCheck = true;
					}
					break;

				default:
			}
		}
		else if (!sampleIsMarkerFound(i))
		{
			switch (i)
			{
				case 0:
					laserMarkerFound = false;
					break;

				case 1:
					reflectors[0].markerFound = false;
					break;

				case 2:
					reflectors[1].markerFound = false;
					break;

				default:
			}
		}
	}

	sampleUpdateEnd(dat);
	
	return true;
}

void Game::UpdateLaserArea(GameState& gameState, gef::Matrix44 markerTransform)
{
	if (countdown <= 0)
	{
		gameState.SetGameState(ENDSCREEN);
		return;
	}
	else
	{
		countdown -= deltaTime;
	}

	if (laserSize < maxLaserSize)
	{
		laserSize += deltaTime * 0.1f;
	}

	for (int i = 0; i < 2; i++)
	{
		if (reflectorLaserSize[i] < maxReflectorLaserSize * 3.0f)
		{
			reflectorLaserSize[i] += deltaTime * 0.5f;
		}
	}

	laserBoxMarkerTransform = markerTransform;

	for (int i = 0; i < walls.size(); i++)
	{
		walls[i].object->Update(deltaTime, markerTransform);
	}

	laserBox->Update(deltaTime, markerTransform);
	target->Update(deltaTime, markerTransform);
}

void Game::UpdateReflector(int i, gef::Matrix44 markerTransform)
{
	reflectors[i].markerTransform = markerTransform;
	reflectors[i].markerFound = true;

	reflectors[i].object->Update(deltaTime, markerTransform);
}

void Game::Render(gef::SpriteRenderer& spriteRenderer, gef::Renderer3D& renderer3D, PrimitiveRenderer* primitiveRenderer, PrimitiveBuilder* primitiveBuilder, gef::Platform& platform)
{
	AppData* dat = sampleRenderBegin();

	spriteRenderer.set_projection_matrix(orthoMat);

	spriteRenderer.Begin(true);

	if (dat->currentImage)
	{
		camTex->set_texture(dat->currentImage->tex_yuv);
		camSprite.set_texture(camTex);
		spriteRenderer.DrawSprite(camSprite);
	}

	spriteRenderer.End();

	renderer3D.set_projection_matrix(proj3DMat);

	renderer3D.set_view_matrix(identityMat);

	// Begin rendering 3D meshes, don't clear the frame buffer
	renderer3D.Begin(false);

	//Setup and draw lines every frame
	CreateLines();

	DrawLines(primitiveRenderer, &renderer3D, primitiveBuilder, platform);

	gef::Material mat = primitiveBuilder->green_material();

	renderer3D.set_override_material(NULL);

	//Update meshes every frame if marker found
	if (&laserBox && laserMarkerFound)
	{
		mat.set_colour(0xffff8080); // sunrise orange
		renderer3D.set_override_material(&mat);

		renderer3D.DrawMesh(*laserBox);

		renderer3D.set_override_material(NULL);

		mat.set_colour(0xff99ebff); // yellow
		renderer3D.set_override_material(&mat);

		for (int i = 0; i < walls.size(); i++)
		{
			if (&walls[i].object)
			{
				renderer3D.DrawMesh(*walls[i].object);
			}
		}

		renderer3D.set_override_material(NULL);

		mat.set_colour(0xff50B400); // green
		renderer3D.set_override_material(&mat);

		if (&target)
		{
			renderer3D.override_material();
			renderer3D.DrawMesh(*target);
		}

		renderer3D.set_override_material(NULL);
	}

	mat.set_colour(0xffffeb99); // blueish
	renderer3D.set_override_material(&mat);

	for (int i = 0; i < reflectors.size(); i++)
	{
		if (&reflectors[i].object && reflectors[i].markerFound)
		{
			renderer3D.DrawMesh(*reflectors[i].object);
		}
	}

	renderer3D.set_override_material(NULL);

	renderer3D.End();

	sampleRenderEnd();
}

void Game::Input(gef::SonyControllerInputManager* controller, GameState& gameState)
{

}

void Game::InitCamera(gef::Platform& platform_)
{
	// initialise sony framework
	sampleInitialize();
	smartInitialize();

	// reset marker tracking
	AppData* dat = sampleUpdateBegin();
	smartTrackingReset();
	sampleUpdateEnd(dat);

	//Camera Setup
	identityMat.SetIdentity();

	orthoMat = platform_.OrthographicFrustum(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	proj3DMat = platform_.PerspectiveProjectionFov(SCE_SMART_IMAGE_FOV, (float)SCE_SMART_IMAGE_WIDTH / (float)SCE_SMART_IMAGE_HEIGHT, 0.1f, 1000.0f);

	scaleMat.SetIdentity();

	scaleMat.set_m(1, 1, CalcImageScaleFactor());

	proj3DMat = scaleMat * proj3DMat;

	camSprite.set_width(2.0f);
	camSprite.set_height(2.0f * CalcImageScaleFactor());
	camSprite.set_position(gef::Vector4(0.0f, 0.0f, 1.0f));
}

float Game::CalcImageScaleFactor()
{
	float scaleFactor;

	float dispAspratio = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
	float camAspRatio = (float)CAMERA_WIDTH / (float)CAMERA_HEIGHT;

	scaleFactor = dispAspratio / camAspRatio;

	return scaleFactor;
}

void Game::InitGameObjects(PrimitiveBuilder* primitiveBuilder, gef::Platform& platform)
{
	// create a new scene object and read in the data from the file
	modelScene = new gef::Scene();

	//gather laser mesh
	modelScene->ReadSceneFromFile(platform, "models/ModelScenes/Reflector.scn");

	// create the materials from the material data present in the scene file
	modelScene->CreateMaterials(platform);

	// if there is mesh data in the scene, create a mesh to draw from the first mesh
	mesh = GetFirstMesh(modelScene, platform);

	//TODO::Change meshes to models created
	laserBox = new game_object();
	laserBox->set_mesh(mesh);

	laserBox->SetPosition(gef::Vector4(0.0f, 0.0f, 0.03f));
	laserBox->SetRotation(gef::Vector4(180.0f, 0.0f, 0.0f));
	laserBox->SetScale(gef::Vector4(1.0f, 1.0f, 1.0f));

	for (int i = 0; i < 2; i++)
	{
		Reflectors* reflector = new Reflectors();
		reflectors.push_back(*reflector);

		reflectors[i].object = new game_object();
		reflectors[i].object->set_mesh(mesh);
		reflectors[i].object->SetPosition(gef::Vector4(0.0f, 0.0f, 0.03f));
		reflectors[i].object->SetRotation(gef::Vector4(180.0f, 0.0f, 90.0f));
		reflectors[i].object->SetScale(gef::Vector4(1.0f, 1.0f, 1.0f));

		reflectors[i].markerID = i + 1;
	}

	srand(static_cast <unsigned> (time(0)));

	//gather model data to use the model as a mesh
	modelScene = new gef::Scene();
	modelScene->ReadSceneFromFile(platform, "models/ModelScenes/Target.scn");
	modelScene->CreateMaterials(platform);
	mesh = GetFirstMesh(modelScene, platform);

	target = new game_object();
	target->set_mesh(mesh);
	target->SetRotation(gef::Vector4(00.0f, 0.0f, 0.0f));
	target->SetScale(gef::Vector4(1.0f, 1.0f, 1.0f));
}

void Game::SetWallPositions(gef::Platform& platform)
{
	if (!walls.empty())
	{
		walls.clear();
	}

	//gather model data to use the model as a mesh
	modelScene = new gef::Scene();
	modelScene->ReadSceneFromFile(platform, "models/ModelScenes/Wall.scn");
	modelScene->CreateMaterials(platform);
	mesh = GetFirstMesh(modelScene, platform);

	int wallNum = RandomInt(6, 3);

	//create random number of walls between 3 and 6
	for (int i = 0; i < wallNum; i++)
	{
		Walls* wall = new Walls();
		walls.push_back(*wall);

		walls[i].object = new game_object();
		walls[i].object->set_mesh(mesh);

		walls[i].object->SetRotation(gef::Vector4(180.0f, 0.0f, 0.0f));
		walls[i].object->SetScale(gef::Vector4(1.0f, 1.0f, 1.0f));
	}

	float minDist = 0.25f;
	float maxDist = 0.3f;

	//set position and rotation of nely created walls
	for (int i = 0; i < wallNum; i++)
	{
		//Set random positions for all walls
		switch (i)
		{
		case 0: //block laser in front
			walls[i].object->SetPosition(gef::Vector4(0.0f, RandomFloat(-minDist, -maxDist), 0.01f));
			walls[i].object->SetRotation(gef::Vector4(0.0f, 0.0f, 0.0f));
			break;
		case 1:
			walls[i].object->SetPosition(gef::Vector4(0.0f, RandomFloat(maxDist, minDist), 0.01f));
			break;
		case 2: //front left of laser box
			walls[i].object->SetPosition(gef::Vector4(RandomFloat(maxDist, minDist), RandomFloat(maxDist, minDist), 0.01f));
			break;
		case 3: //back right of laser box
			walls[i].object->SetPosition(gef::Vector4(RandomFloat(-minDist, -maxDist), RandomFloat(-minDist, -maxDist), 0.01f));
			break;
		case 4: //back left of laser box
			walls[i].object->SetPosition(gef::Vector4(RandomFloat(maxDist, minDist), RandomFloat(-minDist, -maxDist), 0.01f));
			break;
		case 5: //front right
			walls[i].object->SetPosition(gef::Vector4(RandomFloat(-minDist, -maxDist), RandomFloat(maxDist, minDist), 0.01f));
			break;
		case 6:	//random
			walls[i].object->SetPosition(gef::Vector4(RandomFloat(maxDist, -maxDist), RandomFloat(maxDist, -maxDist), 0.01f));
			break;
		}

		if (i != 0)
		{
			walls[i].object->SetRotation(gef::Vector4(0.0f, 0.0f, RandomFloat(180.0f, 0.0f)));
		}
	}

	int randWall = RandomInt(wallNum - 1, 1);

	//set target to be at one of the walls
	target->SetPosition(gef::Vector4(walls[randWall].object->GetPosition().x() - 0.01f, walls[randWall].object->GetPosition().y(), walls[randWall].object->GetPosition().z()));
	target->SetRotation(gef::Vector4(0.0f, 0.0f, walls[randWall].object->GetRotation().z()));
}

void Game::CreateLines()
{
	Lines* line = new Lines();

	SetLineDescription(*line, "laserLine", laserBox->GetTransform(), false, 0.0f, 0.0f, true, laserSize * 1.05f, 0.0f, "green");

	SetLineDescription(*line, "reflectorLine1", reflectors[0].object->GetTransform(), false, 0.0f, 0.0f, true, 0.03f, -0.03f, "green");
	SetLineDescription(*line, "reflectorLine2", reflectors[0].object->GetTransform(), true, 0.03f, -0.03f, false, 0.0f, 0.0f, "green");
	SetLineDescription(*line, "reflectorLine3", reflectors[1].object->GetTransform(), false, 0.0f, 0.0f, true, 0.03f, -0.03f, "green");
	SetLineDescription(*line, "reflectorLine4", reflectors[1].object->GetTransform(), true, 0.03f, -0.03f, false, 0.0f, 0.0f, "green");

	SetLineDescription(*line, "targetLine", target->GetTransform(), true, 0.02f, -0.02f, false, 0.0f, 0.0f, "green");

	if (laserMarkerFound)
	{
		for (int i = 0; i < walls.size(); i++)
		{
			std::stringstream WallLine;
			WallLine << "WallLine" << i + 1;

			SetLineDescription(*line, WallLine.str(), walls[i].object->GetTransform(), true, -0.05f, 0.05f, false, 0.0f, 0.0f, "green");
		}
	}

	if (reflectedLine1 == true)
	{
		if (reflectors[0].markerFound && reflectors[0].collision)
		{
			SetLineDescription(*line, "reflectedLine1", reflectors[0].markerTransform, true, reflectorLaserSize[0] * 1.05f, 0.0f, false, 0.0f, 0.0f, "red");
		}

		reflectedLine1 = false;
	}

	if (reflectedLine2 == true)
	{
		if (reflectors[1].markerFound && reflectors[1].collision)
		{
			SetLineDescription(*line, "reflectedLine2", reflectors[1].markerTransform, true, reflectorLaserSize[1] * 1.05f, 0.0f, false, 0.0f, 0.0f, "red");
		}

		reflectedLine2 = false;
	}

	delete line;
}

void Game::SetLineDescription(Lines line, std::string name, const gef::Matrix44 & transform, bool xAxis, const float xSize, float xAxisOffset, bool yAxis, const float ySize, float yAxisOffset, std::string laserColour)
{
	line.name = name;
	line.trans = transform;
	line.xAxis = xAxis;
	line.xSize = xSize;
	line.xOff = xAxisOffset;
	line.yAxis = yAxis;
	line.ySize = ySize;
	line.yOff = yAxisOffset;
	line.laserColour = laserColour;

	lineVect.push_back(line);
}

//sends line vector full of the line data to be rendered and then calls collision check and then clears the line vector to be updated again
void Game::DrawLines(PrimitiveRenderer* primitiveRenderer, gef::Renderer3D* renderer3D, PrimitiveBuilder* primitiveBuilder, gef::Platform& platform)
{
	primitiveRenderer->Reset();

	for (int i = 0; i < lineVect.size(); i++)
	{
		DrawAxes(primitiveRenderer, lineVect[i].trans, lineVect[i].xAxis, lineVect[i].xSize, lineVect[i].xOff, lineVect[i].yAxis, lineVect[i].ySize, lineVect[i].yOff, lineVect[i].laserColour);
		lineVect[i].Start = lineStart;
		lineVect[i].End = lineEnd;
	}

	if (&laserBox && laserMarkerFound)
	{
		LaserCollisionCheck(primitiveBuilder, platform);
	}

	lineVect.clear();

	primitiveRenderer->Render(*renderer3D);
}

//adds lines to be drawn by the renderer
void Game::DrawAxes(PrimitiveRenderer * primitive_renderer, const gef::Matrix44 & transform, bool xAxis, const float xSize, float xAxisOffset, bool yAxis, const float ySize, float yAxisOffset, std::string laserColour)
{
	gef::Vector4 origin(xAxisOffset, yAxisOffset, 0.0f);
	origin = origin.Transform(transform);

	lineStart = gef::Vector2(origin.x(), origin.y());

	if (xAxis)
	{
		gef::Vector4 x_axis(1.0f, 0.0f, 0.0f);

		x_axis = x_axis * xSize;

		x_axis = x_axis.Transform(transform);

		if (laserColour == "green")
		{
			primitive_renderer->AddLine(gef::Vector4(origin.x(), origin.y(), origin.z()), x_axis, gef::Colour(0.0f, 1.0f, 0.0f));
		}
		else
		{
			primitive_renderer->AddLine(gef::Vector4(origin.x(), origin.y(), origin.z()), x_axis, gef::Colour(1.0f, 0.0f, .0f));
		}

		lineEnd = gef::Vector2(x_axis.x(), x_axis.y());
	}

	if (yAxis)
	{
		gef::Vector4 y_axis(0.0f, 1.0f, 0.0f);

		y_axis = y_axis * ySize;

		y_axis = y_axis.Transform(transform);

		if (laserColour == "green")
		{
			primitive_renderer->AddLine(gef::Vector4(origin.x(), origin.y(), origin.z()), y_axis, gef::Colour(0.0f, 1.0f, 0.0f));
		}
		else
		{
			primitive_renderer->AddLine(gef::Vector4(origin.x(), origin.y(), origin.z()), y_axis, gef::Colour(1.0f, 0.0f, 0.0f));
		}

		lineEnd = gef::Vector2(y_axis.x(), y_axis.y());
	}
}

void Game::LaserCollisionCheck(PrimitiveBuilder* primitiveBuilder, gef::Platform& platform)
{
	//0 - laser // 1 - reflector1 // 2 - reflector1 // 3 - reflector2 // 4 - reflector2 // 5 - target //6-11 walls //11+ reflected lines
	for (int i = 0; i < lineVect.size(); i++)
	{
		//Original laser checks
		if (lineVect[i].name == "laserLine")
		{
			for (int k = 0; k < walls.size(); k++)
			{
				if (IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[k + 6].Start, lineVect[k + 6].End))
				{
					laserSize = Distance(laserBoxMarkerTransform.GetTranslation().x(), laserBoxMarkerTransform.GetTranslation().y(), walls[k].object->GetTransform().GetTranslation().x(), walls[k].object->GetTransform().GetTranslation().y());
				}
			}

			for (int j = 0; j < reflectors.size(); j++)
			{
				//set opposing reflector
				int otherLine[2];
				int otherRef;

				if (j == 0) // first reflector lines
				{
					otherLine[0] = 1;
					otherLine[1] = 2;
					otherRef = 1;
				}
				else
				{
					otherLine[0] = 3;
					otherLine[1] = 4;
					otherRef = 0;
				}

				if (IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[otherLine[0]].Start, lineVect[otherLine[0]].End) || IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[otherLine[1]].Start, lineVect[otherLine[1]].End)) //reflectors 1- 4, first two are reflector 1 - so j + 1 & j + 2 = reflector1 and 1+=1 = 2 
				{
					laserSize = Distance(laserBoxMarkerTransform.GetTranslation().x(), laserBoxMarkerTransform.GetTranslation().y(), reflectors[j].object->GetTransform().GetTranslation().x(), reflectors[j].object->GetTransform().GetTranslation().y());

					if (j == 0)
					{
						reflectedLine1 = true;
					}
					else if (j == 1)
					{
						reflectedLine2 = true;
					}

					reflectors[j].collision = true;
				}
			}
		}
		
		////check refelcted lines against collision with other objects
		for (int j = 0; j < 2; j++)
		{
			std::stringstream ReflectingLine;
			ReflectingLine << "reflectedLine" << j + 1;

			if (lineVect[i].name == ReflectingLine.str())
			{
				//set opposing reflector
				int otherLine[2];
				int otherRef;

				if (j == 0) // first reflector lines
				{
					otherLine[0] = 3;
					otherLine[1] = 4;
					otherRef = 1;
				}
				else
				{
					otherLine[0] = 1;
					otherLine[1] = 2;
					otherRef = 0;
				}

				//colliding with other reflector
				if (IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[otherLine[0]].Start, lineVect[otherLine[0]].End) || IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[otherLine[1]].Start, lineVect[otherLine[1]].End))
				{
					reflectorLaserSize[j] = Distance(reflectors[j].object->GetTransform().GetTranslation().x(), reflectors[j].object->GetTransform().GetTranslation().y(), reflectors[otherRef].object->GetTransform().GetTranslation().x(), reflectors[otherRef].object->GetTransform().GetTranslation().y());

					if (j == 0)
					{
						reflectedLine2 = true;
					}
					else
					{
						reflectedLine1 = true;
					}

					reflectors[otherRef].collision = true;
				}

				//colliding with a wall
				for (int k = 0; k < walls.size(); k++)
				{
					if (IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[k + 6].Start, lineVect[k + 6].End))
					{
						reflectorLaserSize[j] = Distance(reflectors[j].object->GetTransform().GetTranslation().x(), reflectors[j].object->GetTransform().GetTranslation().y(), walls[k].object->GetTransform().GetTranslation().x(), walls[k].object->GetTransform().GetTranslation().y());
					}
				}

				//colliding with target
				if (IsLineIntersecting(lineVect[i].Start, lineVect[i].End, lineVect[5].Start, lineVect[5].End)) //target is always 5th
				{
					//Reset scene every target hit.
					//Give points and reset timer
					if (timer >= 0.15f)
					{
						SetWallPositions(platform);
						points += 50;

						timer = 0;
					}
					else
					{
						timer += deltaTime;
					}
				}
			}
		}
	}
}

//2d line collision
bool Game::IsLineIntersecting(gef::Vector2 lineAStart, gef::Vector2 lineAEnd, gef::Vector2 lineBStart, gef::Vector2 lineBEnd)
{
	//B1 * A2 - A1 * B2
	float denominator = ((lineAEnd.x - lineAStart.x) * (lineBStart.y - lineBEnd.y)) - ((lineAEnd.y - lineAStart.y) * (lineBStart.x - lineBEnd.x));

	float numerator1 = ((lineAStart.y - lineBEnd.y) * (lineBStart.x - lineBEnd.x)) - ((lineAStart.x - lineBEnd.x) * (lineBStart.y - lineBEnd.y));
	float numerator2 = ((lineAStart.y - lineBEnd.y) * (lineAEnd.x - lineAStart.x)) - ((lineAStart.x - lineBEnd.x) * (lineAEnd.y - lineAStart.y));

	if (denominator == 0)
	{
		return numerator1 == 0 && numerator2 == 0;
	}

	float r = numerator1 / denominator;
	float s = numerator2 / denominator;

	return (r >= 0 && r <= 1) && (s >= 0 && s <= 1);
}

void Game::ResetGame(GameState* gameState)
{
	firstMarkerCheck = false;
	secondMarkerCheck = false;
	thirdMarkerCheck = false;

	points = 0;
	timer = 0;

	if (gameState->GetGameDifficulty() == EASY)
	{
		countdown = 60.0f;
	}
	else if (gameState->GetGameDifficulty() == MEDIUM)
	{
		countdown = 30.0f;

	}
	else if (gameState->GetGameDifficulty() == HARD)
	{
		countdown = 20.0f;
	}
	
	laserSize = 0.5f;

	for (int i = 0; i < 2; i++)
	{
		reflectorLaserSize[i] = 0.5f;
	}

	lineVect.clear();
}

// Function to calculate distance 
float Game::Distance(float x1, float y1, float x2, float y2)
{
	// Calculating distance 
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

//This works by returning 'a' plus something, where something is between 0 and 'b-a' which makes the end result lie in between 'a and b'.
//want floats instead of ints which rand returns
float Game::RandomFloat(float max, float min)
{
	float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

int Game::RandomInt(int max, int min)
{
	if (max < min)
	{
		max = min;
	}

	int r = rand() % (max - min + 1) + min;
}

//show text on screen
void Game::DrawHUD(gef::Font* font, gef::SpriteRenderer* spriteRenderer)
{
	if (firstMarkerCheck && secondMarkerCheck && thirdMarkerCheck)
	{
		//cutndownd
		font->RenderText(spriteRenderer, gef::Vector4(750.0f, 20.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Time: %.1f", countdown);

		// display points
		font->RenderText(spriteRenderer, gef::Vector4(50.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Points: %i", points);

		//Text to tell player to move markers into frame
		if (laserMarkerFound == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.1, (float)DISPLAY_HEIGHT * 0.1, -0.9f), 1.2f, 0xffffffff, gef::TJ_LEFT, "Countdown paused. Move marker 1 into frame to resume game");
		}

		if (reflectors[0].markerFound == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.1, (float)DISPLAY_HEIGHT * 0.15, -0.9f), 1.2f, 0xffffffff, gef::TJ_LEFT, "Move marker 2 into frame");
		}

		if (reflectors[1].markerFound == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.1, (float)DISPLAY_HEIGHT * 0.2, -0.9f), 1.2f, 0xffffffff, gef::TJ_LEFT, "Move marker 3 into frame");
		}
	}
	else
	{
		if (firstMarkerCheck == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3, (float)DISPLAY_HEIGHT * 0.5, -0.9f), 1.5f, 0xffffffff, gef::TJ_LEFT, "Move marker 1 into frame");
		}
		else if (secondMarkerCheck == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3, (float)DISPLAY_HEIGHT * 0.5, -0.9f), 1.5f, 0xffffffff, gef::TJ_LEFT, "Move marker 2 into frame");
		}
		else if (thirdMarkerCheck == false)
		{
			font->RenderText(spriteRenderer, gef::Vector4((float)DISPLAY_WIDTH * 0.3, (float)DISPLAY_HEIGHT * 0.5, -0.9f), 1.5f, 0xffffffff, gef::TJ_LEFT, "Move marker 3 into frame");
		}
	}
}

//gather mesh from scene
gef::Mesh * Game::GetFirstMesh(gef::Scene* scene, gef::Platform& platform)
{
	gef::Mesh* mesh = NULL;

	if (scene)
	{
		// now check to see if there is any mesh data in the file, if so lets create a mesh from it
		if (scene->mesh_data.size() > 0)
			mesh = scene->CreateMesh(platform, scene->mesh_data.front());
	}

	return mesh;
}