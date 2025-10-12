/******************************************************************************
Helper for drawing simple but general 3D objects with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <stdio.h>
#include <vgl/DrawGL3D.h>
#include <GL/glu.h>

#include <glm/ext.hpp>

namespace vgl {

///////////////////////////////////////////////////////////////////////////////
// simple primitive with GLU
///////////////////////////////////////////////////////////////////////////////
void drawAxes(double length)
{
	float materialR[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float materialG[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	float materialB[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float materialW[4] = { 0.8f, 0.8f, 0.8f, 0.0f }; // default diffuse

	static bool init = false;
	static GLUquadricObj *quadratic;
	if (!init) {
		quadratic = gluNewQuadric();
		init = true;
	}

	double radius = length*0.05;

	// Z-axis
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialB);
	gluCylinder(quadratic, radius, radius, length*0.75, 16, 16);
	glPushMatrix();
	glTranslatef(0, 0, length*0.75f);
	gluCylinder(quadratic, radius*2.0, 0.0f, length*0.25, 16, 16);
	glPopMatrix();

	// Y-axis
	glPushMatrix();
	glRotatef(-90.0f, 1, 0, 0);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialG);
	gluCylinder(quadratic, radius, radius, length*0.75, 16, 16);
	glPushMatrix();
	glTranslatef(0, 0, length*0.75);
	gluCylinder(quadratic, radius*2.0, 0.0f, length*0.25, 16, 16);
	glPopMatrix();

	glPopMatrix();

	// X-axis
	glPushMatrix();
	glRotatef(90.0f, 0, 1, 0);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialR);
	gluCylinder(quadratic, radius, radius, length*0.75, 16, 16);
	glPushMatrix();
	glTranslatef(0, 0, length*0.75);
	gluCylinder(quadratic, radius*2.0, 0.0, length*0.25, 16, 16);
	glPopMatrix();

	glPopMatrix();

	// set material color to default (light gray)
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialW);
}

void drawSphere(float radius, glm::vec4 color)
{
	static bool init = false;
	static GLUquadricObj *quadratic;
	if (!init) {
		quadratic = gluNewQuadric();
		init = true;
	}

	glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(color));
	gluSphere(quadratic, radius, 16, 16);
}


///////////////////////////////////////////////////////////////////////////////
// simple primitive (GL_QUADS)
///////////////////////////////////////////////////////////////////////////////
void drawCube(float radius)
{
	glBegin(GL_QUADS);

	// quad 1: "front"
	glNormal3f( 0.0f,  0.0f, +1.0f);
	glVertex3f(-0.5f, -0.5f, +0.5f);
	glVertex3f(+0.5f, -0.5f, +0.5f);
	glVertex3f(+0.5f, +0.5f, +0.5f);
	glVertex3f(-0.5f, +0.5f, +0.5f);

	// quad 2: "back"
	glNormal3f( 0.0f,  0.0f, -1.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, +0.5f, -0.5f);
	glVertex3f(+0.5f, +0.5f, -0.5f);
	glVertex3f(+0.5f, -0.5f, -0.5f);

	// quad 3: "left"
	glNormal3f(-1.0f,  0.0f,  0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, +0.5f);
	glVertex3f(-0.5f, +0.5f, +0.5f);
	glVertex3f(-0.5f, +0.5f, -0.5f);
	
	// quad 4: "right"
	glNormal3f(+1.0f,  0.0f,  0.0f);
	glVertex3f(+0.5f, -0.5f, -0.5f);
	glVertex3f(+0.5f, +0.5f, -0.5f);
	glVertex3f(+0.5f, +0.5f, +0.5f);
	glVertex3f(+0.5f, -0.5f, +0.5f);

	// quad 5: "bottom"
	glNormal3f( 0.0f, -1.0f,  0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(+0.5f, -0.5f, -0.5f);
	glVertex3f(+0.5f, -0.5f, +0.5f);
	glVertex3f(-0.5f, -0.5f, +0.5f);

	// quad 6: "top"
	glNormal3f( 0.0f, +1.0f,  0.0f);
	glVertex3f(-0.5f, +0.5f, -0.5f);
	glVertex3f(-0.5f, +0.5f, +0.5f);
	glVertex3f(+0.5f, +0.5f, +0.5f);
	glVertex3f(+0.5f, +0.5f, -0.5f);

	glEnd();
}


///////////////////////////////////////////////////////////////////////////////
// simple primitive (GL_LINES)
///////////////////////////////////////////////////////////////////////////////
void drawGridXY(float length, int step)
{
	// draw grid on XZ plane
	glBegin(GL_LINES);

	float step_x = length / (float)step;
	float step_z = length / (float)step;

	// parallel with Y-axis
	for (int i = 0; i <= step; i++) {
		float x = -length * 0.5f + step_x * i;
		float y = +length * 0.5f;
		glVertex3f(x, -y, 0.0f);
		glVertex3f(x, +y, 0.0f);
	}

	// parallel with X-axis
	for (int i = 0; i <= step; i++) {
		float x = +length * 0.5f;
		float y = -length * 0.5f + step_z * i;
		glVertex3f(-x, y, 0.0f);
		glVertex3f(+x, y, 0.0f);
	}

	glEnd();
}

void drawGridXZ(float length, int step)
{
	// draw grid on XZ plane
	glBegin(GL_LINES);

	float step_x = length / (float)step;
	float step_z = length / (float)step;

	// parallel with Z-axis
	for (int i = 0; i <= step; i++) {
		float x = -length * 0.5f + step_x * i;
		float z = length * 0.5f;
		glVertex3f(x, 0, -z);
		glVertex3f(x, 0,  z);
	}

	// parallel with X-axis
	for (int i = 0; i <= step; i++) {
		float x = length * 0.5f;
		float z = -length * 0.5f + step_z * i;
		glVertex3f(-x, 0, z);
		glVertex3f( x, 0, z);
	}

	glEnd();
}

void drawAABB(glm::vec3 minAB, glm::vec3 maxAB)
{
	float l = minAB.x;
	float r = maxAB.x;
	float t = maxAB.y;
	float b = minAB.y;
	float n = minAB.z;
	float f = maxAB.z;

	// draw 12 lines
	glBegin(GL_LINES);

	// near to Z
	glVertex3f(l, t, n); glVertex3f(r, t, n);
	glVertex3f(r, t, n); glVertex3f(r, b, n);
	glVertex3f(r, b, n); glVertex3f(l, b, n);
	glVertex3f(l, b, n); glVertex3f(l, t, n);

	// far to Z
	glVertex3f(l, t, f); glVertex3f(r, t, f);
	glVertex3f(r, t, f); glVertex3f(r, b, f);
	glVertex3f(r, b, f); glVertex3f(l, b, f);
	glVertex3f(l, b, f); glVertex3f(l, t, f);

	// between near-far
	glVertex3f(l, t, n); glVertex3f(l, t, f);
	glVertex3f(r, t, n); glVertex3f(r, t, f);
	glVertex3f(l, b, n); glVertex3f(l, b, f);
	glVertex3f(r, b, n); glVertex3f(r, b, f);

	glEnd();
}


///////////////////////////////////////////////////////////////////////////////
// camera frustum
///////////////////////////////////////////////////////////////////////////////
inline void drawPerspCamera(glm::mat4 proj)
{
	float z_n = (proj[2][2] - 1.0f) / proj[3][2]; z_n = 1.0f / z_n; // near
	float z_f = (proj[2][2] + 1.0f) / proj[3][2]; z_f = 1.0f / z_f; // far

	bool is_inf = glm::isinf(z_f);
	if (is_inf) z_f = 1e8f; // large enough value

	// extract screen boundary from matrix
	float l = (proj[2][0] - 1.0f) / proj[0][0]; // left
	float r = (proj[2][0] + 1.0f) / proj[0][0]; // right (GL coordinates)
	float t = (proj[2][1] + 1.0f) / proj[1][1]; // top (GL coordinates)
	float b = (proj[2][1] - 1.0f) / proj[1][1]; // bottom 

	float l_n = z_n * l; float l_f = z_f * l;
	float r_n = z_n * r; float r_f = z_f * r;
	float t_n = z_n * t; float t_f = z_f * t;
	float b_n = z_n * b; float b_f = z_f * b;

	// inverted z due to NDC
	z_n = -z_n;
	z_f = -z_f;

	// draw 12 lines
	glBegin(GL_LINES);

	// near square
	glVertex3f(l_n, t_n, z_n); glVertex3f(r_n, t_n, z_n);
	glVertex3f(r_n, t_n, z_n); glVertex3f(r_n, b_n, z_n);
	glVertex3f(r_n, b_n, z_n); glVertex3f(l_n, b_n, z_n);
	glVertex3f(l_n, b_n, z_n); glVertex3f(l_n, t_n, z_n);

	// far square
	if (!is_inf) {
		glVertex3f(l_f, t_f, z_f); glVertex3f(r_f, t_f, z_f);
		glVertex3f(r_f, t_f, z_f); glVertex3f(r_f, b_f, z_f);
		glVertex3f(r_f, b_f, z_f); glVertex3f(l_f, b_f, z_f);
		glVertex3f(l_f, b_f, z_f); glVertex3f(l_f, t_f, z_f);
	}

	// lines between squares
	glVertex3f(l_n, t_n, z_n); glVertex3f(l_f, t_f, z_f);
	glVertex3f(r_n, t_n, z_n); glVertex3f(r_f, t_f, z_f);
	glVertex3f(l_n, b_n, z_n); glVertex3f(l_f, b_f, z_f);
	glVertex3f(r_n, b_n, z_n); glVertex3f(r_f, b_f, z_f);

	glEnd();
}
inline void drawOrthoCamera(glm::mat4 proj)
{
	// extract clipping plane from matrix
	float z_n = -(1.0f + proj[3][2]) / proj[2][2]; // near
	float z_f =  (1.0f - proj[3][2]) / proj[2][2]; // far

	// extract screen boundary from matrix
	float l = -(1.0f + proj[3][0]) / proj[0][0]; // left
	float r =  (1.0f - proj[3][0]) / proj[0][0]; // right (GL coordinates)
	float t =  (1.0f - proj[3][1]) / proj[1][1]; // top (GL coordinates)
	float b = -(1.0f + proj[3][1]) / proj[1][1]; // bottom 

	// draw 12 lines
	glBegin(GL_LINES);

	// near square
	glVertex3f(l, t, z_n); glVertex3f(r, t, z_n);
	glVertex3f(r, t, z_n); glVertex3f(r, b, z_n);
	glVertex3f(r, b, z_n); glVertex3f(l, b, z_n);
	glVertex3f(l, b, z_n); glVertex3f(l, t, z_n);

	// far square
	glVertex3f(l, t, z_f); glVertex3f(r, t, z_f);
	glVertex3f(r, t, z_f); glVertex3f(r, b, z_f);
	glVertex3f(r, b, z_f); glVertex3f(l, b, z_f);
	glVertex3f(l, b, z_f); glVertex3f(l, t, z_f);

	// lines between squares
	glVertex3f(l, t, z_n); glVertex3f(l, t, z_f);
	glVertex3f(r, t, z_n); glVertex3f(r, t, z_f);
	glVertex3f(l, b, z_n); glVertex3f(l, b, z_f);
	glVertex3f(r, b, z_n); glVertex3f(r, b, z_f);

	glEnd();
}

void drawCameraFrustum(glm::mat4 proj)
{
	// orthogonal or perspective
	if (0.0f != proj[3][3]) {
		drawOrthoCamera(proj);
	}
	else {
		// infinity or not
		drawPerspCamera(proj);
	}
}


///////////////////////////////////////////////////////////////////////////////
// 3D data
///////////////////////////////////////////////////////////////////////////////
void drawPointCloud(const std::vector<glm::vec3>& points)
{
	if (0 == points.size()) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &points[0]);
	glDrawArrays(GL_POINTS, 0, (GLsizei)points.size());
	glDisableClientState(GL_VERTEX_ARRAY);
}
void drawPointCloud(const std::vector<glm::vec3>& points, const std::vector<glm::u8vec3>& colors)
{
	if (0 == colors.size()) return;

	if (points.size() != colors.size()) {
		fprintf(stderr, "ERROR: the size of vertex and color is not matched.\n");
		return;
	}

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0]);
	drawPointCloud(points);
	glDisableClientState(GL_COLOR_ARRAY);
}

void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::uint>& F)
{
	if (0 == V.size() || 0 == F.size()) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &V[0]);
	glDrawElements(GL_TRIANGLES, F.size(), GL_UNSIGNED_INT, &F[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
}
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::uint>& F)
{
	if (0 == N.size()) return;

	if (V.size() != N.size()) {
		fprintf(stderr, "ERROR: the size of vertex and normal is not matched.\n");
		return;
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, &N[0]);
	drawTriMesh(V, F);
	glDisableClientState(GL_NORMAL_ARRAY);
}
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F)
{
	if (0 == C.size()) return;

	if (V.size() != C.size()) {
		fprintf(stderr, "ERROR: the size of vertex and color is not matched.\n");
		return;
	}

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_UNSIGNED_BYTE, 0, &C[0]);
	drawTriMesh(V, F);
	glDisableClientState(GL_COLOR_ARRAY);
}
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F)
{
	if (0 == N.size() || 0 == C.size()) return;

	if (V.size() != N.size()) {
		fprintf(stderr, "ERROR: the size of vertex and normal is not matched.\n");
		return;
	}
	if (V.size() != C.size()) {
		fprintf(stderr, "ERROR: the size of vertex and color is not matched.\n");
		return;
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, &N[0]);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_UNSIGNED_BYTE, 0, &C[0]);
	drawTriMesh(V, F);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


///////////////////////////////////////////////////////////////////////////////
// lighting and material
///////////////////////////////////////////////////////////////////////////////
void setLight(GLenum lightNum, glm::vec4 lightPos,
	const glm::vec4 ambient, const glm::vec4 diffuse, const glm::vec4 specular)
{
	if (lightNum < GL_LIGHT0 || GL_LIGHT7 < lightNum) {
		fprintf(stderr, "ERROR: invalid light number.\n");
		return;
	}

	// configure and enable light source
	glLightfv(lightNum, GL_POSITION, glm::value_ptr(lightPos));
	glLightfv(lightNum, GL_AMBIENT , glm::value_ptr(ambient));
	glLightfv(lightNum, GL_DIFFUSE , glm::value_ptr(diffuse));
	glLightfv(lightNum, GL_SPECULAR, glm::value_ptr(specular));
	glEnable(lightNum);
}

void setMaterial(GLenum face, const glm::vec4 ambient, const glm::vec4 diffuse, const glm::vec4 specular, const glm::vec4 emission, const float shininess)
{
	glMaterialfv(face, GL_AMBIENT , glm::value_ptr(ambient));
	glMaterialfv(face, GL_DIFFUSE , glm::value_ptr(diffuse));
	glMaterialfv(face, GL_SPECULAR, glm::value_ptr(specular));
	glMaterialfv(face, GL_EMISSION, glm::value_ptr(emission));
	glMaterialf(face, GL_SHININESS, shininess);
}


///////////////////////////////////////////////////////////////////////////////
// convert normal to normalmap color
///////////////////////////////////////////////////////////////////////////////
inline glm::u8vec3 getNormalColor(const glm::vec3& normal)
{
	// coloring is based on "normal = (2*color)-1"
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
	glm::vec3 n = glm::normalize(normal);
	glm::vec3 c = 127.5f * (n + glm::vec3(1.0));
	return c;
}
std::vector<glm::u8vec3> GetNormalColors(const std::vector<glm::vec3>& normal)
{
	std::vector<glm::u8vec3> colors;
	for (glm::vec3 _n : normal) {
		colors.push_back(getNormalColor(_n));
	}

	return colors;
}

}
