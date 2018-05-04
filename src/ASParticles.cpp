#include "ASParticles.h"

void ASParticlesInstances::InitBuffers(const char * techniqueName, UINT nbr)
{
	Init("RenderParticles", m_meshPath.c_str(), m_maxParticles + 1, &m_updateRenderTechnique);
}
