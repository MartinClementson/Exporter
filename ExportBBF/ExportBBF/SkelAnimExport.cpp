#include "SkelAnimExport.h"

SkelAnimExport::SkelAnimExport()
{
}

bool animExists(const std::string& filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}

SkelAnimExport::SkelAnimExport(string & filePath)
{
	m_filePath = filePath;
}

SkelAnimExport::~SkelAnimExport()
{
}

void SkelAnimExport::IterateSkinClusters()
{
    MStatus res;

    MItDependencyNodes skinIter(MFn::kSkinClusterFilter, &res);
    if (res == MStatus::kSuccess)
    {
        while (!skinIter.isDone())
        {
            LoadSkinData(skinIter.item());

            skinIter.next();
        }
    }
}

void SkelAnimExport::IterateJoints()
{
    MStatus res;

    MItDag jointIter(MItDag::kDepthFirst, MFn::kJoint, &res);

    if (res == MStatus::kSuccess)
    {
        LoadJointData(jointIter.item(), -1, 0);
    }
}

void SkelAnimExport::IterateAnimations(bool anims)
{
	if (anims)
	{
		MStatus res;

		MFnIkJoint jointFn;

		MPlugArray soloArray;

		/*Iterates all animation layers and setting their weight value to 0.
		Later we want this because processing every layer requires all others
		to be set to 0, otherwise probably we can't get the keyframe values.*/
		MItDependencyNodes layerWeightIter(MFn::kAnimLayer, &res);
		if (res == MStatus::kSuccess)
		{
			while (!layerWeightIter.isDone())
			{
				MFnDependencyNode animLayerFn(layerWeightIter.item(), &res);

				MPlug soloPlug = animLayerFn.findPlug("solo", &res);
				MPlug mutePlug = animLayerFn.findPlug("mute", &res);

				soloPlug.setBool(false);
				mutePlug.setBool(false);

				soloArray.append(soloPlug);

				layerWeightIter.next();
			}
		}

		int soloCounter = 0;
		/*Iterates every animation layer for the joints of the skeleton.*/
		MItDependencyNodes animLayerIter(MFn::kAnimLayer, &res);
		if (res == MStatus::kSuccess)
		{
			while (!animLayerIter.isDone())
			{
				MFnDependencyNode animLayerFn(animLayerIter.item(), &res);

				/*Skip the base animation layer, we DON'T EVER use that as a layer for this project.*/
				if (animLayerFn.name() == "BaseAnimation")
				{
					animLayerIter.next();
					soloArray[soloCounter].setDouble(false);
					soloCounter++;
					continue;
				}

				int jointCounter = 0;
				nrOfAnimLayers++;

				MainHeader s_Head;
				string tempAnimId = m_filePath + m_meshName + "_" + string(animLayerFn.name().asChar()) + ".anim";
				s_Head.type = (int)Resources::ResourceType::RES_ANIMATION;
				s_Head.id = (unsigned int)std::hash<std::string>{}(tempAnimId);

				LayerIdHeader layerId{ s_Head.id };
				animIdList.push_back(layerId);

				if (animExists(tempAnimId))
				{
					string animName = ("Overwrite " + m_meshName + "_" + string(animLayerFn.name().asChar()) + "?");
					std::wstring stemp = std::wstring(animName.begin(), animName.end());
					LPCWSTR sw = stemp.c_str();
					if (MessageBox(NULL, sw, TEXT(".anim file already exists"), MB_YESNO) == IDYES)
					{
						fstream animationFile(tempAnimId.c_str(), std::fstream::out | std::fstream::binary);

						animationFile.write((char*)&s_Head, sizeof(MainHeader));

						JointAnimHeader jointAnimHead;
						jointAnimHead.jointCount = jointList.size();
						animationFile.write((char*)&jointAnimHead, sizeof(JointAnimHeader));

						/*Set weight plug to 1, for the current animation layer that is extracted.*/
						soloArray[soloCounter].setDouble(true);

						/*Set zero to the previous animation layer being extracted.*/
						soloArray[soloCounter - 1].setDouble(false);

						/*Iterate all blend nodes, which are the joints connected to each animation layer.*/
						MPlug blendNodePlug = animLayerFn.findPlug("blendNodes", &res);
						if (res == MStatus::kSuccess)
						{
							MItDependencyGraph blendIter(animLayerIter.item(), MFn::kBlendNodeAdditiveRotation,
								MItDependencyGraph::Direction::kDownstream, MItDependencyGraph::Traversal::kBreadthFirst,
								MItDependencyGraph::Level::kNodeLevel, &res);

							blendIter.enablePruningOnFilter();

							while (!blendIter.isDone())
							{
								/*There is a strange occurrence when the blend nodes are connected to two layers in a iteration,
								so if the joint counter is equal or greater than joint list, break from the loop.*/
								if (jointCounter >= jointList.size())
									break;

								MFnDependencyNode blendFn(blendIter.currentItem(), &res);

								MString jointName;

								/*Find the connection plug from the blend node to find the "REAL joint" node.*/
								MPlug outputPlug = MFnDependencyNode(blendIter.currentItem()).findPlug("rotateOrder", &res);
								if (res == MStatus::kSuccess)
								{
									MPlugArray outputConnection;
									outputPlug.connectedTo(outputConnection, true, false, &res);
									if (outputConnection.length())
									{
										/*Set the joint Fn from the node of the connection plug.*/
										jointFn.setObject(outputConnection[0].node());
										jointName = jointFn.name();
									}
								}
								/*Find the plug that is connected to the animation curve.*/
								MPlug inputBPlug = blendFn.findPlug("inputB", &res);

								if (res == MStatus::kSuccess)
								{
									MObjectArray objArray;

									/*Finds the animation curve connected to the blend node plug, which is "animated".*/
									MAnimUtil::findAnimation(inputBPlug.child(0), objArray, &res);

									if (objArray.length())
									{
										/*If a keyframe is set in the timeline in Maya, all the curves will get a value by default,
										so to get keyframe values, only the rotation curve is required to get ALL transformation values.*/
										MFnAnimCurve animCurveFn(objArray[0], &res);

										/*Obtain the number of keys from each animation curve found.*/
										int numKeys = animCurveFn.numKeyframes(&res);

										JointKeyFrameHeader keyTrack;
										keyTrack.numKeys = numKeys;

										animationFile.write((char*)&keyTrack, sizeof(JointKeyFrameHeader));

										for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
										{
											KeyframeHeader keyData;

											/*The current time on the timeline in Maya for each set keyframe.*/
											MTime keyTime = animCurveFn.time(keyIndex);
											/*The current time value for each keyframe, in seconds format.*/
											keyData.timeValue = keyTime.as(MTime::kSeconds);

											/*With the time of the current keyframe, we set where the keyframe is set in timeline.*/
											MAnimControl::setCurrentTime(keyTime);

											/*Keyframes transformation values are obtained here: quat, trans and scale.*/
											double rotation[3];
											MTransformationMatrix::RotationOrder rotOrder;

											MQuaternion quaternion;

											if (jointFn.getRotation(quaternion, MSpace::kObject))
											{
												quaternion[0] *= -1.0;
												quaternion[1] *= -1.0;

												keyData.quaternion[0] = quaternion[0];
												keyData.quaternion[1] = quaternion[1];
												keyData.quaternion[2] = quaternion[2];
												keyData.quaternion[3] = quaternion[3];
											}

											MVector transVec = jointFn.getTranslation(MSpace::kObject, &res);
											if (res == MStatus::kSuccess)
											{
												double translation[3];
												transVec.get(translation);

												translation[2] *= -1.0;

												std::copy(translation, translation + 3, keyData.translation);
											}

											double scale[3];
											if (jointFn.getScale(scale))
											{
												std::copy(scale, scale + 3, keyData.scale);
											}

											animationFile.write((char*)&keyData, sizeof(KeyframeHeader));
										}

										jointCounter++;
									}
								}
								blendIter.next();
							}
						}

						soloCounter++;
						animationFile.close();
					}
				}
				else
				{
					fstream animationFile(tempAnimId.c_str(), std::fstream::out | std::fstream::binary);

					animationFile.write((char*)&s_Head, sizeof(MainHeader));

					JointAnimHeader jointAnimHead;
					jointAnimHead.jointCount = jointList.size();
					animationFile.write((char*)&jointAnimHead, sizeof(JointAnimHeader));

					/*Set weight plug to 1, for the current animation layer that is extracted.*/
					soloArray[soloCounter].setDouble(true);

					/*Set zero to the previous animation layer being extracted.*/
					soloArray[soloCounter - 1].setDouble(false);

					/*Iterate all blend nodes, which are the joints connected to each animation layer.*/
					MPlug blendNodePlug = animLayerFn.findPlug("blendNodes", &res);
					if (res == MStatus::kSuccess)
					{
						MItDependencyGraph blendIter(animLayerIter.item(), MFn::kBlendNodeAdditiveRotation,
							MItDependencyGraph::Direction::kDownstream, MItDependencyGraph::Traversal::kBreadthFirst,
							MItDependencyGraph::Level::kNodeLevel, &res);

						blendIter.enablePruningOnFilter();

						while (!blendIter.isDone())
						{
							/*There is a strange occurrence when the blend nodes are connected to two layers in a iteration,
							so if the joint counter is equal or greater than joint list, break from the loop.*/
							if (jointCounter >= jointList.size())
								break;

							MFnDependencyNode blendFn(blendIter.currentItem(), &res);

							MString jointName;

							/*Find the connection plug from the blend node to find the "REAL joint" node.*/
							MPlug outputPlug = MFnDependencyNode(blendIter.currentItem()).findPlug("rotateOrder", &res);
							if (res == MStatus::kSuccess)
							{
								MPlugArray outputConnection;
								outputPlug.connectedTo(outputConnection, true, false, &res);
								if (outputConnection.length())
								{
									/*Set the joint Fn from the node of the connection plug.*/
									jointFn.setObject(outputConnection[0].node());

									jointName = jointFn.name();
								}
							}
							/*Find the plug that is connected to the animation curve.*/
							MPlug inputBPlug = blendFn.findPlug("inputB", &res);

							if (res == MStatus::kSuccess)
							{
								MObjectArray objArray;

								/*Finds the animation curve connected to the blend node plug, which is "animated".*/
								MAnimUtil::findAnimation(inputBPlug.child(0), objArray, &res);

								if (objArray.length())
								{
									/*If a keyframe is set in the timeline in Maya, all the curves will get a value by default,
									so to get keyframe values, only the rotation curve is required to get ALL transformation values.*/
									MFnAnimCurve animCurveFn(objArray[0], &res);

									/*Obtain the number of keys from each animation curve found.*/
									int numKeys = animCurveFn.numKeyframes(&res);

									JointKeyFrameHeader keyTrack;
									keyTrack.numKeys = numKeys;

									animationFile.write((char*)&keyTrack, sizeof(JointKeyFrameHeader));

									for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
									{
										KeyframeHeader keyData;

										/*The current time on the timeline in Maya for each set keyframe.*/
										MTime keyTime = animCurveFn.time(keyIndex);
										/*The current time value for each keyframe, in seconds format.*/
										keyData.timeValue = keyTime.as(MTime::kSeconds);

										/*With the time of the current keyframe, we set where the keyframe is set in timeline.*/
										MAnimControl::setCurrentTime(keyTime);

										/*Keyframes transformation values are obtained here: quat, trans and scale.*/
										double rotation[3];
										MTransformationMatrix::RotationOrder rotOrder;

										MQuaternion quaternion;

										if (jointFn.getRotation(quaternion, MSpace::kObject))
										{
											quaternion[0] *= -1.0;
											quaternion[1] *= -1.0;

											keyData.quaternion[0] = quaternion[0];
											keyData.quaternion[1] = quaternion[1];
											keyData.quaternion[2] = quaternion[2];
											keyData.quaternion[3] = quaternion[3];
										}

										MVector transVec = jointFn.getTranslation(MSpace::kObject, &res);
										if (res == MStatus::kSuccess)
										{
											double translation[3];
											transVec.get(translation);

											translation[2] *= -1.0;

											std::copy(translation, translation + 3, keyData.translation);
										}

										double scale[3];
										if (jointFn.getScale(scale))
										{
											std::copy(scale, scale + 3, keyData.scale);
										}

										animationFile.write((char*)&keyData, sizeof(KeyframeHeader));
									}
									jointCounter++;
								}
							}
							blendIter.next();
						}
					}

					soloCounter++;
					animationFile.close();
				}
				animLayerIter.next();
			}
		}
	}
}

void SkelAnimExport::LoadSkinData(MObject skinNode)
{
    MStatus res;

    float weightSum = 0;

    int weightsCounter = 0;
    int influenceCounter = 0;

    MFnSkinCluster skinFn(skinNode, &res);
    if (res == MStatus::kSuccess)
    {
        MDagPathArray influences;

        /*Find the number of geometries connecting to the skinCluster, in this case it's always 1 mesh connected.*/
        unsigned int numGeoms = skinFn.numOutputConnections(&res);
        for (unsigned int geomIndex = 0; geomIndex < numGeoms; geomIndex++)
        {
            /*Obtain the index for the connected mesh.*/
            unsigned int index = skinFn.indexForOutputConnection(geomIndex, &res);

            /*Obtain influence count, which is the joints controlling the binded skin mesh.*/
            MDagPathArray jointPathArray;
            skinFn.influenceObjects(jointPathArray, &res);
            unsigned int jointArrayLength = jointPathArray.length();

            /*Find the path to the connected mesh, and iterate it's control vertices. */
            MDagPath skinPath;
            if (skinFn.getPathAtIndex(index, skinPath))
            {
                /*Iterator for the path to the binded skin mesh, iterates all control vertices.*/
                MItGeometry geometryIter(skinPath, &res);

                /*Total count of all control points in Mesh, need to be readjusted for indices.*/
                int controlVerticesCount = geometryIter.count(&res);

                /*Resize the blend data list to be the size of control vertices count.*/
                skinList.resize(controlVerticesCount);

                while (!geometryIter.isDone())
                {
                    /*Obtain each control vertex componoment object.*/
                    MObject component = geometryIter.component(&res);

                    int cvIndex = geometryIter.index();

                    MIntArray inflIndexArray;

                    /*Append each joint's dagpath index to the array.*/
                    for (unsigned jointIndex = 0; jointIndex < jointArrayLength; jointIndex++)
                    {
                        inflIndexArray.append(skinFn.indexForInfluenceObject(jointPathArray[jointIndex]));
                    }

                    MDoubleArray weights;
                    skinFn.getWeights(skinPath, component, inflIndexArray, weights);

                    int weightsLength = weights.length();

                    for (int i = 0; i < weightsLength; i++)
                    {
						/*If a joint has zero weight value, skip it for this control vertex.
						There is always 4 joints influencing a control vertex.*/
						if (weights[i] != 0)
                        {
                            skinList[cvIndex].weights[weightsCounter] = weights[i];
                            skinList[cvIndex].boneInfluences[influenceCounter] = inflIndexArray[i];

                            weightsCounter++, influenceCounter++;

                            weightSum += weights[i];
                        }
                    }

                    weightSum = 0;
                    weightsCounter = 0, influenceCounter = 0;

                    geometryIter.next();
                }
            }
        }
    }
}

void SkelAnimExport::LoadJointData(MObject jointNode, int parentIndex, int currentIndex)
 {
    MStatus res;

    JointHeader jointData;

    MFnIkJoint jointFn(jointNode, &res);
    if (res == MStatus::kSuccess)
    {
        /*Obtain the plug for every joint's bindpose matrix.*/
        MPlug bindPosePlug = jointFn.findPlug("bindPose", &res);
        if (res == MStatus::kSuccess)
        {
           MString jointName = jointFn.name();

           /*Get the matrix as a MObject.*/
           MObject bpNode;
           bindPosePlug.getValue(bpNode);

           /*Retrieve the matrix data from the bindpose MObject.*/
           MFnMatrixData bindPoseFn(bpNode, &res);

           /*The actual bindpose matrix is obtained here from every joint.*/
		   MMatrix tempBindPose = bindPoseFn.matrix(&res);

           if (res == MStatus::kSuccess)
           {
			    MMatrix tempInvBindPose = tempBindPose.inverse();
				float inverseBindPose[16];
				/*Convert MMatrix bindpose to a float[16] array.*/
				ConvertMMatrixToFloatArray(tempInvBindPose, inverseBindPose);
				memcpy(jointData.invBindPose, &inverseBindPose, sizeof(float) * 16);

			   /*Assign both current joint ID and it's parent ID.*/
			   jointData.parentIndex = parentIndex;
			   jointData.jointIndex = currentIndex;

               currentIndex++;

               jointList.push_back(jointData);
           }
        }
    }

    /*This is where the recursive happens. Basically for a "CHEST" joint, both the l_arm and r_arm,
    will know that both are parents to "CHEST" joint.*/
    if (jointFn.childCount() > 0)
    {
        for (int childIndex = 0; childIndex < jointFn.childCount(); childIndex++)
        {
            /*If there is a child in the skeleton hierarchy, decrement the currentIndex, which is now parentIndex.*/
            LoadJointData(jointFn.child(childIndex), currentIndex - 1, jointList.size());
        }
    }
}

void SkelAnimExport::setFilePath(string & filePath)
{
	this->m_filePath = filePath;
}

void SkelAnimExport::setMeshName(string & meshName)
{
	this->m_meshName = meshName;
}

void SkelAnimExport::writeJointData()
{
	if (animExists((m_filePath + m_meshName + ".skel")))
	{
		string animName = ("Overwrite " + m_meshName + ".skel?");
		std::wstring stemp = std::wstring(animName.begin(), animName.end());
		LPCWSTR sw = stemp.c_str();
		MainHeader s_head;
		s_head.id = (unsigned int)std::hash<std::string>{}((m_filePath + m_meshName + ".skel"));
		this->m_UID = s_head.id;
		if (MessageBox(NULL, sw, TEXT(".skel file already exists"), MB_YESNO) == IDYES)
		{
			fstream skeletonFile((m_filePath + m_meshName + ".skel"), std::fstream::out | std::fstream::binary);

			SkeletonHeader skelHeader;
			skelHeader.jointCount = jointList.size();
			skelHeader.animLayerCount = nrOfAnimLayers;

			s_head.type = (int)Resources::ResourceType::RES_SKELETON;

			skeletonFile.write((char*)&s_head, sizeof(MainHeader));

			skeletonFile.write((char*)&skelHeader, sizeof(SkeletonHeader));

			skeletonFile.write((char*)jointList.data(), sizeof(JointHeader) * skelHeader.jointCount);

			skeletonFile.write((char*)animIdList.data(), sizeof(LayerIdHeader) * nrOfAnimLayers);

			skeletonFile.close();
		}
	}
	else
	{
		fstream skeletonFile((m_filePath + m_meshName + ".skel"), std::fstream::out | std::fstream::binary);

		SkeletonHeader skelHeader;
		skelHeader.jointCount = jointList.size();
		skelHeader.animLayerCount = nrOfAnimLayers;

		MainHeader s_head;
		s_head.type = (int)Resources::ResourceType::RES_SKELETON;
		s_head.id = (unsigned int)std::hash<std::string>{}((m_filePath + m_meshName + ".skel"));
		this->m_UID = s_head.id;

		skeletonFile.write((char*)&s_head, sizeof(MainHeader));

		skeletonFile.write((char*)&skelHeader, sizeof(SkeletonHeader));

		skeletonFile.write((char*)jointList.data(), sizeof(JointHeader) * skelHeader.jointCount);

		skeletonFile.write((char*)animIdList.data(), sizeof(LayerIdHeader) * nrOfAnimLayers);

		skeletonFile.close();
	}
}

void SkelAnimExport::ConvertMMatrixToFloatArray(MMatrix inputMatrix, float outputMatrix[16])
{
    int matrixCounter = 0;

    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            outputMatrix[matrixCounter] = inputMatrix[row][column];

            matrixCounter++;
        }
    }

	/*Flip the Z-value of translation in each bindpose matrix, for conversion to DirectX.*/
	outputMatrix[14] *= -1;
}
