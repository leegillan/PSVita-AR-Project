#pragma once

#include <graphics/mesh_instance.h>
#include <math.h>

class game_object : public gef::MeshInstance
{

public:
	game_object();
	~game_object();

	void Init();
	bool Update(float frame_time);
	bool Update(float frame_time, gef::Matrix44 markerTransform);

	void BuildTransform();
	void BuildTransform(gef::Matrix44 markerTransform);

	//getters
	gef::Vector4 GetVelocity() { return velocity; }
	gef::Vector4 GetRotation() { return rotation; }
	gef::Matrix44 GetTransform() { return localTransform; }
	gef::Vector4 GetPosition() { return position; }

	//Setters
	void SetPosition(gef::Vector4 p) { position = p; }
	void SetVelocity(gef::Vector4 v) { velocity = v; }
	void SetScale(gef::Vector4 sc) { scale = sc; }
	void SetRotation(gef::Vector4 ro) { rotation = ro; }
	void IsStatic(bool iS) { isStatic = iS; }

private:
	gef::Vector4 position;
	gef::Vector4 velocity;
	gef::Vector4 scale;
	gef::Vector4 rotation;

	gef::Matrix44 localTransform;

	bool isStatic;
};

