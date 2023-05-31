#include "game_object.h"
#include <math.h>
#include <maths/math_utils.h>

game_object::game_object()
{
	Init();
}

game_object::~game_object()
{
}

void game_object::Init()
{
	position = gef::Vector4(0.0f, 0.0f, 0.0f);
	velocity = gef::Vector4(0.0f, 0.0f, 0.0f);
	scale = gef::Vector4(1.0f, 1.0f, 1.0f);
	rotation = gef::Vector4(0.0f, 0.0f, 0.0f);

	localTransform.SetIdentity();

	isStatic = false;
}

bool game_object::Update(float frame_time)
{
	if (!isStatic)
	{
		position += velocity * frame_time;
	}

	BuildTransform();

	return true;
}

bool game_object::Update(float frame_time, gef::Matrix44 markerTransform)
{
	if (!isStatic)
	{
		position += velocity * frame_time;
	}

	BuildTransform(markerTransform);

	return true;
}

void game_object::BuildTransform()
{
	gef::Matrix44 scaleMat;
	gef::Matrix44 rotMat;
	gef::Matrix44 transform;

	scaleMat.SetIdentity();
	rotMat.SetIdentity();
	transform.SetIdentity();

	scaleMat.Scale(scale);
	rotMat.RotationX(gef::DegToRad(rotation.x()));
	rotMat.RotationY(rotation.y());
	rotMat.RotationZ(rotation.z());
	transform.SetTranslation(position);
	
	set_transform(scaleMat * rotMat * transform);
}

void game_object::BuildTransform(gef::Matrix44 markerTransform)
{
	gef::Matrix44 scaleMat;
	gef::Matrix44 rotX;
	gef::Matrix44 rotY;
	gef::Matrix44 rotZ;
	gef::Matrix44 transform;

	scaleMat.SetIdentity();
	rotX.SetIdentity();
	rotY.SetIdentity();
	rotZ.SetIdentity();
	transform.SetIdentity();

	scaleMat.Scale(scale);
	rotX.RotationX(gef::DegToRad(rotation.x()));
	rotY.RotationY(gef::DegToRad(rotation.y()));
	rotZ.RotationZ(gef::DegToRad(rotation.z()));
	transform.SetTranslation(position);

	localTransform = scaleMat * (rotX * rotY * rotZ) * transform;

	localTransform = localTransform * markerTransform;

	set_transform(localTransform);
}

