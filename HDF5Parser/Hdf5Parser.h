/**
 * datatime 
 * 
 */

#ifndef __HDF5PARSER_H__
#define __HDF5PARSER_H__

#include <vector>
#include <map>
#include <queue>
#include "H5Cpp.h"
#include "Eigen/Eigen"

typedef unsigned long long ULLONG;
using namespace std;
typedef std::map<std::string, Eigen::VectorXd> EigenH51Dim;
typedef std::map<std::string, Eigen::MatrixXd> EigenH52Dim;

enum GeoType
{
	Curve,
	Animation,
};

typedef struct Hdf5Data
{
	EigenH51Dim data1Dim;
	EigenH52Dim data2Dim;
} H5Data;

//----------------------
struct Component {
	string name;
	vector<double> values;
};

struct Characteristic {
	string name;
	vector<Component> component;
};

struct Object {
	string name;
	vector<Characteristic> sonObj;
	vector<Characteristic> characters;
};

struct Filter {
	string name;
	vector<Object> object;
};
//---------------------------------

struct BaseNode
{
	string name;
	std::vector<BaseNode*> nodeVec;
};

struct NodeObject {
	string name;
	NodeObject* nobj = nullptr;

	BaseNode* node;
};

struct TimeStamps
{
	string name;
	vector<double> stamps;
};

struct BaseDatumPara
{
	GeoType type;
	string objectName;
	string sonObjName;
	string characteristicName;
	string componentName; 
};

struct ItemNode {
	ItemNode* parent;
	std::string name;
	vector<ItemNode*> itemVec;
	vector<ItemNode*> subVec;
};

struct Item {
	std::string name;
	ItemNode* itemNode;
	Item* item = nullptr;
};

class Hdf5Parser
{
public:
	Hdf5Parser();
	~Hdf5Parser();
	bool readHdf5(const char* path);
	// an interface for the constraint of curve
	TimeStamps getTimeStamps();
	Filter getCurveFilterInfo();
	vector<double> getBaseDatum(const BaseDatumPara& para); // const ItemNode& para
	bool getDataInfo(Item* item);
private:
	bool readH5Group(const H5::Group& group, const char* objName);
	bool readH5DataSet(const H5::Group& group, const char* objName);
	bool objTraverse(const H5::Group& group);
	void clearH5Data();

	ULLONG getMemSize();
	EigenH51Dim getH5Data1Dim();
	EigenH52Dim getH5Data2Dim();

	std::vector<std::string> getCurveObjectNames();
	std::vector<std::string> getCurveCharaceristic();
	std::vector<double> getComponent(std::string character, std::string component);

	void getHdf5Data(H5Data& h5Data);

	vector<string> getObjects();
	vector<string> getCharacteristics();
	
private:
	H5Data m_h5Data;
	EigenH51Dim m_eigenH51Dim;
	EigenH52Dim m_eigenH52Dim;
	std::vector<std::string> m_curveAttri;
	std::vector<std::string> m_animationAttri;

	// index to find base dataset value
	std::map<std::string,std::vector<double>> m_curveIndex;	
	std::map<std::string, std::vector<double>> m_animationIndex;

	// base dataset value
	std::map<string, vector<double>> curveBaseData;
	std::map<string, vector<double>> animationBaseData;

	TimeStamps m_timeStamps;

	ULLONG m_memSize;
	Filter m_filter;
};

#endif
