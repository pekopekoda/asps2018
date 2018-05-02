#include "ASScene.h"

ASScene* ASScene::GetInstance()
{
	if (m_singleton == NULL)
		m_singleton = new ASScene();

	return m_singleton;
}

ASScene::ASScene(): m_screen(new ASScreen()), m_particles(new ASParticles()), m_fields(new ASFields())
{
}

ASScene::~ASScene()
{
}

void ASScene::Init(vector<std::string> buf)
{
	m_screen->InitShaderResources(buf);
	m_fields->InitShaderResources(buf);
	m_particles->InitShaderResources(buf);
	
	m_screen->InitViews();
	m_fields->InitViews();
	m_particles->InitViews();
	m_screen->AddEffectResourceVariable(m_fields->GetMainRenderResource(), 2);
	m_screen->AddEffectResourceVariable(m_particles->GetMainRenderResource(), 2);
	m_screen->AddEffectResourceVariable(m_particles->GetInstanceRenderResource(), 2);
	
	m_screen->InitBuffers();
	m_fields->InitBuffers();
	m_particles->InitBuffers();
}

void ASScene::Render()
{
	m_fields->Render();
	m_particles->Render();
	m_screen->Render();
}

void ASScene::Clear()
{
	m_screen->Clear();
	m_fields->Clear();
	m_particles->Clear();
}
