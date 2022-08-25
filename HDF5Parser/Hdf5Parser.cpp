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

H5Data Hdf5Parser::getH5Data()
{
	return m_h5Data;
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
	DataSet dataset = group.openDataSet(objName);
	DataSpace fspace = dataset.getSpace();

	char buffname[OBJ_NAME_LEN] = {0};
	dataset.getObjName(buffname,260);

	hsize_t dims_out[1];
	int dims = fspace.getSimpleExtentDims(dims_out, NULL);
	const int DIM = dims_out[0];

	hsize_t start[1];
	hsize_t count[1];
	start[0] = 0;
	count[0] = DIM;
	hsize_t mdim[] = { DIM };
	DataSpace mspace(1, mdim);
	mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

	double* outdata = new double[DIM];
	dataset.read(outdata, PredType::NATIVE_DOUBLE, mspace, fspace);

	BASEDATAVec baseData;
	for (int i = 0;i< DIM;i++)
	{
		baseData.emplace_back(outdata[i]);
	}

	// 超过一定大小，表明读取文件失败，此时返回失败,并清空数据
	m_memSize += sizeof(double) * DIM;
	if (m_memSize > MEMORY_MAX)
	{
		m_h5Data.clear();
		return false;
	}

	m_h5Data.insert(std::pair<std::string, BASEDATAVec>(buffname,baseData));

	delete[] outdata;
	outdata = NULL;

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
