/**
 * 
 * 
 */

#ifndef __HDF5PARSER_H__
#define __HDF5PARSER_H__

#include <vector>
#include <map>
#include <queue>
#include "H5Cpp.h"
using namespace H5;

typedef unsigned long long ULLONG;
typedef std::vector<double> DATA1DIM;
typedef std::vector<DATA1DIM> DATA2DIM;
typedef std::map<std::string, DATA1DIM> H5Data1Dim;
typedef std::map<std::string, DATA2DIM> H5Data2Dim;

class Hdf5Parser
{
public:
	Hdf5Parser();
	~Hdf5Parser();
	H5Data1Dim getH5Data1Dim();
	H5Data2Dim getH5Data2Dim();
	ULLONG getMemSize();
	bool readHdf5(const char* path);
private:
	bool readH5Group(const Group& group, const char* objName);
	bool readH5DataSet(const Group& group, const char* objName);
	bool objTraverse(const Group& group);

private:
	H5Data1Dim m_data1Dim;
	H5Data2Dim m_data2Dim;
	ULLONG m_memSize;
};

#endif
