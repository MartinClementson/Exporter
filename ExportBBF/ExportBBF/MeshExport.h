#ifndef MESH_EXPORT_H
#define MESH_EXPORT_H
#include "HeaderStructs.h"
#include "maya_includes.h"
#include <QtWidgets\qprogressbar.h>

using namespace std;
class MeshExport
{
private:
	vector<Vertex> * vertices = nullptr;
	vector<unsigned int> * newIndex = nullptr;
	vector<SkinData> * skinList = nullptr;

	unsigned int jointCount;

	fstream * outFile;
public:
	MeshExport();
	MeshExport(fstream * outFile);
	MeshExport(fstream * outFile, vector<SkinData> * skinList, unsigned int jointCount);
	MeshExport(string &filePath);
	~MeshExport();
	void exportMesh(MObject & mNode);
	static int getProgressBarValue();
private:
	/*export a mesh with a skeleton*/
	void exportDynamic(MFnMesh & mMesh, MFnTransform & mTran);

	/*ecport a mesh without any skeleton*/
	void exportStatic(MFnMesh & mMesh, MFnTransform & mTran);
};
#endif 

