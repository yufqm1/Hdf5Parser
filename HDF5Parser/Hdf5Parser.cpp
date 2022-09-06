#include "Hdf5Parser.h"

#include <iostream>

#define OBJ_NAME_LEN 260
#define MEMORY_MAX 10 * 1024 * 1024 * 1024		// 10G
using namespace std;

static void _split(const std::string& s, char delim,
	std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	_split(s, delim, elems);
	return elems;
}

Hdf5Parser::Hdf5Parser() : m_memSize(0ULL)
{
}

Hdf5Parser::~Hdf5Parser()
{
}

void Hdf5Parser::getHdf5Data(H5Data& h5Data)
{
	h5Data.data1Dim = m_eigenH51Dim;
	h5Data.data2Dim = m_eigenH52Dim;
}

EigenH51Dim Hdf5Parser::getH5Data1Dim()
{
	return m_eigenH51Dim;
}

EigenH52Dim Hdf5Parser::getH5Data2Dim()
{
	return m_eigenH52Dim;
}

Filter Hdf5Parser::getCurveFilterInfo()
{
	vector<Component> component;

	Component compX;
	compX.name = "x";
	component.emplace_back(compX);
	Component compY;
	compY.name = "y";
	component.emplace_back(compY);
	Component compZ;
	compZ.name = "z";
	component.emplace_back(compZ);

	vector<Characteristic> characteristic;
	string temp;
	for (auto attri : m_curveAttri)
	{
		Characteristic chara;
		if (temp == attri.substr(0, attri.length() - 1))
		{
			continue;
		}

		temp = attri.substr(0, attri.length() - 1);
		chara.name = temp;


		chara.component = component;
		characteristic.emplace_back(chara);
	}

	vector<Characteristic> sonObj;
	string sontmp = "";
	for (auto attri : m_curveIndex)
	{
		std::vector<std::string> sonl = split(attri.first, '/');
		if (sontmp == sonl.back())
		{
			continue;
		}
		sontmp = sonl.back();

		Characteristic son;
		son.name = sonl.back();
		son.component = component;
		sonObj.emplace_back(son);
	}

	vector<Object> objects;
	for (auto st : m_curveIndex)
	{
		cout << st.first << endl;

		Object ob;
		std::vector<std::string> list = split(st.first, '/');
		for (int i = 0;i < list.size() - 1;i++) // minus 1 since there's a superfluous property called a name
		{
			if (list[i] == "Index" && i + 1 < list.size())
			{
				ob.name = list[i + 1];
			}
		}

		ob.characters = characteristic;
		ob.sonObj = sonObj;
		objects.emplace_back(ob);
	}

	Filter filter;
	filter.name = "constraint";
	filter.object = objects;

	return Filter();
}

vector<double> Hdf5Parser::getBaseDatum(const BaseDatumPara& para)
{
	switch (para.type)
	{
	case Curve:
	{
		string key = "/Curve/Index/";
		key.append(para.objectName);
		key.append("/Markers/");
		key.append(para.sonObjName);

		int index = 0;
		for (int i = 0; i < m_curveAttri.size(); i++)
		{
			if (strstr(m_curveAttri[i].c_str(), para.characteristicName.c_str()))
			{
				index = i;
				break;
			}
		}

		if (m_curveIndex.find(key) == m_curveIndex.end())
		{
			break; 
		}

		int queryIndex = m_curveIndex.at(key)[index]; 

		string baseKey = "/Curve/";
		baseKey.append(to_string(queryIndex));


		return curveBaseData.at(baseKey);

		}
		break;
	case Animation:
		break;
	default:
		break;
	}



	return vector<double>();
}

TimeStamps Hdf5Parser::getTimeStamps()
{
	return m_timeStamps;
}

ULLONG Hdf5Parser::getMemSize()
{
	return m_memSize;
}

bool Hdf5Parser::readHdf5(const char* path)
{
	H5::H5File file(path, H5F_ACC_RDONLY);
	bool success = objTraverse(file);
	if (!success)
	{
		// TODO
		return false;
	}

	return true;
}

std::vector<std::string> Hdf5Parser::getCurveObjectNames()
{
	return std::vector<std::string>();
}

std::vector<std::string> Hdf5Parser::getCurveCharaceristic()
{
	return std::vector<std::string>();
}

std::vector<double> Hdf5Parser::getComponent(std::string character, std::string component)
{
	return std::vector<double>();
}

// recurse to read group
bool Hdf5Parser::readH5Group(const H5::Group& group, const char* objName)
{
	H5::Group _group = group.openGroup(objName);
	bool success = objTraverse(_group);
	if (!success)
	{
		// TODO
		return false;
	}

	return true;
}

bool Hdf5Parser::readH5DataSet(const H5::Group& group, const char* objName)
{
	double* outdata = nullptr;
	H5::DataSet dataset = group.openDataSet(objName);
	H5::DataSpace fspace = dataset.getSpace();
	hsize_t* dims_size = new hsize_t[fspace.getSimpleExtentNdims()];
	int rank = fspace.getSimpleExtentDims(dims_size, NULL);
	if (rank == 1)	// one dim dataSet
	{
		int dimNum = dims_size[0];
		hsize_t start[1] = { 0 };
		hsize_t count[1] = { dimNum };

		H5::DataSpace mspace(rank, dims_size);
		mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

		outdata = new double[dimNum];
		dataset.read(outdata, H5::PredType::NATIVE_DOUBLE, mspace, fspace);

		// add into Eigen vector
		Eigen::VectorXd vec(dimNum);
		std::vector<double> temp;
		for (int i = 0; i < dimNum; i++)
		{
			temp.emplace_back(outdata[i]);
			vec(i, 0) = outdata[i];
		}

		// if the value exceeds a certain size,clear h5Data and return false
		m_memSize += sizeof(double) * dimNum;
		if (m_memSize > MEMORY_MAX)
		{
			clearH5Data();
			return false;
		}

		char buffname[OBJ_NAME_LEN] = { 0 };
		dataset.getObjName(buffname, OBJ_NAME_LEN);
		// m_eigenH51Dim.insert(std::pair<std::string, Eigen::VectorXd>(buffname, vec));

		// get attribute
		if (m_curveAttri.empty() && strstr(buffname, "/Curve/Index/"))
		{
			for (int i = 0; i < dataset.getNumAttrs(); i++)
			{
				H5::Attribute attr = dataset.openAttribute(i);
				char test[OBJ_NAME_LEN];
				ssize_t si = attr.getName(test);
				cout << "test =========" << test << endl;

				m_curveAttri.emplace_back(test);
			}
		}

		// classification
		if (strstr(buffname, "/Curve/") && !strstr(buffname,"/Curve/Index/"))
		{
			curveBaseData.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}
		else if (strstr(buffname, "/Curve/Index/"))
		{
			m_curveIndex.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}


		if (strstr(buffname, "/Animation/") && !strstr(buffname, "/Animation/Index/"))
		{
			animationBaseData.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		} else if (strstr(buffname, "/Animation/Index/"))
		{
			m_animationIndex.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}

		if (strstr(buffname,"/TimeStamps"))
		{
			m_timeStamps.name = "TimeStamps";
			m_timeStamps.stamps = temp;
		}
		
	}

	if (rank == 2)	// two dims dataSet
	{
		int row = dims_size[0];
		int col = dims_size[1];
		int dim = row * col;
		hsize_t start[2] = { 0 };
		hsize_t count[2] = { row,col };

		H5::DataSpace mspace(rank, dims_size);
		mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

		outdata = new double[dim];
		dataset.read(outdata, H5::PredType::NATIVE_DOUBLE, mspace, fspace); //The first argument can only be passed to a one-dimensional array
		

		Eigen::MatrixXd matrix(row,col);
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				matrix(i,j) = outdata[i * col + j];
			}
		}

		// if the value exceeds a certain size,clear h5Data and return false
		m_memSize += sizeof(double) * dim;
		if (m_memSize > MEMORY_MAX)
		{
			clearH5Data();
			return false;
		}

		char buffname[OBJ_NAME_LEN] = { 0 };
		dataset.getObjName(buffname, OBJ_NAME_LEN);

		// m_eigenH52Dim.insert(std::pair<std::string, Eigen::MatrixXd>(buffname, matrix));
	}

	if (outdata)
	{
		delete[] outdata;
		outdata = nullptr;
	}
	
	if (dims_size)
	{
		delete[] dims_size;
		dims_size = nullptr;
	}

	return true;

}

// traverse object node
bool Hdf5Parser::objTraverse(const H5::Group& group)
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
			isTraverse = readH5Group(group, objName);	
			break;			// Recursion is non-returnable here
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

void Hdf5Parser::clearH5Data()
{
	m_eigenH51Dim.clear();
	m_eigenH52Dim.clear();
}

