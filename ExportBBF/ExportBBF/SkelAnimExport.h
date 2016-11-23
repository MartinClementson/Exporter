#ifndef SKELANIMEXPORT_H
#define SKELANIMEXPORT_H
 
#include "maya_includes.h"
#include "HeaderStructs.h"
#include <vector>
#include <map>

#define PI 3.14159265

class SkelAnimExport
{
public:

    SkelAnimExport();
    ~SkelAnimExport();

    std::vector<SkinData> skinList;
    std::vector<JointData> jointList;

    void IterateSkinClusters();
    void IterateJoints();
    void IterateAnimations();

    void LoadSkinData(MObject skinNode);
    void LoadJointData(MObject jointNode, int parentIndex, int currentIndex);

private:
    /*Function that converts a MMatrix to a float[16] array.*/
    void ConvertMMatrixToFloatArray(MMatrix inputMatrix, float outputMatrix[16]);
};

#endif 
