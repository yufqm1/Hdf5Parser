/**
 * datatime: 22/9/7
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
typedef std::map<std::string, std::vector<double>> DouMap;
typedef std::map<std::string, std::vector<int>> IntMap;
struct ItemNode {
	ItemNode(const char* s_name) {
		name = s_name;
	}
	ItemNode* pre;
	std::string name;
	std::vector<ItemNode*> itemVec;
	std::vector<ItemNode*> subVec;
};

struct H5Item {
	H5Item(const char* s_name) {
		name = s_name;
	}
	std::string name;
	std::vector<ItemNode*> itemVec;
	H5Item* next = nullptr;
};

class Hdf5Parser
{
public:
	Hdf5Parser();
	~Hdf5Parser();
	bool readHdf5(const char* path);
	// 2D	Curve
	H5Item* getH5Item();
	std::vector<double> getCurveDateSet(ItemNode* itemNode); // const ItemNode& para
	// 3D	Animation
	std::vector<double> getTimeStamps();
	DouMap getAnimationBaseData();
	IntMap getAnimationIndex();

private:
	bool readH5Group(const H5::Group& group, const char* objName);
	bool readH5DataSet(const H5::Group& group, const char* objName);
	bool objTraverse(const H5::Group& group);
	void clearH5Data();

	std::vector<std::string> getObjects();
	std::vector<std::string> getCharacteristics();
	std::vector<ItemNode*> createItemNodes(ItemNode* parent, const char* item);
	ULLONG getMemSize();
private:
	H5Item* m_filter;
	H5Item* m_object;
	H5Item* m_character;
	H5Item* m_component;
	// simple name
	std::vector<std::string> m_curveAttri;
	std::vector<std::string> m_animationAttri;

	// index to find base dataset value
	std::map<std::string,std::vector<double>> m_curveIndex;
	std::map<std::string, std::vector<double>> m_animationIndex;
	// base dataset value
	std::map<std::string, std::vector<double>> m_curveBaseData;
	std::map<std::string, std::vector<double>> m_animationBaseData;

	std::vector<double> m_timeStamps;
	ULLONG m_memSize;
};

#endif
