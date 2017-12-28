#ifndef _OBJREADER_H_
#define _OBJREADER_H_
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
//  [12/23/2017 ysl]
//---------------------------------
//Only polygon is supportted in the ObjReader class
// Single Line
// v(3): vertex
// vt(2): texture coordinate
// vn(3): vertex normal
// p(1): point
// l(2): line
// f():

template<typename T>
using Point3d = std::tuple<T, T, T>;

template<typename T>
using Point2d = std::tuple<T, T>;

template<typename T>
using Vector3d = std::tuple<T, T, T>;


class Model
{
public:
	Model()noexcept;
	Model(const std::string  & fileName)noexcept;
	bool load(const std::string & fileName);
	bool isLoaded()const;

	std::size_t getVertexCount()const { return m_vertices.size(); }
	std::size_t getNormalCount()const { return m_normals.size(); }
	std::size_t getTextureCoordCount()const { return m_textures.size(); }
	std::size_t getFacesCount()const { return m_facesIndices.size(); }

	const std::vector<Point3d<float>> & getVertices()const { return m_vertices; }
	const std::vector<std::vector<int>> & getFaceIndices()const { return m_facesIndices; }
	const std::vector<Vector3d<float>> & getNormals()const { return m_normals; }

	void translateX(float x);
	void translateY(float y);
	void translateZ(float z);
	void scaleX(float s);
	void scaleY(float s);
	void scaleZ(float s);

	void translate(float x, float y, float z);
	void scale(float sx, float sy, float sz);


	//std::size_t getNormalsCount()const { return m_normals.size(); }

	float * getVerticesFlatArray()const { return m_verticesFlatArray; }
	//float * getNormalsSFlatArray()const { return m_normalsFlatArray; }
	//float * getTexturesFlayArray()const { return m_texturesFlatArray; }
	int * getFacesIndicesFlatArray()const { return m_facesIndicesFlatArray; }
	//int * getNormalsIndicesFlayArray()const { return m_normalsIndicesFlatArray; }
	//int * getTextureIndicesFlayArray()const { return m_textureIndicesFlatArray; }

	virtual ~Model();

private:
	void _init();
	void _destroy();
private:
	void _toVerticesFlatArray();
	void _toNormalsSFlatArray();
	void _toTexturesFlayArray();
	void _toFacesIndicesFlatArray();
	void _toFormalsIndicesFlayArray();
	void _toTextureIndicesFlayArray();


	bool m_loaded;

	std::vector<std::tuple<float, float, float>> m_normals;
	std::vector<std::tuple<float, float, float>> m_vertices;
	std::vector<std::tuple<float, float>> m_textures;
	std::vector<std::vector<int>> m_facesIndices;
	std::vector<std::vector<int>> m_normalsIndices;
	std::vector<std::vector<int>> m_texturesIndices;




	float * m_verticesFlatArray;
	float * m_normalsFlatArray;
	float * m_texturesFlatArray;
	int * m_facesIndicesFlatArray;
	int * m_normalsIndicesFlatArray;
	int * m_textureIndicesFlatArray;
};
#endif
