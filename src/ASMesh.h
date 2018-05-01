#include <string>
#include <fstream>
using namespace std;

//Extract mesh from *.aso files and sort informations
class ASMesh
{
public:
	ASMesh ();
	~ASMesh();

	static bool LoadFromFile		(ASMesh* mesh, const char* filename);

	bool		SetMeshVertexTab	(void* a_VertexData, unsigned int a_VertexSize, unsigned int a_VertexCount, unsigned int a_VertexType);
	bool		SetMeshIndexTab		(void* a_IndexData, unsigned int a_IndexSize, unsigned int a_IndexCount);

	void*		GetMeshVertexTab	();	
	void*		GetMeshIndexTab		();

	unsigned int	GetVertexSize	();
	unsigned int	GetIndexSize	();

	unsigned int	GetVertexCount	();
	unsigned int	GetIndexCount	();

	unsigned int	GetVertexType	();

	void			Clear();

private:
	
	void*	_pVertexTab;
	void*	_pIndexTab;

	unsigned int	_uiIndexCount;
	unsigned int	_uiIndexSize;
	unsigned int	_uiVertexCount;
	unsigned int	_uiVertexSize;
	unsigned int	_flagVertexType;
};

