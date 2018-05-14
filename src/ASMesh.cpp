#include "ASMesh.h"
typedef struct 
{
	float x,y,z;
}VERTEX_POS;

typedef struct 
{
	float x,y,z;
	float nx,ny,nz;
}VERTEX_POSNORM;

typedef struct 
{
	float x,y,z;
	float u,v;
}VERTEX_POSUV;

typedef struct 
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
}VERTEX_POSNORMUV;


ASMesh::	ASMesh ()
{
	_pVertexTab = NULL;
	_pIndexTab	= NULL;
}


ASMesh::	~ASMesh ()
{
	Clear();
}

bool ASMesh::LoadFromFile		(ASMesh* mesh, const char* filename)
{
	ifstream fi = ifstream(filename);
	bool exists = bool(fi);
	fi.close();
	assert(exists&&"Mesh path does not exist");
	FILE* fp;
	char buffer [256];
	unsigned int vType = 0;
	unsigned int vCount = 0;
	unsigned int tCount = 0;
	unsigned int vSize = 0;
	unsigned int vCurrent = 0;
	unsigned int tCurrent = 0;
	
	void* vData = NULL;
	unsigned int* tData = {NULL};
	
	unsigned int vSizeArray [4] = {3, 6, 5, 8};
	
	fopen_s (&fp, filename, "r");
	if (fp == NULL)
		return false;

	while (fgets (buffer, 256, fp) != NULL)
	{
		if (strncmp (buffer, "vtype", 5) == 0)
		{
			sscanf_s (buffer + 5, "%d", &vType);
			vSize = vSizeArray [vType];
		}
		else if (strncmp (buffer, "vertices", 8) == 0)
		{
			sscanf_s (buffer + 9, "%d", &vCount);
			vData = new float[vCount*vSize];
		}
		else if (strncmp (buffer, "triangles", 9) == 0)
		{
			sscanf_s (buffer + 10, "%d", &tCount);
			tData = new unsigned int [tCount];
		}
		else if (buffer [0] == 'v')
		{
			switch (vType)
			{
				case 0:
				{
					VERTEX_POS temp;
					sscanf_s (buffer + 2, "%f %f %f", &temp.x, &temp.y, &temp.z);
					memcpy (&((VERTEX_POS*)vData)[vCurrent], &temp, sizeof (VERTEX_POS));
					vCurrent++;
					break;
				}
				case 1:
				{
					VERTEX_POSNORM temp;
					sscanf_s (buffer + 2, "%f %f %f, %f %f %f", &temp.x, &temp.y, &temp.z, &temp.nx, &temp.ny, &temp.nz);
					memcpy (&((VERTEX_POSNORM*)vData)[vCurrent], &temp, sizeof (VERTEX_POSNORM));
					vCurrent++;
					break;
				}
				case 2:
				{
					VERTEX_POSUV temp;
					sscanf_s (buffer + 2, "%f %f %f, %f %f", &temp.x, &temp.y, &temp.z, &temp.u, &temp.v);
					memcpy (&((VERTEX_POSUV*)vData)[vCurrent], &temp, sizeof (VERTEX_POSUV));
					vCurrent++;
					break;
				}
				case 3:
				{
					VERTEX_POSNORMUV temp;
					sscanf_s (buffer + 2, "%f %f %f, %f %f %f, %f %f", &temp.x, &temp.y, &temp.z, &temp.nx, &temp.ny, &temp.nz, &temp.u, &temp.v);
					memcpy (&((VERTEX_POSNORMUV*)vData)[vCurrent], &temp, sizeof (VERTEX_POSNORMUV));
					vCurrent++;
					break;
				}
			}
			
		}
		else if (buffer [0] == 't')
		{
			sscanf_s (buffer + 2, "%d %d %d", &tData [tCurrent], &tData [tCurrent+1], &tData [tCurrent+2]);
			tCurrent += 3;
		}
		
	}
	
	

	fclose (fp);
	if (! mesh->SetMeshVertexTab	(vData, vSize*sizeof(float),vCount, vType))
		return false;
	if (! mesh->SetMeshIndexTab	(tData, sizeof (unsigned int), tCount))
		return false;
	return true;
}


bool		ASMesh::SetMeshVertexTab	(void* a_VertexData, unsigned int a_VertexSize, unsigned int a_VertexCount, unsigned int a_VertexType)
{
	if (_pVertexTab != NULL)
		return false;
	_pVertexTab = a_VertexData;
	_uiVertexCount = a_VertexCount;
	_uiVertexSize = a_VertexSize;
	_flagVertexType = a_VertexType;

	return true;
}


bool		ASMesh::SetMeshIndexTab		(void* a_IndexData, unsigned int a_IndexSize, unsigned int a_IndexCount)
{
	if (_pIndexTab != NULL)
		return false;
	_pIndexTab = a_IndexData;
	_uiIndexCount = a_IndexCount;
	_uiIndexSize = a_IndexSize;

	return true;
}


void*		ASMesh::GetMeshVertexTab	()
{
	return _pVertexTab;
}


void*		ASMesh::GetMeshIndexTab		()
{
	return	_pIndexTab;
}


unsigned int	ASMesh::GetVertexSize	()
{
	return _uiVertexSize;
}


unsigned int	ASMesh::GetIndexSize	()
{
	return _uiIndexSize;
}


unsigned int	ASMesh::GetVertexCount	()
{
	return _uiVertexCount;
}


unsigned int	ASMesh::GetIndexCount	()
{
	return _uiIndexCount;
}


unsigned int	ASMesh::GetVertexType	()
{
	return _flagVertexType;
}



void			ASMesh::Clear()
{
	delete [] _pVertexTab;
	delete [] _pIndexTab;
}
