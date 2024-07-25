#pragma once
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <cmath>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <gl/glut.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI 3.1415926
typedef OpenMesh::TriMesh_ArrayKernelT<> sphereMesh;

// 创建球体网格函数
sphereMesh create_sphere_mesh(int n) {
    sphereMesh mesh;
    int count = n * n + 2;
    sphereMesh::VertexHandle* vhandle = new sphereMesh::VertexHandle[count];
    std::vector<sphereMesh::VertexHandle> face_vhandles;

    int k = -1;
    for (int j = 0; j < n; j++) {
        double distance = cos(j * PI / n);
        double r_circle = sin(j * PI / n);
        for (int i = 0; i < n; i++) {
            ++k;
            vhandle[k] = mesh.add_vertex(sphereMesh::Point(r_circle * cos(2 * i * PI / n), r_circle * sin(2 * i * PI / n), distance));
        }
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            int topRight = i * n + j;
            int topLeft = i * n + (j + 1) % n;
            int bottomRight = (i + 1) * n + j;
            int bottomLeft = (i + 1) * n + (j + 1) % n;
            face_vhandles.clear();
            face_vhandles.push_back(vhandle[bottomRight]);
            face_vhandles.push_back(vhandle[bottomLeft]);
            face_vhandles.push_back(vhandle[topRight]);
            mesh.add_face(face_vhandles);

            face_vhandles.clear();
            face_vhandles.push_back(vhandle[bottomLeft]);
            face_vhandles.push_back(vhandle[topLeft]);
            face_vhandles.push_back(vhandle[topRight]);
            mesh.add_face(face_vhandles);
        }
    }

    vhandle[k + 1] = mesh.add_vertex(sphereMesh::Point(0, 0, -1));
    for (int i = 0; i < n; i++) {
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[k - i]);
        face_vhandles.push_back(vhandle[k - (i + 1) % n]);
        face_vhandles.push_back(vhandle[k + 1]);
        mesh.add_face(face_vhandles);
    }

    delete[] vhandle;
    return mesh;
}

// 加载纹理
GLuint load_texture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        exit(1);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, channels == 4 ? GL_RGBA : GL_RGB, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return texture;
}

// 计算UV坐标
void calculate_uv_coordinates(sphereMesh& mesh) {
    for (auto vh : mesh.vertices()) {
        OpenMesh::Vec3f pos = mesh.point(vh);
        float u = 0.5f + (atan2(pos[2], pos[0]) / (2 * PI));
        float v = 0.5f - (asin(pos[1]) / PI);
        mesh.set_texcoord2D(vh, OpenMesh::Vec2f(u, v));
    }
}

// 顶点和纹理坐标数据
struct Vertex {
    float position[3];
    float texcoord[2];
};

// 创建顶点缓冲对象 (VBO)
void create_vbo(sphereMesh& mesh, GLuint& vbo, GLuint& vao) {
    std::vector<Vertex> vertices;
    for (auto vh : mesh.vertices()) {
        OpenMesh::Vec3f pos = mesh.point(vh);
        OpenMesh::Vec2f tex = mesh.texcoord2D(vh);
        Vertex vertex = { {pos[0], pos[1], pos[2]}, {tex[0], tex[1]} };
        vertices.push_back(vertex);
    }

    std::vector<GLuint> indices;
    for (auto fh : mesh.faces()) {
        for (auto fv_it = mesh.fv_iter(fh); fv_it.is_valid(); ++fv_it) {
            indices.push_back((*fv_it).idx());
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// 加载着色器
GLuint load_shader(const char* vertex_path, const char* fragment_path) {
    std::string vertex_code;
    std::string fragment_code;
    std::ifstream v_shader_file;
    std::ifstream f_shader_file;

    v_shader_file.open(vertex_path);
    f_shader_file.open(fragment_path);
    std::stringstream v_shader_stream, f_shader_stream;

    v_shader_stream << v_shader_file.rdbuf();
    f_shader_stream << f_shader_file.rdbuf();

    v_shader_file.close();
    f_shader_file.close();

    vertex_code = v_shader_stream.str();
    fragment_code = f_shader_stream.str();

    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();

    GLuint vertex_shader, fragment_shader;
    GLint success;
    GLchar info_log[512];

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &v_shader_code, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &f_shader_code, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

int sphereDemo() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Textured Sphere", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 创建球体网格并计算UV坐标
    int n = 100; // 或者根据需要设置n的值
    sphereMesh mesh = create_sphere_mesh(n);
    calculate_uv_coordinates(mesh);

    // 加载纹理
    GLuint texture = load_texture("circle.jpg");

    // 创建VBO和VAO
    GLuint vbo, vao;
    create_vbo(mesh, vbo, vao);

    // 加载着色器
    GLuint shader_program = load_shader("vertShader.glsl", "fragShader.glsl");

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器
        glUseProgram(shader_program);

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);

        // 绑定VAO
        glBindVertexArray(vao);

        // 绘制球体
        glDrawArrays(GL_TRIANGLES, 0, mesh.n_faces() * 3);

        // 解绑
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


// 生成球  n 最好输入偶数  10 或者 100 
//#include <iostream>
//#include <OpenMesh/Core/IO/MeshIO.hh>
//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//#include<cmath>
//#define PI 3.1415926
//using namespace std;
//typedef OpenMesh::TriMesh_ArrayKernelT<> sphereMesh;
//
//void sphereDemo()
//{
//	sphereMesh mesh;
//	int n, count = 10000, k = -1;
//	cout << "n:";
//	cin >> n;
//	sphereMesh::VertexHandle* vhandle = new sphereMesh::VertexHandle[count];
//	std::vector<sphereMesh::VertexHandle>face_vhandles;
//	for (int j = 0; j < n; j++) {
//		double distance = cos(j * PI / n);// 原先是 sin
//		double r_circle = sin(j * PI / n);
//		for (int i = 0; i < n; i++) {
//			++k;
//			vhandle[k] = mesh.add_vertex(sphereMesh::Point(r_circle * cos(2 * i * PI / n), r_circle * sin(2 * i * PI / n), distance));
//			// 对网格添加顶点
//		}
//	}
//	for (int i = 0; i < n - 1; i++) {
//		for (int j = 0; j < n; j++) {
//			int topRight = i * n + j;
//			int topLeft = i * n + (j + 1) % n;
//			int bottomRight = (i + 1) * n + j;
//			int bottomLeft = (i + 1) * n + (j + 1) % n;
//			face_vhandles.clear();
//			face_vhandles.push_back(vhandle[bottomRight]);
//			face_vhandles.push_back(vhandle[bottomLeft]);
//			face_vhandles.push_back(vhandle[topRight]);
//			mesh.add_face(face_vhandles);
//
//			face_vhandles.clear();
//			face_vhandles.push_back(vhandle[bottomLeft]);
//			face_vhandles.push_back(vhandle[topLeft]);
//			face_vhandles.push_back(vhandle[topRight]);
//			mesh.add_face(face_vhandles);
//
//		}
//	}
//	//vhandle[k+1] = mesh.add_vertex(sphereMesh::Point(0, 0, 1));
//	vhandle[k + 1] = mesh.add_vertex(sphereMesh::Point(0, 0, -1));
//	for (int i = 0; i < n; i++) {
//
//		face_vhandles.clear();
//		face_vhandles.push_back(vhandle[k - i]);
//		face_vhandles.push_back(vhandle[k - (i + 1) % n]);
//		face_vhandles.push_back(vhandle[k + 1]);
//		mesh.add_face(face_vhandles);
//	}
//	try
//	{
//		if (!OpenMesh::IO::write_mesh(mesh, "sphereDemo.off")) {
//			std::cerr << "Cannot write mesh to file ' sphereDemo.off ' " << std::endl;
//			return ;
//		}
//	}
//	catch (std::exception& x) {
//		std::cerr << x.what() << std::endl;
//		return ;
//	}
//}