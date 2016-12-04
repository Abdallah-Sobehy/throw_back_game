// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <glm/gtx/euler_angles.hpp>//rotation
#include <glm/gtx/norm.hpp>//rotation

void draw_background(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID );
void drawSphere(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID );
void drawHealthBar(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID,int x );
GLuint programID;
GLuint vertexPosition_modelspaceID;
GLuint vertexUVID;
GLuint vertexNormal_modelspaceID;
GLuint LightID;

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

    glfwSetWindowTitle( "Throw Back" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(1024/2, 768/2);

	// Dark blue background
    //glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
    programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
    vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    vertexUVID = glGetAttribLocation(programID, "vertexUV");
    vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
    GLuint Texture = loadBMP_custom("Tank/tank.bmp");
    GLuint BG_Texture = loadBMP_custom("BG1.bmp");
    GLuint fire_Texture = loadBMP_custom("fire.bmp");
    GLuint healthBar= loadBMP_custom("green.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("Tank/tank.obj", vertices, uvs, normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");// bashawer 3ala el variable
    // LightPosition_worldspace elly fel shader

    std::vector<glm::vec3> BG_vertices;
    std::vector<glm::vec2> BG_uvs;
    std::vector<glm::vec3> BG_normals;
    loadOBJ("square.obj", BG_vertices, BG_uvs, BG_normals);

    std::vector<glm::vec3> fire_vertices;
    std::vector<glm::vec2> fire_uvs;
    std::vector<glm::vec3> fire_normals;
    loadOBJ("fire.obj", fire_vertices, fire_uvs, fire_normals);

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
        glUseProgram(programID);
        draw_background(BG_vertices, BG_uvs, BG_normals,BG_Texture, TextureID);
        drawSphere(fire_vertices, fire_uvs,fire_normals,fire_Texture,TextureID);
        drawHealthBar(BG_vertices, BG_uvs, BG_normals,healthBar, TextureID,-2);
        drawHealthBar(BG_vertices, BG_uvs, BG_normals,healthBar, TextureID,2);



        // Compute the MVP matrix from keyboard and mouse input
        //computeMatricesFromInputs();
        //glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        //glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 Projection1 = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 View1       = glm::lookAt(
                                                                glm::vec3(0,-5,20),
                                                                glm::vec3(0,0,0), // and looks at the origin
                                                                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );
        glm::mat4 RotationMatrix1 = eulerAngleYXZ(1.80f, 0.0f,0.0f);
        glm::mat4 TranslationMatrix1 = translate(mat4(), vec3(-15,-12,0));
        glm::mat4 ScalingMatrix1 = scale(mat4(), vec3(0.70f, 0.7f, 0.7f));
        glm::mat4 Model1 = TranslationMatrix1*RotationMatrix1*ScalingMatrix1;

        glm::mat4 MVP = Projection1 * View1 * Model1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model1[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View1[0][0]);

        glm::vec3 lightPos = glm::vec3(-15,-12,16); // position of the light source
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                   // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,    // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

        //////////////////////////////////////////////////////////////////////////////////
        /// SECOND TANK - just a copy and paste of the upper part with changing the transformation parameters
        ///
        glm::mat4 Projection2 = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 View2       = glm::lookAt(
                                                                glm::vec3(0,-5,20),
                                                                glm::vec3(0,0,0), // and looks at the origin
                                                                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );
        glm::mat4 RotationMatrix2 = eulerAngleYXZ(-1.80f, 0.0f,0.0f);
        glm::mat4 TranslationMatrix2 = translate(mat4(), vec3(15,-12,0));
        glm::mat4 ScalingMatrix2 = scale(mat4(), vec3(0.70f, 0.7f, 0.7f));
        glm::mat4 Model2 = TranslationMatrix2*RotationMatrix2*ScalingMatrix2;

        MVP = Projection2 * View2 * Model2;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model2[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View2[0][0]);

        lightPos = glm::vec3(15,-12,16); // position of the light source
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            vertexPosition_modelspaceID,  // The attribute we want to configure
            3,                            // size
            GL_FLOAT,                     // type
            GL_FALSE,                     // normalized?
            0,                            // stride
            (void*)0                      // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(vertexUVID);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            vertexUVID,                   // The attribute we want to configure
            2,                            // size : U+V => 2
            GL_FLOAT,                     // type
            GL_FALSE,                     // normalized?
            0,                            // stride
            (void*)0                      // array buffer offset
        );

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(vertexNormal_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
            vertexNormal_modelspaceID,    // The attribute we want to configure
            3,                            // size
            GL_FLOAT,                     // type
            GL_FALSE,                     // normalized?
            0,                            // stride
            (void*)0                      // array buffer offset
        );

        // Draw the triangles !
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

        glDisableVertexAttribArray(vertexPosition_modelspaceID);
        glDisableVertexAttribArray(vertexUVID);
        glDisableVertexAttribArray(vertexNormal_modelspaceID);


        // Swap buffers
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void draw_background(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID )
{
    //std::vector<glm::vec3> vertices;
    //std::vector<glm::vec2> uvs;
    //std::vector<glm::vec3> normals;

    GLuint vertexbuffer;
    // Load it into a VBO
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glm::mat4 Projection3 = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View3       = glm::lookAt(
                                                            glm::vec3(0,0,20),
                                                            glm::vec3(0,0,0), // and looks at the origin
                                                            glm::vec3(0,-1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                );
    glm::mat4 RotationMatrix3 = eulerAngleYXZ(0.0f, 0.0f,0.0f);
    glm::mat4 TranslationMatrix3 = translate(mat4(), vec3(0,0,0));
    glm::mat4 ScalingMatrix3 = scale(mat4(), vec3(30.0f, 20.0f, 20.0f));
    glm::mat4 Model3 = TranslationMatrix3*RotationMatrix3*ScalingMatrix3;

    glm::mat4 MVP = Projection3 * View3 * Model3;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model3[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View3[0][0]);

    glm::vec3 lightPos = glm::vec3(0,5,15); // position of the light source
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    /*lightPos = glm::vec3(0,5,3); // position of the light source
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);*/

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        vertexPosition_modelspaceID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(vertexUVID);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        vertexUVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(vertexNormal_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        vertexNormal_modelspaceID,    // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexUVID);
    glDisableVertexAttribArray(vertexNormal_modelspaceID);

}

void drawSphere(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID )
{
    //std::vector<glm::vec3> vertices;
    //std::vector<glm::vec2> uvs;
    //std::vector<glm::vec3> normals;

    GLuint vertexbuffer;
    // Load it into a VBO
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glm::mat4 Projection3 = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View3       = glm::lookAt(
                                                            glm::vec3(0,0,20),
                                                            glm::vec3(0,0,0), // and looks at the origin
                                                            glm::vec3(0,-1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                );
    glm::mat4 RotationMatrix3 = eulerAngleYXZ(0.0f, 0.0f,0.0f);
    glm::mat4 TranslationMatrix3 = translate(mat4(), vec3(6,6,0));
    glm::mat4 ScalingMatrix3 = scale(mat4(), vec3(0.7f, 0.7f, 0.7f));
    glm::mat4 Model3 = TranslationMatrix3*RotationMatrix3*ScalingMatrix3;

    glm::mat4 MVP = Projection3 * View3 * Model3;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model3[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View3[0][0]);

    glm::vec3 lightPos = glm::vec3(0,5,6); // position of the light source
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        vertexPosition_modelspaceID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(vertexUVID);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        vertexUVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(vertexNormal_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        vertexNormal_modelspaceID,    // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexUVID);
    glDisableVertexAttribArray(vertexNormal_modelspaceID);

}

void drawHealthBar(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture,GLuint textureID,int x )
{
    //std::vector<glm::vec3> vertices;
    //std::vector<glm::vec2> uvs;
    //std::vector<glm::vec3> normals;

    GLuint vertexbuffer;
    // Load it into a VBO
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glm::mat4 Projection3 = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View3       = glm::lookAt(
                                                            glm::vec3(0,0,2),
                                                            glm::vec3(0,0,0), // and looks at the origin
                                                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                );
    glm::mat4 RotationMatrix3 = eulerAngleYXZ(0.0f, 0.0f,0.0f);
    glm::mat4 TranslationMatrix3 = translate(mat4(), vec3(x,-0.5,0));
    glm::mat4 ScalingMatrix3 = scale(mat4(), vec3(0.3f , 0.03f, 1.0f));
    glm::mat4 Model3 = TranslationMatrix3*RotationMatrix3*ScalingMatrix3;

    glm::mat4 MVP = Projection3 * View3 * Model3;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model3[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View3[0][0]);

    glm::vec3 lightPos = glm::vec3(x,-0.5,10); // position of the light source
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        vertexPosition_modelspaceID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(vertexUVID);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        vertexUVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(vertexNormal_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        vertexNormal_modelspaceID,    // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexUVID);
    glDisableVertexAttribArray(vertexNormal_modelspaceID);

}
