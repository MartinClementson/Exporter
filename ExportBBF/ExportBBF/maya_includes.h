#ifndef MAYA_INCLUDES_H
#define MAYA_INCLUDES_H

//some definitions for the DLL to play nice with Maya
#define NT_PLUGIN
#define REQUIRE_IOSTREAM
#define EXPORT __declspec(dllexport)

//#include <QtGui\qevent.h>
////#include <qtwidgets\qabstractbutton.h>
//#include <QtWidgets\qmainwindow.h>
////#include <qtwidgets\qapplication.h>
//#include <QtWidgets\qpushbutton.h>
//#include <QtWidgets\qapplication.h>
//#include <QtWidgets\qcheckbox.h>
//#include <QtCore\qobject.h>
#include <maya/MQtUtil.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MUintArray.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MEulerRotation.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
#include <maya/MFnCamera.h>
#include <maya/M3dView.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MImage.h>
#include <maya/MFnPointLight.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MIteratorType.h>
#include <maya/MFloatMatrix.h>

// Includes for openMayaAnim
#include <maya/MFnAnimCurve.h>
#include "maya/MFnSkinCluster.h"
#include "maya/MFnIkJoint.h"
#include <maya/MFnMatrixData.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MItKeyframe.h>
#include <maya/MItDependencyGraph.h>
#include "maya/MItGeometry.h"
#include "maya/MAnimControl.h"
#include "maya/MQuaternion.h"
#include "maya/MTime.h"
#include "maya/MAnimUtil.h"

#include "maya/MFnSet.h"
#include <Windows.h>
#include <WinBase.h>


#include <vector>
#include <iostream>
#include <fstream>

// Wrappers
#include <maya/MGlobal.h>
#include <maya/MCallbackIdArray.h>

// Messages
#include <maya/MMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MDagMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MModelMessage.h>
#include <maya/MBoundingBox.h>
// Commands
#include <maya/MPxCommand.h>

// Libraries to link from Maya
// This can be also done in the properties setting for the project.
//#pragma comment(lib,"Foundation.lib")
//#pragma comment(lib,"OpenMaya.lib")
//#pragma comment(lib,"OpenMayaUI.lib")
//#pragma comment(lib, "OpenMayaAnim.lib")

#include <DirectXMath.h>

#endif



