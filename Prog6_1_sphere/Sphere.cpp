#include <cmath>
#include <vector>
#include <iostream>
#include <glm\glm.hpp>
#include "Sphere.h"
using namespace std;

Sphere::Sphere() {
	init(48);
}

Sphere::Sphere(int prec) {
	init(prec);
}
//���Ƕ�ֵת���ɻ���ֵ
float Sphere::toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

void Sphere::init(int prec) {
	numVertices = (prec + 1) * (prec + 1);
	numIndices = prec * prec * 6;
	for (int i = 0; i < numVertices; i++) { vertices.push_back(glm::vec3()); }
	for (int i = 0; i < numVertices; i++) { texCoords.push_back(glm::vec2()); }
	for (int i = 0; i < numVertices; i++) { normals.push_back(glm::vec3()); }
	for (int i = 0; i < numVertices; i++) { tangents.push_back(glm::vec3()); }
	for (int i = 0; i < numIndices; i++) { indices.push_back(0); }

	// calculate triangle vertices
	for (int i = 0; i <= prec; i++) {
		for (int j = 0; j <= prec; j++) {
			//float y = (float)cos(toRadians(180.0f - i * 180.0f / prec)); //y��ֵ�� 180�㵽0��
			//float x = -(float)cos(toRadians(j*360.0f / prec))*(float)abs(cos(asin(y)));//x��ֵ�� 0�㵽360��
			//float z = (float)sin(toRadians(j*360.0f / (float)(prec)))*(float)abs(cos(asin(y)));//zֵ�� 0�㵽360��
			float y = (float)cos(toRadians(180.0f - i * 180.0f / prec));
			float sin_theta = (float)sqrt(1 - y * y); // ���� sin(��)
			float x = -(float)cos(toRadians(j * 360.0f / prec)) * sin_theta;
			float z = (float)sin(toRadians(j * 360.0f / (float)(prec))) * sin_theta;
			vertices[i*(prec + 1) + j] = glm::vec3(x, y, z);
			texCoords[i*(prec + 1) + j] = glm::vec2(((float)i / prec), ((float)j / prec)*2);
			normals[i*(prec + 1) + j] = glm::vec3(x, y, z);

			// calculate tangent vector
			if (((x == 0) && (y == 1) && (z == 0)) || ((x == 0) && (y == -1) && (z == 0))) {
				tangents[i*(prec + 1) + j] = glm::vec3(0.0f, 0.0f, -1.0f);
			}
			else {
				tangents[i*(prec + 1) + j] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z));
			}
		}
	}
	// calculate triangle indices
	for (int i = 0; i<prec; i++) {
		for (int j = 0; j<prec; j++) {
			indices[6 * (i*prec + j) + 0] = i*(prec + 1) + j;
			indices[6 * (i*prec + j) + 1] = i*(prec + 1) + j + 1;
			indices[6 * (i*prec + j) + 2] = (i + 1)*(prec + 1) + j;
			indices[6 * (i*prec + j) + 3] = i*(prec + 1) + j + 1;
			indices[6 * (i*prec + j) + 4] = (i + 1)*(prec + 1) + j + 1;
			indices[6 * (i*prec + j) + 5] = (i + 1)*(prec + 1) + j;
		}
	}
}

int Sphere::getNumVertices() { return numVertices; }
int Sphere::getNumIndices() { return numIndices; }
std::vector<int> Sphere::getIndices() { return indices; }
std::vector<glm::vec3> Sphere::getVertices() { return vertices; }
std::vector<glm::vec2> Sphere::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Sphere::getNormals() { return normals; }
std::vector<glm::vec3> Sphere::getTangents() { return tangents; }