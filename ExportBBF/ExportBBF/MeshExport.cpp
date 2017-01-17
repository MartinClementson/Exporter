#include "MeshExport.h"



MeshExport::MeshExport()
{
}



bool bbfExists(const std::string& filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}

MeshExport::MeshExport(string & filePath, vector<SkinData>* skinList)
{
	this->skinList = skinList;
	this->filePath = filePath;

	/*if (bbfExists(filePath))
	{
		size_t f = filePath.rfind("/", filePath.length());
		string pAth = filePath.substr(f + 1, filePath.length());

		string meshName = ("Overwrite " + pAth + "?");
		std::wstring stemp = std::wstring(meshName.begin(), meshName.end());
		LPCWSTR sw = stemp.c_str();
		MainHeader s_Head;
		s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
		m_UID = s_Head.id;
		if (MessageBox(NULL, sw, TEXT("bbf file already exists"), MB_YESNO) == IDYES)
		{
			outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

			s_Head.type = (int)Resources::ResourceType::RES_MESH;

			outFile->write((char*)&s_Head, sizeof(MainHeader));
		}
		else
			overWrite = false;
	}
	else
	{
		outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

		MainHeader s_Head;
		s_Head.type = (int)Resources::ResourceType::RES_MESH;
		s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
		m_UID = s_Head.id;

		outFile->write((char*)&s_Head, sizeof(MainHeader));
	}*/
}

MeshExport::MeshExport(string & filePath)
{
	this->filePath = filePath;
	/*if (bbfExists(filePath))
	{
		size_t f = filePath.rfind("/", filePath.length());
		string pAth = filePath.substr(f + 1, filePath.length());

		string meshName = ("Overwrite " + pAth + "?");
		std::wstring stemp = std::wstring(meshName.begin(), meshName.end());
		LPCWSTR sw = stemp.c_str();
		MainHeader s_Head;
		s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
		m_UID = s_Head.id;
		if (MessageBox(NULL, sw, TEXT("bbf file already exists"), MB_YESNO) == IDYES)
		{
			outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

			s_Head.type = (int)Resources::ResourceType::RES_MESH;

			outFile->write((char*)&s_Head, sizeof(MainHeader));
		}
		else
			overWrite = false;
	}
	else
	{
		outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

		MainHeader s_Head;
		s_Head.type = (int)Resources::ResourceType::RES_MESH;
		s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
		m_UID = s_Head.id;

		outFile->write((char*)&s_Head, sizeof(MainHeader));
	}*/
}

MeshExport::~MeshExport()
{
	delete outFile; //may crash here <---------------------------------------------------
	//vertices->clear();
	//newIndex->clear();
	//delete vertices;
	//delete newIndex;
}

void MeshExport::exportMesh(MObject & mNode,bool customObb)
{

	MFnMesh mMesh1(MFnTransform(mNode).child(0), NULL);
	MFnTransform mTran1 = mNode;
	
	string attrName = mTran1.name().asChar();
	if (attrName == "BBOX")
	{
		
		return;
	}

	if (attrName != "BBOX")
	{
		if (bbfExists(filePath))
		{
			size_t f = filePath.rfind("/", filePath.length());
			string pAth = filePath.substr(f + 1, filePath.length());

			string meshName = ("Overwrite " + pAth + "?");
			std::wstring stemp = std::wstring(meshName.begin(), meshName.end());
			LPCWSTR sw = stemp.c_str();
			MainHeader s_Head;
			s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
			m_UID = s_Head.id;
			if (MessageBox(NULL, sw, TEXT("bbf file already exists"), MB_YESNO) == IDYES)
			{
				outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

				s_Head.type = (int)Resources::ResourceType::RES_MESH;

				outFile->write((char*)&s_Head, sizeof(MainHeader));
			}
			else
				overWrite = false;
		}
		else
		{
			outFile = new fstream(filePath, std::fstream::out | std::fstream::binary);

			MainHeader s_Head;
			s_Head.type = (int)Resources::ResourceType::RES_MESH;
			s_Head.id = (unsigned int)std::hash<std::string>{}(filePath);
			m_UID = s_Head.id;

			outFile->write((char*)&s_Head, sizeof(MainHeader));
			overWrite = true;
		}
		if (overWrite)
		{
			/*extracting the nodes from the MObject*/
			MFnMesh mMesh(MFnTransform(mNode).child(0), NULL);
			MFnTransform mTran = mNode;

			/*forcing the quadsplit to be right handed*/
			MString quadSplit = "setAttr """;
			quadSplit += mMesh.name();
			quadSplit += ".quadSplit"" 0;";
			MGlobal::executeCommandStringResult(quadSplit);

			/*checking if the mesh has a skeleton*/
			if (skinList != nullptr)
			{
				MStatus res;
				MFnDependencyNode skinDepNode = mMesh.object();
				MPlug skinCluster = skinDepNode.findPlug("inMesh", &res);
				MPlugArray skinClusterConnection;
				skinCluster.connectedTo(skinClusterConnection, true, false, &res);
				MFnSkinCluster skinClusterObject(skinClusterConnection[0].node(), &res);

				/*Checking to see if the mesh has a skeleton*/
				if (res)
				{
					//newBox.exportBoundingBox(mNode);
					exportDynamic(mMesh, mTran);
				}
			}
			else
			{

				if (!customObb)
				{
					newBox.exportBoundingBox(mNode);
				}
				exportStatic(mMesh, mTran, customObb);
			}
		}
	}
}

int MeshExport::getProgressBarValue()
{
	MIntArray offset, indexList;
	MItDag meshIt(MItDag::kBreadthFirst, MFn::kMesh, NULL);
	for (; !meshIt.isDone(); meshIt.next())
	{
		MFnMesh(meshIt.currentItem()).getTriangles(offset, indexList);
	}
	return indexList.length();
}

void MeshExport::GenerateID(std::string *filePath)
{
	if(filePath != nullptr)
		this->m_UID = (unsigned int)std::hash<std::string>{}(*filePath);
	else
		this->m_UID = (unsigned int)std::hash<std::string>{}(this->filePath);
}


void MeshExport::exportCustomObb(MStatus &res)
{


	MItDag meshIt(MItDag::kBreadthFirst, MFn::kTransform, &res);
	for (; !meshIt.isDone(); meshIt.next())
	{
		MFnTransform trans = meshIt.currentItem();
		if (trans.child(0).hasFn(MFn::kMesh))
		{
			string attrName = trans.name().asChar();

			if (attrName[0] == 'B')
				if (attrName[1] == 'B')
					if (attrName[2] == 'O')
						if (attrName[3] == 'X')
						{
							MFnMesh mMesh(trans.child(0), NULL);
							//ta med rotation
							// translation
							//skala
							//AAOOB
							MIntArray indexList, offsetList;

							const float* position = mMesh.getRawPoints(NULL);
							mMesh.getTriangles(offsetList, indexList);

							MDGModifier Modifier;

							MVector center;
							Vector3 m_rotation = { 0,0,0 };
							Vector3 m_position = { 0,0,0 };
							Vector3 min = { 0,0,0 };
							Vector3 max = { 0,0,0 };

							for (unsigned int i = 0; i < indexList.length(); i++)
							{
								m_position.x = position[indexList[i] * 3];
								m_position.y = position[indexList[i] * 3 + 1];
								m_position.z = position[indexList[i] * 3 + 2];

								if (min.x > m_position.x)
									min.x = m_position.x;
								if (min.y > m_position.y)
									min.y = m_position.y;
								if (min.z > m_position.z)
									min.z = m_position.z;

								if (max.x < m_position.x)
									max.x = m_position.x;
								if (max.y < m_position.y)
									max.y = m_position.y;
								if (max.z < m_position.z)
									max.z = m_position.z;
							}
							MVector pivPos = trans.rotatePivot(MSpace::kObject, NULL);

							obbHead.position = pivPos;
							center.x = max.x - min.x;
							center.y = max.y - min.y;
							center.z = max.z - min.z;

							obbHead.extension[0] = max.x;
							obbHead.extension[1] = max.y;
							obbHead.extension[2] = max.z;

							Vector3* temp = nullptr;

							double x, y, z, w;
							double rotation[3];
							//trans.getRotation()
						//	trans.getRotationQuaternion(x, y, z, w,MSpace::kObject);
							trans.getRotation(rotation,MTransformationMatrix::RotationOrder::kXYZ,MSpace::kTransform);

							//obbHead.extensionDir[0].x

							MFloatMatrix IdMatrix;
							IdMatrix.setToIdentity();



							double testMatrix[4][4] =
							{ x,0,0,0,
								0,y,0,0,
								0,0,z,0,
								0,0,0,w };



							MFloatMatrix m_Matrix(testMatrix);

							MFloatMatrix resultMatrix = IdMatrix * m_Matrix;

							obbHead.extensionDir[0].x = resultMatrix.matrix[0][0];
							obbHead.extensionDir[0].y = 0;
							obbHead.extensionDir[0].z = 0;

							obbHead.extensionDir[1].x = 0;
							obbHead.extensionDir[1].y = resultMatrix.matrix[1][1];
							obbHead.extensionDir[1].z = 0;

							obbHead.extensionDir[2].x = 0;
							obbHead.extensionDir[2].y = 0;
							obbHead.extensionDir[2].z = resultMatrix.matrix[2][2];

							DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixIdentity();

							for (int i = 0; i < 3; i++)
							{
								temp = &obbHead.extensionDir[i];
								RotationMatrix.r[i] = DirectX::XMVectorSet(temp->x, temp->y, temp->z, 0);
								temp = nullptr;
							}



							//	Modifier.deleteNode(meshIt.currentItem());
							//	Modifier.doIt();
							break;
						}
						else
							return;
		}
	}

}

void MeshExport::exportDynamic(MFnMesh & mMesh, MFnTransform & mTran)
{
	MStatus res;

	MItDependencyNodes layerWeightIter(MFn::kAnimLayer, &res);
	if (res == MStatus::kSuccess)
	{
		while (!layerWeightIter.isDone())
		{
			MFnDependencyNode animLayerFn(layerWeightIter.item(), &res);

			MPlug soloPlug = animLayerFn.findPlug("solo", &res);
			MPlug mutePlug = animLayerFn.findPlug("mute", &res);

			mutePlug.setBool(true);

			layerWeightIter.next();
		}
	}

	/*Declaring variables to be used*/
	MIntArray indexList, offsetIdList, normalCount, uvCount, uvIds, normalIdList;
	MFloatPointArray points;
	MFloatArray u, v;
	MFloatVectorArray tangents;
	QWidget *control = MQtUtil::findControl("progressBar");
	QProgressBar *pBar = (QProgressBar*)control;

	/*getting the index list of the vertex positions*/
	mMesh.getTriangles(offsetIdList, indexList);
	mMesh.getTriangleOffsets(uvCount, offsetIdList);
	mMesh.getAssignedUVs(uvCount, uvIds);
	mMesh.getNormalIds(normalCount, normalIdList);
	mMesh.getTangents(tangents, MSpace::kObject);

	/*getting the data for the vertices*/
	const float * postitions = mMesh.getRawPoints(NULL);
	const float * normalsPos = mMesh.getRawNormals(NULL);
	mMesh.getUVs(u, v);

	/*extracting all the information from maya and putting them in a array of vertices*/
	vector<unsigned int> * newIndex = new vector<unsigned int>[indexList.length()];
	vector<SkelVertex> * sVertices = new vector<SkelVertex>[indexList.length()];
	SkelVertex tempVertex;
	MeshHeader hHead;
	BoundingBoxHeader obbHead;
	hHead.hasSkeleton = true;

	//Recalculating the vertices using only the unique vertices based on individual normals
	for (unsigned int i = 0; i < indexList.length(); ++i)
	{
		tempVertex.position.x = postitions[indexList[i] * 3];
		tempVertex.position.y = postitions[indexList[i] * 3 + 1];
		tempVertex.position.z = (postitions[indexList[i] * 3 + 2] * -1);

		tempVertex.normal.x = normalsPos[normalIdList[offsetIdList[i]] * 3];
		tempVertex.normal.y = normalsPos[normalIdList[offsetIdList[i]] * 3 + 1];
		tempVertex.normal.z = (normalsPos[normalIdList[offsetIdList[i]] * 3 + 2]*-1);

		/*tempVertex.position.x = postitions[indexList[i] * 3];
		tempVertex.position.y = postitions[indexList[i] * 3 + 2];
		tempVertex.position.z = (postitions[indexList[i] * 3 + 1] * -1);

		tempVertex.normal.x = normalsPos[normalIdList[offsetIdList[i]] * 3];
		tempVertex.normal.y = normalsPos[normalIdList[offsetIdList[i]] * 3 + 2];
		tempVertex.normal.z = (normalsPos[normalIdList[offsetIdList[i]] * 3 + 1] * -1);*/

		tempVertex.tangent.x = tangents[normalIdList[offsetIdList[i]]].x;
		tempVertex.tangent.y = tangents[normalIdList[offsetIdList[i]]].y;
		tempVertex.tangent.z = tangents[normalIdList[offsetIdList[i]]].z;

		tempVertex.UV.u = u[uvIds[offsetIdList[i]]];
		tempVertex.UV.v = 1.0 - v[uvIds[offsetIdList[i]]];

		/*for loop that controls the weight. If the weight is to low,
		the influence will be set to -1. Which means that the weight will
		not be calculated in the engine.*/
		for (int k = 0; k < 4; ++k)
		{
			tempVertex.weights[k] = skinList->at(indexList[i]).weights[k];
			if (tempVertex.weights[k] < 0.01f)
			{
				tempVertex.influence[k] = -1;
			}
			else
			{
				tempVertex.influence[k] = skinList->at(indexList[i]).boneInfluences[k];
			}
		}

		bool exists = false;

		for (int j = 0; j < sVertices->size(); ++j)
		{
			if (memcmp(&tempVertex, &sVertices->at(j), sizeof(Vertex)) == 0)
			{
				exists = true;
				newIndex->push_back(j);
				break;
			}
		}
		if (!exists)
		{
			newIndex->push_back((unsigned int)sVertices->size());
			sVertices->push_back(tempVertex);
		}
		pBar->setValue(pBar->value() + 1);
	}
	//sVertices->shrink_to_fit(); //kanske sedundant
	//newIndex->shrink_to_fit();
	for (int i = 1; i < newIndex->size(); i += 3)
	{
		unsigned int tempIndex = newIndex->at(i);
		newIndex->at(i) = newIndex->at(i + 1);
		newIndex->at(i + 1) = tempIndex;
	}

	/*creating the mesh header and setting the length of the vertices and indices*/

	hHead.indexLength = (unsigned int)newIndex->size();
	hHead.vertices = (unsigned int)sVertices->size();
	//hHead.jointCount = jointCount;

	/*Getting the transformation matrix*/
	//MFnDependencyNode depNode = mMesh.parent(0);
	//MFnMatrixData parentMatrix = depNode.findPlug("pm").elementByLogicalIndex(0).asMObject();
	//hHead.transMatrix = mTran.transformationMatrix()*parentMatrix.matrix();

	obbHead = *newBox.getObbHead();

	/*writing the information to the binary file*/
	outFile->write((char*)&hHead, sizeof(MeshHeader));
	outFile->write((char*)sVertices->data(), sizeof(SkelVertex)*sVertices->size());
	outFile->write((char*)newIndex->data(), sizeof(unsigned int)*newIndex->size());
	
	//outFile->write((char*)&obbHead, sizeof(BoundingBoxHeader));

	/*clearing the variables*/
	sVertices->clear();
	newIndex->clear();
	outFile->close();
}

void MeshExport::exportStatic(MFnMesh & mMesh, MFnTransform & mTran,bool customObb)
{
	/*Declaring variables to be used*/

	string attrName = mTran.name().asChar();


	if (attrName != "BBOX")
	{
		MIntArray indexList, offsetIdList, normalCount, uvCount, uvIds, normalIdList;
		MFloatPointArray points;
		MFloatArray u, v;
		MFloatVectorArray tangents;
		MeshHeader hHead;
		BoundingBoxHeader obbHeads;
		QWidget *control = MQtUtil::findControl("progressBar");
		QProgressBar *pBar = (QProgressBar*)control;


		/*getting the index list of the vertex positions*/
		mMesh.getTriangles(offsetIdList, indexList);
		mMesh.getTriangleOffsets(uvCount, offsetIdList);
		mMesh.getAssignedUVs(uvCount, uvIds);
		mMesh.getNormalIds(normalCount, normalIdList);
		mMesh.getTangents(tangents, MSpace::kObject);

		/*getting the data for the vertices*/
		const float * postitions = mMesh.getRawPoints(NULL);
		const float * normalsPos = mMesh.getRawNormals(NULL);
		mMesh.getUVs(u, v);

		/*extracting all the information from maya and putting them in a array of vertices*/
		newIndex = new vector<unsigned int>[indexList.length()];
		vertices = new vector<Vertex>[indexList.length()];
		hHead.hasSkeleton = false;

		Vertex tempVertex;

		//Recalculating the vertices using only the unique vertices based on individual normals
		for (unsigned int i = 0; i < indexList.length(); ++i)
		{
			tempVertex.position.x = postitions[indexList[i] * 3];
			tempVertex.position.y = postitions[indexList[i] * 3 + 1];
			tempVertex.position.z = (postitions[indexList[i] * 3 + 2] * -1);

			tempVertex.normal.x = normalsPos[normalIdList[offsetIdList[i]] * 3];
			tempVertex.normal.y = normalsPos[normalIdList[offsetIdList[i]] * 3 + 1];
			tempVertex.normal.z = (normalsPos[normalIdList[offsetIdList[i]] * 3 + 2] * -1);

			tempVertex.tangent.x = tangents[normalIdList[offsetIdList[i]]].x;
			tempVertex.tangent.y = tangents[normalIdList[offsetIdList[i]]].y;
			tempVertex.tangent.z = tangents[normalIdList[offsetIdList[i]]].z;

			tempVertex.UV.u = u[uvIds[offsetIdList[i]]];
			tempVertex.UV.v = 1.0 - v[uvIds[offsetIdList[i]]];

			bool exists = false;

			for (int j = 0; j < vertices->size(); ++j)
			{
				if (memcmp(&tempVertex, &vertices->at(j), sizeof(Vertex)) == 0)
				{
					exists = true;
					newIndex->push_back(j);
					break;
				}
			}
			if (!exists)
			{
				newIndex->push_back((unsigned int)vertices->size());
				vertices->push_back(tempVertex);
			}
			pBar->setValue(pBar->value() + 1);
		}
		for (int i = 1; i < newIndex->size(); i += 3)
		{
			unsigned int tempIndex = newIndex->at(i);
			newIndex->at(i) = newIndex->at(i + 1);
			newIndex->at(i + 1) = tempIndex;
		}
		//vertices->shrink_to_fit();
		//newIndex->shrink_to_fit();

		/*creating the mesh header and setting the length of the vertices and indices*/
		hHead.indexLength = (unsigned int)newIndex->size();
		hHead.vertices = (unsigned int)vertices->size();

		/*Getting the transformation matrix*/
		//MFnDependencyNode depNode = mMesh.parent(0);
		//MFnMatrixData parentMatrix = depNode.findPlug("pm").elementByLogicalIndex(0).asMObject();
		//hHead.transMatrix = mTran.transformationMatrix()*parentMatrix.matrix();
	//	if (!customObb)
		//{
		
		//obbHead = newBox.getObbHead();
		obbHeads = this->obbHead;
		
		//}
		/*writing the information to the binary file*/
		outFile->write((char*)&hHead, sizeof(MeshHeader));
		outFile->write((char*)vertices->data(), sizeof(Vertex)*hHead.vertices);
		outFile->write((char*)newIndex->data(), sizeof(unsigned int)*hHead.indexLength);
		outFile->write((char*)&obbHeads, sizeof(BoundingBoxHeader));

		/*deleting allocated variables*/
		vertices->clear();
		newIndex->clear();
		outFile->close();
	}
}
