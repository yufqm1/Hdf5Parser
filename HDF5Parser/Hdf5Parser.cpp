#include "Hdf5Parser.h"

#include <iostream>

#define OBJ_NAME_LEN 260
#define MEMORY_MAX 10 * 1024 * 1024 * 1024		// 10G

Hdf5Parser::Hdf5Parser() : m_memSize(0ULL)
{
}

Hdf5Parser::~Hdf5Parser()
{
}

H5Data1Dim Hdf5Parser::getH5Data1Dim()
{
	return m_data1Dim;
}

H5Data2Dim Hdf5Parser::getH5Data2Dim()
{
	return m_data2Dim;
}

ULLONG Hdf5Parser::getMemSize()
{
	return m_memSize;
}

bool Hdf5Parser::readHdf5(const char* path)
{
	H5File file(path, H5F_ACC_RDONLY);
	bool success = objTraverse(file);
	if (!success)
	{
		// TODO
		return false;
	}

	return true;
}

// 递归读取group
bool Hdf5Parser::readH5Group(const Group& group, const char* objName)
{
	Group _group = group.openGroup(objName);
	bool success = objTraverse(_group);
	if (!success)
	{
		// TODO
		return false;
	}

	return true;
}

bool Hdf5Parser::readH5DataSet(const Group& group, const char* objName)
{
	double* outdata = nullptr;
	DataSet dataset = group.openDataSet(objName);	
	DataSpace fspace = dataset.getSpace();
	hsize_t* dims_size = new hsize_t[fspace.getSimpleExtentNdims()];
	int dims = fspace.getSimpleExtentDims(dims_size, NULL);
	if (dims == 1)	// 一维dataSet
	{
		int dimNum = dims_size[0];
		hsize_t start[1] = { 0 };
		hsize_t count[1] = { dimNum };

		DataSpace mspace(dims, dims_size);
		mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

		outdata = new double[dimNum];
		dataset.read(outdata, PredType::NATIVE_DOUBLE, mspace, fspace);

		DATA1DIM baseData;
		for (int i = 0; i < dimNum; i++)
		{
			baseData.emplace_back(outdata[i]);
		}

		// 超过一定大小，表明读取文件失败，此时返回失败,并清空数据
		m_memSize += sizeof(double) * dimNum;
		if (m_memSize > MEMORY_MAX)
		{
			m_data1Dim.clear();
			return false;
		}

		char buffname[OBJ_NAME_LEN] = { 0 };
		dataset.getObjName(buffname, 260);
		m_data1Dim.insert(std::pair<std::string, DATA1DIM>(buffname, baseData));
	}

	if (dims == 2)	// 二维dataSet
	{
		int dim = dims_size[0] * dims_size[1];
		hsize_t start[2] = { 0 };
		hsize_t count[2] = { dims_size[0],dims_size[1] };

		DataSpace mspace(dims, dims_size);
		mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

		outdata = new double[dim];
		dataset.read(outdata, PredType::NATIVE_DOUBLE, mspace, fspace); //第一个参数只能传一维数组

		DATA1DIM data1Dim;
		DATA2DIM data2Dim;
		for (int i = 0; i < dims_size[0]; i++)
		{
			for (int j = 0; j < dims_size[1]; j++)
			{
				data1Dim.emplace_back(outdata[i * dims_size[1] + j]);
			}

			data2Dim.emplace_back(data1Dim);
			data1Dim.clear();	// data1Dim需清空，否则上一维获取的数据会残留
		}

		// 超过一定大小，表明读取文件失败，此时返回失败,并清空数据
		m_memSize += sizeof(double) * dim;
		if (m_memSize > MEMORY_MAX)
		{
			m_data1Dim.clear();
			return false;
		}

		char buffname[OBJ_NAME_LEN] = { 0 };
		dataset.getObjName(buffname, OBJ_NAME_LEN);
		m_data2Dim.insert(std::pair<std::string, DATA2DIM>(buffname, data2Dim));
	}

	delete[] outdata;
	delete[] dims_size;
	outdata = nullptr;
	dims_size = nullptr;

	return true;

}

// 遍历节点object元素
bool Hdf5Parser::objTraverse(const Group& group)
{
	bool isTraverse = false;
	for (size_t i = 0; i < group.getNumObjs(); i++)
	{
		char objName[OBJ_NAME_LEN];
		ssize_t name_size = group.getObjnameByIdx(i, objName, OBJ_NAME_LEN);
		if (name_size <= 0)
		{
			continue;
		}

		H5O_type_t type = group.childObjType(objName);
		switch (type)
		{
		case H5O_TYPE_UNKNOWN:
			break;
		case H5O_TYPE_GROUP:
			isTraverse = readH5Group(group, objName);	// 此处递归不可直接return
			break;
		case H5O_TYPE_DATASET:
			isTraverse = readH5DataSet(group, objName);
			break;
		case H5O_TYPE_NAMED_DATATYPE:
			break;
		case H5O_TYPE_MAP:
			break;
		case H5O_TYPE_NTYPES:
			break;
		default:
			break;
		}
	}

	return isTraverse;
}

