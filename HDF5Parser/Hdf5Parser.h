#ifndef __HDF5PARSER_H__
#define __HDF5PARSER_H__

#include <vector>
#include <map>
#include <queue>
#include "H5Cpp.h"
using namespace H5;

typedef unsigned long long ULLONG;
typedef std::vector<double> BASEDATAVec;
typedef std::map<std::string, BASEDATAVec> H5Data;

class Hdf5Parser
{
public:
	Hdf5Parser();
	~Hdf5Parser();
	H5Data getH5Data();
	ULLONG getMemSize();
	bool readHdf5(const char* path);
private:
	bool readH5Group(const Group& group, const char* objName);
	bool readH5DataSet(const Group& group, const char* objName);
	bool objTraverse(const Group& group);
private:
	H5Data m_h5Data;
	ULLONG m_memSize;
};

#endif
