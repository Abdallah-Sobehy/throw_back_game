// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/throw_back_fns.hpp>
#include <glm/gtx/euler_angles.hpp>//rotation
#include <glm/gtx/norm.hpp>//rotation

#define LEFT_TANK_X -15
#define RIGHT_TANK_X 15
#define TANK_Y -12
#define TANK_STEP 0.2
#define PROJECTILE_Y (TANK_Y+2.5)
#define PROJECTILE_Z 0.4
#define LEFT_TANK_PROJECTILE_X (RIGHT_TANK_X+3.4)
#define RIGHT_TANK_PROJECTILE_X (RIGHT_TANK_X-3.4)
#define PROJECTILE_STEP 0.4

using namespace glm;

GLuint programID;
GLuint vertexPosition_modelspaceID;
GLuint vertexUVID;
GLuint vertexNormal_modelspaceID;
GLuint LightID;
GLuint TextureID; 

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;

glm::mat4 Projection;

glm::mat4 tmp_view;
glm::mat4 tmp_rotation;
glm::mat4 tmp_translation;
glm::mat4 tmp_scaling ;
glm::mat4 tmp_model;
vec3 tmp_light_pos;

GLuint fire_Texture;
std::vector<glm::vec3> fire_vertices;
std::vector<glm::vec2> fire_uvs;
std::vector<glm::vec3> fire_normals;

float tank_y = TANK_Y;
float r_tank_x = RIGHT_TANK_X;
float l_tank_x = LEFT_TANK_X;
float move_step = TANK_STEP;

float projectile_x = RIGHT_TANK_PROJECTILE_X, projectile_y = PROJECTILE_Y, projectile_z=PROJECTILE_Z;
float projectile_scale = 0.7;
char * turn = "right";

void draw_object(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture_bmp,GLuint textureID,glm::mat4 view, glm::mat4 model, vec3 lightPos);

int main( void ) {

    // initialize GLEW, GLFW
    if(!initialize_gl())
    {
        fprintf(stderr, "GL initialization failed");
        return 1;
    }

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our buffers
    vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    vertexUVID = glGetAttribLocation(programID, "vertexUV");
    vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    Projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    // Load the texture
    GLuint tank_Texture = loadBMP_custom("Tank/tank.bmp");
    GLuint BG_Texture = loadBMP_custom("BG1.bmp");
    fire_Texture = loadBMP_custom("fire.bmp");
    GLuint healthBar_texture= loadBMP_custom("green.bmp");

    // Get a handle for our "myTextureSampler" uniform
    TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec3> tank_vertices;
    std::vector<glm::vec2> tank_uvs;
    std::vector<glm::vec3> tank_normals;
    bool res = loadOBJ("Tank/tank.obj", tank_vertices, tank_uvs, tank_normals);

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, tank_vertices.size() * sizeof(glm::vec3), &tank_vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tank_uvs.size() * sizeof(glm::vec2), &tank_uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, tank_normals.size() * sizeof(glm::vec3), &tank_normals[0], GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    std::vector<glm::vec3> BG_vertices;
    std::vector<glm::vec2> BG_uvs;
    std::vector<glm::vec3> BG_normals;
    loadOBJ("square.obj", BG_vertices, BG_uvs, BG_normals);

    loadOBJ("fire.obj", fire_vertices, fire_uvs, fire_normals);

    // indicates the state of the projectile (wither loaded in one of the tanks or flying
    bool projectile_flying = false;
    float projectile_end_x, projectile_origin_x,projectile_step_x;
    float projectile_radius;

    do {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        if (glfwGetKey( GLFW_KEY_LEFT) ==GLFW_PRESS && !projectile_flying) //left arrow is pressed
        {
            if(turn=="right")
                r_tank_x-=move_step;
            else
                l_tank_x-=move_step;
        }
        else if (glfwGetKey( GLFW_KEY_RIGHT) ==GLFW_PRESS && !projectile_flying)
        {
            if(turn == "right")
                r_tank_x+=move_step;
            else
                l_tank_x+=move_step;
        }
        // if space fire, second condition to ensure the press is detected once
        else if (glfwGetKey( GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey( GLFW_KEY_SPACE) == GLFW_RELEASE && !projectile_flying)
        {
            //fire(turn,0);
            projectile_flying = true;
            if(turn == "right") // switch turns
            {
                // set projectile end point
                projectile_end_x = l_tank_x;
                projectile_step_x=-PROJECTILE_STEP;
                turn = "left";
            }
            else {// left turn
                projectile_end_x = r_tank_x;
                projectile_step_x=PROJECTILE_STEP;
                turn = "right";
            }
            projectile_origin_x = (projectile_end_x+projectile_x)/2;
            projectile_radius = fabs(projectile_x-projectile_end_x);
            fprintf(stderr,"projectile x = %f , projectile end x =  %f, circle origin = %f, radius = %f\n",projectile_x,projectile_end_x,projectile_origin_x,projectile_radius);
            //fprintf(stderr,"fire pressed\n");
            //sleep(2);// to avoid counting to clicks
        }

        // background
        tmp_view = glm::lookAt(glm::vec3(0,0,20), glm::vec3(0,0,0),glm::vec3(0,-1,0));
        tmp_rotation = eulerAngleYXZ(0.0f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(0,0,0));
        tmp_scaling = scale(mat4(), vec3(30, 20, 20));
        tmp_model = tmp_translation*tmp_rotation*tmp_scaling;
        tmp_light_pos = glm::vec3(0,5,15);
        draw_object(BG_vertices, BG_uvs, BG_normals,BG_Texture,TextureID,tmp_view,tmp_model,tmp_light_pos);

        // projectile
        if( !projectile_flying)
        {
            if (turn == "right"){
                projectile_x = r_tank_x-3.4; projectile_y = tank_y+2.5; projectile_z=-0.4;
            }
            else{
                projectile_x = l_tank_x+3.4; projectile_y = tank_y+2.5; projectile_z=-0.4;
            }
        }
        else if (projectile_flying) // compute next position in the projectile motion
        {
            // flying stopping condition
            if ((turn == "right" && projectile_x>=projectile_end_x)|| (turn=="left" &&projectile_x<=projectile_end_x))
            {
                projectile_flying=false;
            }
            else
            {
                projectile_x+=projectile_step_x;
                projectile_y = sqrt(projectile_radius*projectile_radius - (projectile_x-projectile_origin_x)*(projectile_x-projectile_origin_x)) + PROJECTILE_Y-23;
            }

        }
        tmp_view = glm::lookAt(glm::vec3(0,0,20), glm::vec3(0,0,0),glm::vec3(0,1,0));
        tmp_rotation = eulerAngleYXZ(0.0f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(projectile_x,projectile_y,projectile_z));
        tmp_scaling = scale(mat4(), vec3(projectile_scale, projectile_scale, projectile_scale));
        tmp_model = tmp_translation*tmp_rotation*tmp_scaling;
        tmp_light_pos = glm::vec3(projectile_x,projectile_y,10);
        draw_object(fire_vertices, fire_uvs, fire_normals,fire_Texture,TextureID,tmp_view,tmp_model,tmp_light_pos);

        // Left tank
        tmp_view = glm::lookAt(glm::vec3(0,0,20),glm::vec3(0,0,0), glm::vec3(0,1,0));
        tmp_rotation = eulerAngleYXZ(1.65f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(l_tank_x,tank_y,0));
        tmp_scaling = scale(mat4(), vec3(0.70f, 0.7f, 0.7f));
        tmp_model = tmp_translation*tmp_rotation*tmp_scaling;
        tmp_light_pos = glm::vec3(l_tank_x,0,0);
        draw_object(tank_vertices, tank_uvs, tank_normals,tank_Texture,TextureID,tmp_view,tmp_model,tmp_light_pos);

        // Left health bar
        tmp_view = glm::lookAt(glm::vec3(0,0,19.9),glm::vec3(0,0,0), glm::vec3(0,1,0));
        tmp_rotation = eulerAngleYXZ(0.0f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(l_tank_x,-5.5,0));
        tmp_scaling = scale(mat4(), vec3(2.5f, 0.4f, 1.0f));
        tmp_model = tmp_translation*tmp_scaling;
        tmp_light_pos = glm::vec3(l_tank_x,-5.5,10);
        draw_object(BG_vertices, BG_uvs, BG_normals,healthBar_texture,TextureID,tmp_view,tmp_model,tmp_light_pos);


        // right tank
        tmp_view = glm::lookAt(glm::vec3(0,0,20),glm::vec3(0,0,0), glm::vec3(0,1,0));
        tmp_rotation = eulerAngleYXZ(-1.65f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(r_tank_x,tank_y,0));
        tmp_scaling = scale(mat4(), vec3(0.70f, 0.7f, 0.7f));
        tmp_model = tmp_translation*tmp_rotation*tmp_scaling;
        tmp_light_pos = glm::vec3(r_tank_x,0,0);
        draw_object(tank_vertices, tank_uvs, tank_normals,tank_Texture,TextureID,tmp_view,tmp_model,tmp_light_pos);

        // Right health bar
        tmp_view = glm::lookAt(glm::vec3(0,0,19.9),glm::vec3(0,0,0), glm::vec3(0,1,0));
        tmp_rotation = eulerAngleYXZ(0.0f, 0.0f,0.0f);
        tmp_translation = translate(mat4(), vec3(r_tank_x,-5.5,0));
        tmp_scaling = scale(mat4(), vec3(2.5f, 0.4f, 1.0f));
        tmp_model = tmp_translation*tmp_scaling;
        tmp_light_pos = glm::vec3(r_tank_x,-5.5,10);
        draw_object(BG_vertices, BG_uvs, BG_normals,healthBar_texture,TextureID,tmp_view,tmp_model,tmp_light_pos);

        //sleep(0.3);
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
    glDeleteTextures(1, &tank_Texture);
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

/**
 * @brief draw_object draws an object by specifying the .obj file and the texture
 * @param vertices
 * @param uvs
 * @param normals
 * @param texture ID of texture loaded from bmp file
 * @param textureID handle for our "myTextureSampler" uniform
 * @param translation translation values
 * @param rotation rotation values
 * @param scaling scaling values
 */
void draw_object(std::vector<glm::vec3> vertices,std::vector<glm::vec2> uvs,std::vector<glm::vec3> normals,GLuint texture_bmp,GLuint textureID,glm::mat4 view, glm::mat4 model, vec3 lightPos)
{

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

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform

    glm::mat4 MVP = Projection * view * model;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &view[0][0]);

    //lightPos = glm::vec3(0,5,15); // position of the light source
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_bmp);
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
