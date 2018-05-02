#pragma once

#include "ASParticles.h"
#include "ASFields.h"
#include "ASScreen.h"

class ASScene
{
	static ASScene *m_singleton;
	ASScreen* m_screen;
	ASFields *m_fields;
	ASParticles* m_particles;

	ASScene();

public:
	static ASScene * GetInstance();
	~ASScene();

	void Init(vector<std::string> buf);
	void Render();
	void Clear();
};

ASScene* ASScene::m_singleton = NULL;
