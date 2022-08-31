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
#include "Eigen/Eigen"

typedef unsigned long long ULLONG;

typedef std::map<std::string, Eigen::VectorXd> EigenH51Dim;
typedef std::map<std::string, Eigen::MatrixXd> EigenH52Dim;

typedef struct Hdf5Data
{
	EigenH51Dim data1Dim;
	EigenH52Dim data2Dim;
} H5Data;

class Hdf5Parser
{
public:
	Hdf5Parser();
	~Hdf5Parser();
	void getHdf5Data(H5Data& h5Data);
	bool readHdf5(const char* path);

	ULLONG getMemSize();
	EigenH51Dim getH5Data1Dim();
	EigenH52Dim getH5Data2Dim();
private:
	bool readH5Group(const H5::Group& group, const char* objName);
	bool readH5DataSet(const H5::Group& group, const char* objName);
	bool objTraverse(const H5::Group& group);
	void clearH5Data();
private:
	H5Data m_h5Data;
	EigenH51Dim m_eigenH51Dim;
	EigenH52Dim m_eigenH52Dim;
	
	ULLONG m_memSize;
};

#endif
