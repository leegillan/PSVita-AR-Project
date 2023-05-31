#pragma once

#include <system/application.h>
#include <maths/vector2.h>
#include <maths/matrix44.h>
#include <string>

struct Lines
{
	std::string name;
	gef::Matrix44 trans;
	bool xAxis;
	float xSize;
	float xOff;
	bool yAxis;
	float ySize;
	float yOff;
	std::string laserColour;

	gef::Vector2 Start;
	gef::Vector2 End;
};


// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
}

class Line
{
public:
	Line();
	~Line();
};

