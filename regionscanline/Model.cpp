#define _CRT_SECURE_NO_WARNINGS
#include "model.h"
Model::Model()noexcept:
m_loaded(false),
m_verticesFlatArray(nullptr),
m_normalsFlatArray(nullptr),
m_texturesFlatArray(nullptr),
m_facesIndicesFlatArray(nullptr),
m_normalsIndicesFlatArray(nullptr),
m_textureIndicesFlatArray(nullptr)
{
}

Model::Model(const std::string & fileName) noexcept:Model()
{
	m_loaded = load(fileName);
}

bool Model::load(const std::string & fileName)
{
	_init();

	bool ok = true;

	std::ifstream fileIn(fileName);
	if (fileIn.is_open() == false) {
		std::cerr << "Can not open this file\n";
		return false;
	}
	std::string line;
	char l;
	while (getline(fileIn, line)) {
		//std::cout << line << std::endl;
		line = line.substr(0, line.find_first_of('#'));
		//line = line.substr(line.find_first_not_of(' ')+1);
		std::stringstream ss(line);
		std::string objKeyword;
		ss >> objKeyword;
		if (objKeyword == "v") {
			float x, y, z;
			ss >> x >> y >> z;
			m_vertices.emplace_back(x, y, z);
		}
		else if (objKeyword == "vt") {
			float x, y;
			ss >> x >> y;
			m_textures.emplace_back(x, y);
		}
		else if (objKeyword == "vn") {
			float x, y, z;
			ss >> x >> y >> z;
			m_normals.emplace_back(x, y, z);
		}
		else if (objKeyword == "f") {
			std::string str;
			m_facesIndices.push_back(std::vector<int>());
			while (ss >> str) {
				int vid, vtid, vnid;
				sscanf(str.c_str(), "%d/%d/%d", &vid, &vtid, &vnid);
				m_facesIndices.back().emplace_back(vid-1);
			}
		}
		ss.clear();
	}
	//_toVerticesFlatArray();
	//_toFacesIndicesFlatArray();
	return (ok);
}

bool Model::isLoaded() const
{
	return m_loaded;
}

void Model::translateX(float x)
{

}

void Model::translateY(float y)
{

}

void Model::translateZ(float z)
{
}

void Model::scaleX(float s)
{
}

void Model::scaleY(float s)
{
}

void Model::scaleZ(float s)
{
}

void Model::translate(float x, float y, float z)
{
}

void Model::scale(float sx, float sy, float sz)
{
}


void Model::_toVerticesFlatArray()
{
	//std::size_t elemCount = m_vertices.size() * 3;
	//m_verticesFlatArray = new float[elemCount];
	//std::size_t count = m_vertices.size();
	//for (std::size_t i = 0; i < count; i++) {
	//	m_verticesFlatArray[3 * i] = std::get<0>(m_vertices[i]);
	//	m_verticesFlatArray[3 * i + 1] = std::get<1>(m_vertices[i]);
	//	m_verticesFlatArray[3 * i + 2] = std::get<2>(m_vertices[i]);
	//}

}

void Model::_toNormalsSFlatArray()
{

}

void Model::_toTexturesFlayArray()
{

}

void Model::_toFacesIndicesFlatArray()
{
	//std::size_t elemCount = m_facesIndices.size() * 3;
	//m_facesIndicesFlatArray = new int[elemCount];
	//std::size_t count = m_facesIndices.size();
	//for (std::size_t i = 0; i < count; i++) {
	//	m_facesIndicesFlatArray[3 * i] = m_facesIndices[i][0];
	//	m_facesIndicesFlatArray[3 * i + 1] = m_facesIndices[i][1];
	//	m_facesIndicesFlatArray[3 * i + 2] = m_facesIndices[i][2];
	//}
}

void Model::_toFormalsIndicesFlayArray()
{

}

void Model::_toTextureIndicesFlayArray()
{
}

Model::~Model()
{
	_destroy();
}

void Model::_init()
{
	_destroy();

	m_normals.clear();
	m_vertices.clear();
	m_textures.clear();
	m_facesIndices.clear();
	m_normalsIndices.clear();
	m_texturesIndices.clear();

	m_loaded = false;
}

void Model::_destroy()
{
	delete[] m_verticesFlatArray;
	delete[] m_verticesFlatArray;
	delete[] m_normalsFlatArray;
	delete[] m_texturesFlatArray;
	delete[] m_facesIndicesFlatArray;
	delete[] m_normalsIndicesFlatArray;
	delete[] m_textureIndicesFlatArray;
}
