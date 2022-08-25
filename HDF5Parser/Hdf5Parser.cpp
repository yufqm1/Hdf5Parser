#include "Hdf5Parser.h"

#include <iostream>
#define OBJ_NAME_LEN 260

Hdf5Parser::Hdf5Parser()
{
}

Hdf5Parser::~Hdf5Parser()
{
}

H5Data Hdf5Parser::getH5Data()
{
	return m_h5Data;
}

bool Hdf5Parser::readHdf5(const char* path)
{
	H5File file(path, H5F_ACC_RDONLY);
	objTraverse(file);

	return false;
}

void Hdf5Parser::readH5Group(const Group& group, const char* objName)
{
	Group _group = group.openGroup(objName);
	objTraverse(_group);
}

void Hdf5Parser::readH5DataSet(const Group& group, const char* objName)
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

	float* outdata = new float[DIM];
	dataset.read(outdata, PredType::NATIVE_FLOAT, mspace, fspace);

	BASEDATAVec baseData;
	for (int i = 0;i< DIM;i++)
	{
		baseData.emplace_back(outdata[i]);
	}

	m_h5Data.insert(std::pair<std::string, BASEDATAVec>(buffname,baseData));

	delete[] outdata;
	outdata = NULL;
}

void Hdf5Parser::objTraverse(const Group& group)
{
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
		{
			readH5Group(group, objName);
		}
			break;
		case H5O_TYPE_DATASET:
		{
			readH5DataSet(group, objName);
		}
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
}
