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

Hdf5Parser::Hdf5Parser() : 
m_memSize(0ULL),
m_filter(nullptr),
m_object(nullptr),
m_character(nullptr),
m_component(nullptr)
{
}

Hdf5Parser::~Hdf5Parser()
{
}

std::vector<std::string> Hdf5Parser::getObjects()
{
	std::vector<std::string> objNames;

	for (auto index : m_curveIndex)
	{
		std::vector<std::string> temp = split(index.first, '/');
		for (int i = 0;i<temp.size() - 1;i++)
		{
			if (temp[i] == "Index")
			{
				objNames.emplace_back(temp[i+1]);
			}
		}

	}

	return objNames;
}

set<string> Hdf5Parser::getCharacteristics()
{
	std::set<std::string> charNames;
	for (auto curve : m_curveAttri)
	{
		if (curve == "Name")
		{
			continue;
		}
		charNames.insert(curve.substr(0, curve.length() - 1));
	}

	return charNames;
}

vector<ItemNode*> Hdf5Parser::createItemNodes(ItemNode* parent,const char* name)
{
	std::vector<ItemNode*> nodeVec;
	std::string s_name = name;
	if (s_name == "Filter") {
		ItemNode *node = new ItemNode("constraint");
		node->pre = parent;
		node->itemVec = createItemNodes(node,"Object");
		nodeVec.emplace_back(node);

		m_filter->itemVec = nodeVec;
	}
	else if (s_name == "Object") {
		std::vector<std::string> obNames = getObjects();
		for (auto name : obNames) {
			ItemNode* node = new ItemNode(name.c_str());
			node->pre = parent;
			//node->itemVec = createItemNodes(node,"Characteristic");
			node->subVec = createItemNodes(node,"SubObject");
			nodeVec.emplace_back(node);
		}

		m_object->itemVec = nodeVec;
	}
	else if (s_name == "SubObject") {
		string subtmp = "";
		for (auto attri : m_curveIndex)
		{
			std::vector<std::string> sublist = split(attri.first, '/');
			if (subtmp == sublist.back())
			{
				continue;
			}
			subtmp = sublist.back();

			ItemNode* node = new ItemNode(subtmp.c_str());
			node->pre = parent;
			node->itemVec = createItemNodes(node, "Characteristic");
			nodeVec.emplace_back(node);
		}
	}
	else if (s_name == "Characteristic") {
		std::set<std::string> obNames = getCharacteristics();
		for (auto name : obNames) {
			ItemNode* node = new ItemNode(name.c_str());
			node->pre = parent;
			node->itemVec = createItemNodes(node, "Component");
			nodeVec.emplace_back(node);
		}

		m_character->itemVec = nodeVec;

	}
	else if (s_name == "Component") {
		ItemNode* nodeX = new ItemNode("x");
		ItemNode* nodeY = new ItemNode("y");
		ItemNode* nodeZ = new ItemNode("z");
		nodeX->pre = parent;
		nodeY->pre = parent;
		nodeZ->pre = parent;
		nodeVec.emplace_back(nodeX);
		nodeVec.emplace_back(nodeY);
		nodeVec.emplace_back(nodeZ);

		m_component->itemVec = nodeVec;
	}
	else {
		// TODO
	}

	return nodeVec;
}

H5Item* Hdf5Parser::getH5Item()
{
	ItemNode* head = nullptr;
	m_filter = new H5Item("Filter");
	m_object = new H5Item("Object");
	m_character = new H5Item("Characteristic");
	m_component = new H5Item("Component");

	m_filter->next = m_object;
	m_object->next = m_character;
	m_character->next = m_component;
	m_component->next = nullptr;

	m_filter->itemVec = createItemNodes(head, m_filter->name.c_str());

	return m_filter;
}

std::vector<double> Hdf5Parser::getCurveDateSet(const ItemNode* itemNode)
{
	std::vector<double> result;

	ItemNode* node = nullptr;
	if (nullptr == itemNode) {
		return std::vector<double>();
	}

	std::string component = itemNode->name;
	node = itemNode->pre;
	if (nullptr == node)
	{
		return result;
	}
	std::string character = node->name;

	node = node->pre;
	if (nullptr == node)
	{
		return result;
	}
	std::string subObject = node->name;

	node = node->pre;
	if (nullptr == node)
	{
		return result;
	}
	std::string object = node->name;

	std::string key = "/Curve/Index/";
	key.append(object);
	key.append("/Markers/");
	key.append(subObject);

	int index = 0;
	for (int i = 0; i < m_curveAttri.size(); i++)
	{
		if (strstr(m_curveAttri[i].c_str(), character.c_str()))
		{
			index = i;
			break;
		}
	}

	if (m_curveIndex.find(key) == m_curveIndex.end())
	{
		return std::vector<double>();
	}

	int queryIndex = m_curveIndex.at(key)[index];

	std::string baseKey = "/Curve/";
	baseKey.append(to_string(queryIndex));

	return m_curveBaseData.at(baseKey);

}

std::vector<double> Hdf5Parser::getTimeStamps()
{
	return m_timeStamps;
}

DouMap Hdf5Parser::getAnimationBaseData()
{
	return m_animationBaseData;
}

IntMap Hdf5Parser::getAnimationIndex()
{
	IntMap resIndex;
	for (auto idx : m_animationIndex)
	{
		std::vector<std::string> idxVec = split(idx.first,'/');
		std::string newKey;
		for (int i = 0; i< idxVec.size() - 1; i++)
		{
			if (idxVec[i] == "Index")
			{
				newKey = idxVec[i + 1];
			}
		}

		std::vector<int> temp;
		for (int i = 0;i<idx.second.size();i++)
		{
			temp.emplace_back(idx.second.at(i));
		}

		resIndex.insert(std::pair<std::string, std::vector<int>>(newKey, temp));
	}

	return resIndex;
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
		char buffname[OBJ_NAME_LEN] = { 0 };
		dataset.getObjName(buffname, OBJ_NAME_LEN);

		int dimNum = dims_size[0];
		hsize_t start[1] = { 0 };
		hsize_t count[1] = { static_cast<hsize_t>(dimNum) };

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
			m_curveBaseData.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}
		else if (strstr(buffname, "/Curve/Index/"))
		{
			m_curveIndex.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}


		if (strstr(buffname, "/Animation/") && !strstr(buffname, "/Animation/Index/"))
		{
			std::string tempBuffName = buffname;
			//tempBuffName = tempBuffName.substr(11);
			tempBuffName = split(tempBuffName, '/').back();

			m_animationBaseData.insert(std::pair<int, std::vector<double>>(atoi(tempBuffName.c_str()), temp));
		} else if (strstr(buffname, "/Animation/Index/"))
		{
			m_animationIndex.insert(std::pair<std::string, std::vector<double>>(buffname, temp));
		}

		if (strstr(buffname,"/TimeStamps"))
		{
			m_timeStamps = temp;
		}
		
	}

	if (rank == 2)	// two dims dataSet
	{
		int row = dims_size[0];
		int col = dims_size[1];
		int dim = row * col;
		hsize_t start[2] = { 0 };
		hsize_t count[2] = { static_cast<hsize_t>(row),static_cast<hsize_t>(col) };

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

ULLONG Hdf5Parser::getMemSize()
{
	return m_memSize;
}

void Hdf5Parser::clearH5Data()
{
	// if the value exceeds a certain size, clear related vector


}

