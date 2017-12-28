#include <gl/freeglut.h>
#include <gl/GL.h>
#include <string>
#include <list>
#include <cassert>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "FrameBuffer.h"
#include "Model.h"

//-------------------------Data Structure----------------------------
using myreal = float;
using color = std::tuple<unsigned char, unsigned char, unsigned char>;
using plane = std::tuple<myreal, myreal, myreal, myreal>;
struct polygon_info {
	plane p_3d;
	int scanlines_2d;
	bool flag;
	color cl;
	int maxy;
	polygon_info(const plane & p = plane(),
		int scan = 0,
		bool f = false,
		color c = color{ 255,255,255 },
		int my = 0
	) :p_3d(p),
		scanlines_2d(scan),
		flag(f),
		cl(c), maxy(my) {}

	void invert_flag() {
		if (flag == true)flag = false;
		else flag = true;
	}
};

struct edge_info {
	int polyid;
	int x_2d;
	float dx_2d;
	int scanlines_2d;
	int maxy;

	edge_info(int pid = 0,
		int x = 0,
		float dx = 0,
		int scan = 0,
		int my = 0
	) :polyid(pid),
		x_2d(x),
		dx_2d(dx),
		scanlines_2d(scan),
		maxy(my)
	{}
};
std::vector<Point3d<float>> vertices3d;		//model coords
std::vector<Point2d<int>> projection_coords;// vertex index -> coord

std::vector<std::vector<int>> vid;	//polyid -> vid
std::vector<glm::vec3> polygons_normal;
std::vector<polygon_info> polygons_info;
std::vector<edge_info> edges_info;

std::vector<std::vector<int>> edges_table;
std::vector<std::vector<int>> polygons_table;

//-------------------------Helper Functions-----------------------
inline glm::vec3 tuple2vec3(const std::tuple<float, float, float> & p) { return glm::vec3(std::get<0>(p), std::get<1>(p), std::get<2>(p)); }
inline std::tuple<float, float, float> vec3tuple(const glm::vec3 & vec3) { return { vec3.x,vec3.y,vec3.z }; }
float getz(const plane & p, float x, float y) {
	float a = std::get<0>(p);
	float b = std::get<1>(p);
	float c = std::get<2>(p);
	float d = std::get<3>(p);
	return c == 0 ? 99999999 : (-d - a * x - b * y) / c;
}


//-------------------------Control Parameters----------------------
extern int window_width = 1280, window_height = 800;

FrameBuffer frameBuffer(window_width, window_height);
std::string fileName = "C:/Users/ysl/Desktop/scan-line-Z-buffer-master/model/bunny.obj";
//std::string fileName = "C:/Users/ysl/Desktop/test.obj";
Model model(fileName);


glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;
glm::vec3 eye;
glm::vec3 center;
glm::vec3 up;
int background_poly_id = -1;
int left_edge_id = -1;
int right_edge_id = -1;


//------------------------Routines--------------------------
void init_camera_parameter() {
	eye = glm::vec3(0, 0, 5);
	center = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
}
void init_vertices3d_and_vid_from_model() {
	vertices3d = model.getVertices();
	vid = model.getFaceIndices();
}
void init_model_matrix() {
	modelMatrix = glm::scale(modelMatrix, glm::vec3(600, 600, 600));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(1, 0.5, 0));
}
void update_view_matrix() {
	viewMatrix = glm::lookAt(eye, center, up);
}
void update_projection_matrix() {
	projectionMatrix = glm::perspective(glm::radians(45.f), 1024.f / 768.f, 0.1f, 1000.f);
}
void calculate_normal_vector() {
	polygons_normal.clear();
	glm::mat3 normal_matrix = glm::mat3(transpose(inverse(modelMatrix)));
	for (int i = 0; i < vid.size(); i++) {
		int id1 = vid[i][0], id2 = vid[i][1], id3 = vid[i][2];
		glm::vec3 v1 = tuple2vec3(vertices3d[id2]) - tuple2vec3(vertices3d[id1]);
		glm::vec3 v2 = tuple2vec3(vertices3d[id3]) - tuple2vec3(vertices3d[id2]);
		glm::vec3 nor = glm::normalize(normal_matrix * glm::normalize(glm::cross(v1, v2)));
		polygons_normal.push_back(nor);
	}
}
void projection()
{
	//-----------------------Computer projection coords on some plane
	std::vector<Point2d<int>> vertices2d;
	std::vector<glm::vec4> coords;
	for (const auto & p3 : vertices3d) {
		coords.push_back(glm::vec4(
			std::get<0>(p3),
			std::get<1>(p3),
			std::get<2>(p3),
			1.0
		));
	}
	glm::mat4 trans = projectionMatrix * viewMatrix*modelMatrix;
	for (const auto & p3 : coords) {
		glm::vec4 proj_coord = viewMatrix * modelMatrix * p3;
		//clip
		assert(proj_coord.x >= 0 && proj_coord.x < window_width && proj_coord.y >= 0 && proj_coord.y < window_height);
		vertices2d.push_back({ proj_coord.x,proj_coord.y });
	}
	projection_coords = vertices2d;

}
void create_polygons_and_edges_info() {
	//-----------------------------polygon info-------------------------------------
	glm::mat3 normal_matrix = glm::mat3(transpose(inverse(modelMatrix)));
	glm::mat4 viewTrans = viewMatrix * modelMatrix;
	//TODO:
	polygons_info.clear();
	for (int i = 0; i < vid.size(); i++) {
		int maxy = -1;
		int miny = 99999999;
		assert(vid[i].size() >= 3);
		for (int j = 0; j < vid[i].size(); j++) { //for every vertex on the polygon 
			int id = vid[i][j];
			maxy = std::max(maxy, std::get<1>(projection_coords[id]));
			miny = std::min(miny, std::get<1>(projection_coords[id]));

		} // find max y and min y on the plane for the polygon
		  //scanlines number the poly covers
		int scanlines = maxy - miny;
		assert(scanlines >= 0);
		//select a color for the polygon according to normal vector
		glm::vec3 eyeDir = glm::normalize(eye - center);
		glm::vec3 nor = polygons_normal[i];
		float factor = glm::max(0.f, glm::dot(nor, eyeDir));
		color cl = { 255 * factor,255 * factor,255 * factor };
		//calculating plane equation in the view frame
		int id1 = vid[i][0], id2 = vid[i][1], id3 = vid[i][2];  //frist three coords id
		glm::vec3 p1_in_view_frame = viewTrans * glm::vec4(tuple2vec3(vertices3d[id1]), 1);
		glm::vec3 p2_in_view_frame = viewTrans * glm::vec4(tuple2vec3(vertices3d[id2]), 1);
		glm::vec3 p3_in_view_frame = viewTrans * glm::vec4(tuple2vec3(vertices3d[id3]), 1);
		glm::vec3 nor_in_view_frame = glm::normalize(glm::cross(p2_in_view_frame - p1_in_view_frame, p3_in_view_frame - p2_in_view_frame));
		float a = nor_in_view_frame.x;
		float b = nor_in_view_frame.y;
		float c = nor_in_view_frame.z;
		float d = glm::dot(nor_in_view_frame, p2_in_view_frame);
		polygons_info.emplace_back(plane(a, b, c, -d),
			scanlines,
			false,
			cl,
			maxy);
	}
	//Add background as a polygon
	polygons_info.emplace_back(plane(0.f, 0.f, 1.0f, 999999.f),
		window_height,
		false,
		color(0, 0, 0),
		window_height - 1);
	background_poly_id = polygons_info.size() - 1;
	//-----------------------------polygon info end-------------------------------------
	//-----------------------------edge info-------------------------------------
	int vcount = vertices3d.size();
	edges_info.clear();
	for (int i = 0; i < vid.size(); i++) {
		if (vid[i].size() <= 1)continue;		//a plane consists of only one vertex, wrong
		int polyid = i;
		for (int j = 0; j < vid[i].size(); j++) {		//for each line of the polygon
			int id0 = -1, id1 = -1;
			if (j < vid[i].size() - 1)
			{
				id0 = vid[i][j], id1 = vid[i][j + 1];
			}
			else {
				id0 = vid[i][j], id1 = vid[i][0];
			}
			float x0 = std::get<0>(projection_coords[id0]), y0 = std::get<1>(projection_coords[id0]);
			float x1 = std::get<0>(projection_coords[id1]), y1 = std::get<1>(projection_coords[id1]);
			int maxyid = -1;
			int maxy = -1;
			if (y1 > y0) {
				maxy = y1;
				maxyid = id1;
			}
			else {
				maxy = y0;
				maxyid = id0;
			}
			int miny = y1 < y0 ? y1 : y0;
			int x = std::get<0>(projection_coords[maxyid]);
			int scanlines = maxy - miny;
			assert(scanlines >= 0);
			float dx = 0;
			if (y1 != y0) {
				dx = (x1 - x0) / int(y1 - y0);
			}
			else {		//a horizontal edge,override it
				continue;
			}
			//cal z
			edges_info.emplace_back(polyid,
				x,
				dx,
				scanlines,
				maxy);
		}
	}
	//The following tow edges are the left and right edge of the screen
	edges_info.emplace_back(background_poly_id,
		0,
		0,
		window_height,
		window_height - 1);
	left_edge_id = edges_info.size() - 1;
	edges_info.emplace_back(background_poly_id,
		window_width - 1,
		0,
		window_height,
		window_height - 1);
	right_edge_id = edges_info.size() - 1;
}
void create_edge_table_and_polygon_table() {
	std::vector<int> sorted_polyid;		//polyid
	for (int i = 0; i < polygons_info.size(); i++) {
		sorted_polyid.push_back(i);
	}
	std::sort(sorted_polyid.begin(),
		sorted_polyid.end(),
		[](int x, int y)
	{return polygons_info[x].maxy < polygons_info[y].maxy; });
	std::vector<int> sorted_edgeid;
	for (int i = 0; i < edges_info.size(); i++) {
		sorted_edgeid.push_back(i);
	}
	std::sort(sorted_edgeid.begin(),
		sorted_edgeid.end(),
		[](int x, int y) {return edges_info[x].maxy < edges_info[y].maxy; });

	edges_table = std::vector<std::vector<int>>(window_height);
	polygons_table = std::vector<std::vector<int>>(window_height);
	for (int i : sorted_polyid) {
		int height = polygons_info[i].maxy;
		assert(height >= 0 && height <= window_height);
		polygons_table[height].push_back(i);			//order requried ? 
	}
	for (int i : sorted_edgeid) {
		int height = edges_info[i].maxy;
		assert(height >= 0 && height <= window_height);
		edges_table[height].push_back(i);
	}
}
void region_scanline() {
	
	std::list<int> active_edges_list;
	std::list<int> active_polygons_list;
	std::vector<float> active_edges_intersection_x(edges_info.size());
	std::vector<int> active_edges_left_scanlines(edges_info.size());
	std::vector<int> active_polygons_left_scanlines(polygons_info.size());
	auto lambda_pred =
		[&active_edges_intersection_x]
	(int x, int y)
	{return active_edges_intersection_x[x] < active_edges_intersection_x[y]; };

	for (int height = window_height - 1; height >= 0; height--) {			//for every scanline from top to bottom
		for (int pid : polygons_table[height]) {
			active_polygons_list.push_back(pid); //if new polygons enter current scanline
			active_polygons_left_scanlines[pid] = polygons_info[pid].scanlines_2d;
		}
		if (edges_table[height].empty() == false) {
			for (int eid : edges_table[height]) {
				active_edges_list.push_back(eid);	//if new polygons enter current scanline
				active_edges_intersection_x[eid] = edges_info[eid].x_2d;		//initializing x of newly inserted edge's
				active_edges_left_scanlines[eid] = edges_info[eid].scanlines_2d;
			}
			active_edges_list.sort(lambda_pred);//Sorting by x of edge's
		}
		for (auto itr_first = active_edges_list.begin();
			itr_first != active_edges_list.end();
			itr_first++) {
			int the_first_pid = edges_info[*itr_first].polyid;
			polygons_info[the_first_pid].invert_flag();// set 'in' if it's 'out', and vice versa
			auto itr_second = std::next(itr_first);
			if (itr_second == active_edges_list.end())break;
			int the_second_pid = edges_info[*itr_second].polyid;
			std::vector<int> in_polygons;
			for (auto pid : active_polygons_list) {		//find the number of active polygon with 'in' flag
				if (polygons_info[pid].flag == true) {
					in_polygons.push_back(pid);
				}
			}
			Color24 color{255,255,255};
			int startx = (active_edges_intersection_x[*itr_first]);
			int endx = (active_edges_intersection_x[*itr_second]);
			int midx = (startx + endx) / 2;
			int desired_pid = -1;
			myreal maxz = -99999999;
			if (in_polygons.size() == 1) {			//only using the color of the polygon pointed by desired_pid
				color = polygons_info[in_polygons[0]].cl;
			}
			else if (in_polygons.size() > 1){//or comparing the zdepth of all active polygons at the middle point of this region scanline to determine which color to be used
				for (int pid : in_polygons) {
					myreal depth = getz(polygons_info[pid].p_3d, midx, height);
					if (maxz < depth) {
						maxz = depth;
						desired_pid = pid;
					}
				}
				color = polygons_info[desired_pid].cl;
			}
			//draw scanline
			frameBuffer.setHorizontialLineColor24(height, startx, endx, color);
		}
		for (auto item : active_edges_list) {		//update x for the intersection between scanline and edge
			active_edges_intersection_x[item] -= edges_info[item].dx_2d;
		}
		//Sorting the list
		active_edges_list.sort(lambda_pred);
		//update left scanlines and remove active edge
		for (auto itr = active_edges_list.begin(); itr != active_edges_list.end();) {
			if (--active_edges_left_scanlines[*itr] == 0) {
				active_edges_list.erase(itr++);
			}
			else {
				++itr;
			}
		}
		//update left scanlines and remove active polygon
		for (auto itr = active_polygons_list.begin(); itr != active_polygons_list.end();) {
			if (--active_polygons_left_scanlines[*itr] == 0) {
				active_polygons_list.erase(itr++);
			}
			else {
				++itr;
			}
		}
	}
}
void load() {
	init_vertices3d_and_vid_from_model();
	init_camera_parameter();
	init_model_matrix();
	update_view_matrix();
	update_projection_matrix();
	calculate_normal_vector();
	projection();
	create_polygons_and_edges_info();
	create_edge_table_and_polygon_table();
	region_scanline();
}
void init()
{
	glClearColor(0, 0, 0, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, window_width, 0, window_height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);
	glDrawBuffer(GL_FRONT_AND_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos2i(0, 0);
	glDrawPixels(frameBuffer.width(), frameBuffer.height(),
		GL_RGB, GL_UNSIGNED_BYTE, frameBuffer.buffer());
	glutSwapBuffers();
}
float r = 0.f;
void special_keyboard_callback(int key,int x,int y) {
	glm::mat4 rot;
	
	if (key == GLUT_KEY_RIGHT) {
		r += 10;
		rot = glm::rotate(rot, glm::radians(r), glm::vec3(0, 1, 0));
		eye = rot * glm::vec4(0, 0, 5, 1);
		update_view_matrix();
		projection();
		create_polygons_and_edges_info();
		create_edge_table_and_polygon_table();
		region_scanline();
	}
	else if (key == GLUT_KEY_LEFT) {
		r -= 10;
		rot = glm::rotate(rot, glm::radians(r), glm::vec3(0, 1, 0));
		eye = rot * glm::vec4(0, 0, 5, 1);
		update_view_matrix();
		projection();	
		create_polygons_and_edges_info();
		create_edge_table_and_polygon_table();
		region_scanline();
	}
}
int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(250, 100);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutSpecialFunc(special_keyboard_callback);
	glutIdleFunc(display);
	load();
	glutMainLoop();
	return 0;
}