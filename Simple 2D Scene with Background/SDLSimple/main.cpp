/**
* Author: William Wei
* Assignment: Simple 2D Scene
* Date due: 2025-02-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
* 
* Image Sources:
* Lumine: https://www.deviantart.com/yessing/art/Lumine-Render-947504571
* Paimon: https://www.pngall.com/paimon-png/
* Anemo Slime: https://genshin-impact.fandom.com/wiki/Large_Anemo_Slime
* Background: https://genshinresource.tumblr.com/post/696826127993503744/genshin-impact-town-reputation-background-images
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH = 640*1.5,
WINDOW_HEIGHT = 480*1.5;

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
LEVEL_OF_DETAIL = 0, // mipmap reduction image level
TEXTURE_BORDER = 0; // this value MUST be zero

constexpr char LUMINE_SPRITE_FILEPATH[] = "lumine.png",
PAIMON_SPRITE_FILEPATH[] = "paimon.png",
ANEMO_SLIME_SPRITE_FILEPATH[] = "slime.png",
BACKGROUND_SPRITE_FILEPATH[] = "background.png";

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
float g_previous_ticks = 0.0f;
float g_lumine_previous_frames = 0.0f;
float g_paimon_previous_frames = 0.0f;
float g_slime_previous_frames = 0.0f;

constexpr float PAIMON_ROT_SPEED = 2.0f,
LUMINE_SPEED = 2.0f,
LUMINE_RADIUS_FROM_CENTER = 1.0f,
PAIMON_SPEED = 2.0f,
PAIMON_RADIUS_FROM_CENTER = 1.0f,
SLIME_SPEED = 1.5f,
SLIME_JUMP_DISTANCE = 2.5f,
SLIME_MAX_SIZE = 1.5f;


constexpr glm::vec3 BASE_LUMINE_SCALE = glm::vec3(2.0f, 2.0f, 0.0f),
BASE_PAIMON_SCALE = glm::vec3(0.7f, 0.7f, 0.0f),
BASE_SLIME_SCALE = glm::vec3(1.0f, 0.7f, 0.0f),
BASE_BACKGROUND_SCALE = glm::vec3(15.0f, 15.0f, 0.0f);

glm::mat4 g_view_matrix,
g_background_matrix,
g_lumine_matrix,
g_paimon_matrix,
g_slime_matrix,
g_projection_matrix;

glm::vec3 g_translation_lumine = glm::vec3(0.0f, 0.0f, 0.0f),
g_translation_paimon = glm::vec3(0.0f, 0.0f, 0.0f),
g_rotation_paimon = glm::vec3(0.0f, 0.0f, 0.0f),
g_translation_slime = glm::vec3(3.5f, 0.0f, 0.0f),
g_slime_scale = glm::vec3(0.0f, 0.0f, 0.0f),
g_translation_background = glm::vec3(-0.5f, 3.5f, 0.0f);

GLuint g_lumine_texture_id,
g_paimon_texture_id,
g_slime_texture_id,
g_background_texture_id;


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    // Initialise video
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("Simple 2D Scene",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_lumine_matrix = glm::mat4(1.0f);
    g_paimon_matrix = glm::mat4(1.0f);
    g_slime_matrix = glm::mat4(1.0f);
    g_background_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_lumine_texture_id = load_texture(LUMINE_SPRITE_FILEPATH);
    g_paimon_texture_id = load_texture(PAIMON_SPRITE_FILEPATH);
    g_slime_texture_id = load_texture(ANEMO_SLIME_SPRITE_FILEPATH);
    g_background_texture_id = load_texture(BACKGROUND_SPRITE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}

void update()
{
    /* Delta time calculations */
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    /* Game logic */

    g_lumine_previous_frames += LUMINE_SPEED * delta_time;
    g_translation_lumine.x = LUMINE_RADIUS_FROM_CENTER * cos(g_lumine_previous_frames) - 1.5f;
    g_translation_lumine.y = LUMINE_RADIUS_FROM_CENTER * sin(g_lumine_previous_frames) - 0.3f;

    g_rotation_paimon.y += PAIMON_ROT_SPEED * delta_time;
    g_paimon_previous_frames += PAIMON_SPEED * delta_time;
    g_translation_paimon.x = PAIMON_RADIUS_FROM_CENTER * cos(g_paimon_previous_frames);
    g_translation_paimon.y = PAIMON_RADIUS_FROM_CENTER * sin(g_paimon_previous_frames);

    g_slime_previous_frames += SLIME_SPEED * delta_time;
    g_translation_slime.y = abs(SLIME_JUMP_DISTANCE * sin(g_slime_previous_frames));
    g_slime_scale.x = BASE_SLIME_SCALE.x + abs(SLIME_MAX_SIZE * glm::sin(g_slime_previous_frames));
    g_slime_scale.y = BASE_SLIME_SCALE.y + abs(SLIME_MAX_SIZE * glm::sin(g_slime_previous_frames));

    /* Model matrix reset */
    g_background_matrix = glm::mat4(1.0f);
    g_lumine_matrix = glm::mat4(1.0f);
    g_paimon_matrix = glm::mat4(1.0f);
    g_slime_matrix = glm::mat4(1.0f);

    /* Transformations */
    g_background_matrix = glm::translate(g_background_matrix, g_translation_background);
    g_background_matrix = glm::scale(g_background_matrix, BASE_BACKGROUND_SCALE);

    g_lumine_matrix = glm::translate(g_lumine_matrix, g_translation_lumine);
    g_lumine_matrix = glm::scale(g_lumine_matrix, BASE_LUMINE_SCALE);

    g_paimon_matrix = glm::translate(g_lumine_matrix, g_translation_paimon);
    g_paimon_matrix = glm::rotate(g_paimon_matrix, g_rotation_paimon.y, glm::vec3(0.0f, 1.0f, 0.0f));
    g_paimon_matrix = glm::scale(g_paimon_matrix, BASE_PAIMON_SCALE);

    g_slime_matrix = glm::translate(g_slime_matrix, g_translation_slime);
    g_slime_matrix = glm::scale(g_slime_matrix, g_slime_scale);
}

void draw_object(glm::mat4& object_g_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] =
    {
        -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
        0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_background_matrix, g_background_texture_id); // Load background first so other objects are drawn on top
    draw_object(g_lumine_matrix, g_lumine_texture_id);
    draw_object(g_paimon_matrix, g_paimon_texture_id);
    draw_object(g_slime_matrix, g_slime_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
