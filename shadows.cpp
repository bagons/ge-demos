#include "graphicengine.hpp"
#include <iostream>
#include <ranges>
#include <algorithm>

#include "ext/matrix_clip_space.hpp"
#include "ext/matrix_transform.hpp"

EngineSettings settings{ .MAX_NR_POINT_LIGHTS = 1, .MAX_NR_DIRECTIONAL_LIGHTS = 1, .MAX_NR_SPOT_LIGHTS = 1};
Engine ge{"shadow demo", 1024, 1024, settings};


class FullScreenQuadPass : public RenderPass {
public:
    std::shared_ptr<Material> mat;
    geRef<MeshThing> quad_thing;

    FullScreenQuadPass(std::shared_ptr<Material> quad_mat) : mat(quad_mat) {
        const std::vector QUAD_VERTEX_DATA = {
            // Bottom-left
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
            // Bottom-right
            1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            // Top-left
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
            // Top-right
            1.0f,  1.0f, 0.0f,   1.0f, 1.0f
        };

        const std::vector<unsigned int> QUAD_INDICES = {
            0, 1, 2,
            1, 3, 2
        };

        auto quad = std::make_shared<Mesh>(&QUAD_VERTEX_DATA, &QUAD_INDICES, true, false);

        quad_thing = ge.add<MeshThing>(quad, quad_mat);
        quad_thing->visible = false;
    }

    void render() {
        mat->get_shader_program().use();
        mat->apply_uniform_values();
        glDisable(GL_DEPTH_TEST);
        quad_thing->render();
        glEnable(GL_DEPTH_TEST);
    }
};


class DirectionalShadowPass : public RenderPass {
public:
    std::shared_ptr<Texture> shadow_map;
    geRef<DirectionalLight> light;
    unsigned int render_layer;
    const unsigned int width;
    const unsigned int height;
    unsigned int depth_map_fbo;
    glm::mat4 projection;
    glm::mat4 view;
    ShaderProgram geometry_only_sp;
    unsigned int vs_uniform_model_matrix_loc;
    unsigned int light_space_mat_ubo;

    explicit DirectionalShadowPass(const geRef<DirectionalLight> dir_light, const unsigned int shadow_map_width = 1024, const unsigned int shadow_map_height = 1024) :
        light(dir_light), width(shadow_map_width), height(shadow_map_height),
        geometry_only_sp(ShaderProgram(
            Shader("res_demo2/depth_only.vs", Shader::VERTEX_SHADER),
            Shader("res_demo2/depth_only.fs", Shader::FRAGMENT_SHADER)
        )) {
        // generate depth buffer
        glGenFramebuffers(1, &depth_map_fbo);

        // create depth map
        unsigned int depth_map;
        glGenTextures(1, &depth_map);
        glBindTexture(GL_TEXTURE_2D, depth_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     static_cast<int>(shadow_map_width), static_cast<int>(shadow_map_height), 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // connect depth map to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // create shadow map engine Texture (a bit of a hack / exploit)
        shadow_map = std::make_shared<Texture>(Color::BLACK);
        if (ge.are_bindless_textures_supported()) {
            glMakeTextureHandleNonResidentARB(shadow_map->handle);
        }
        glDeleteTextures(1, &shadow_map->id);
        shadow_map->id = depth_map;

        if (ge.are_bindless_textures_supported()) {
            shadow_map->handle = glGetTextureHandleARB(shadow_map->id);
            glMakeTextureHandleResidentARB(shadow_map->handle);

            if (!glIsTextureHandleResidentARB(shadow_map->handle)) {
                std::cerr << "ENGINE ERROR: Texture handle is NOT resident!" << std::endl;
            }
        }

        // cam setup
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.2f, 100.0f);

        vs_uniform_model_matrix_loc = glGetUniformLocation(geometry_only_sp.get_id(), "transform");

        // light space mat UBO
        glGenBuffers(1, &light_space_mat_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, light_space_mat_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, light_space_mat_ubo, 0, sizeof(glm::mat4));

        // connect ubo
        glUniformBlockBinding(geometry_only_sp.get_id(), glGetUniformBlockIndex(geometry_only_sp.get_id(), "LIGHT_MATRICES"), 2);
    }

    void update_light_view_mat() {
        // reverse rotation
        view = glm::lookAt(
            -light->direction.glm_vector() * 10.0f,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );


        const glm::mat4 light_mat = projection * view;

        glBindBuffer(GL_UNIFORM_BUFFER, light_space_mat_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &light_mat);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void render() {
        // save viewport state
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        // 1. first render to depth map
        glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        update_light_view_mat();

        geometry_only_sp.use();
        glCullFace(GL_FRONT);
        for (const auto &id: ge.thing_ids_by_shader_program | std::views::values) {
            auto t = ge.get_thing(id);
            // skip not renderable entities || or || an entity that does bitwise match by render layer
            if (!t->visible or !(t->render_layer & render_layer))
                continue;

            // custom render method
            if (auto* mt = dynamic_cast<MeshThing*>(t)) {
                glm::mat4 model = mt->transform.position.get_transformation_matrix() * mt->transform.rotation.get_transformation_matrix() * mt->transform.scale.get_transformation_matrix();

                glUniformMatrix4fv(static_cast<int>(vs_uniform_model_matrix_loc), 1, GL_FALSE, &model[0][0]);
                glBindVertexArray(mt->get_mesh()->get_vertex_array_object());
                glDrawElements(GL_TRIANGLES, mt->get_mesh()->get_vertex_count(), GL_UNSIGNED_INT, nullptr);
            }
        }
        glCullFace(GL_BACK);

        // reset viewport state
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

enum Actions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    SHADOW_MAP_SWITCH,
    EXIT
};

class Spectator : public Thing {
public:
    float move_speed = 10;
    float mouse_sensitivity = 0.002f;
    geRef<Camera> camera;
    Spectator(geRef<Camera> cam) : Thing(), camera(cam) {

    }

    void update() override {
        if (ge.input.get_mouse_mode() != Input::DISABLED)
            return;
        Vector3 move_vec{0, 0, 0};
        // movement
        if (ge.input.is_pressed(FORWARD))
            move_vec.z -= 1.0f;
        if (ge.input.is_pressed(BACKWARD))
            move_vec.z += 1.0f;

        if (ge.input.is_pressed(LEFT))
            move_vec.x -= 1.0f;
        if (ge.input.is_pressed(RIGHT))
            move_vec.x += 1.0f;



        Rotation::rotate_point(-camera->transform.rotation.x, 0, 0, move_vec);
        Rotation::rotate_point(0, -camera->transform.rotation.y, 0, move_vec);
        move_vec.normalize();
        move_vec *= ge.frame_delta * move_speed;

        camera->transform.position = camera->transform.position + move_vec;

        if (ge.input.is_pressed(UP))
            camera->transform.position.y += ge.frame_delta * move_speed;
        if (ge.input.is_pressed(DOWN))
            camera->transform.position.y -= ge.frame_delta * move_speed;

        // looking around
        if (abs(ge.input.mouse_move_delta.x) > 0 || abs(ge.input.mouse_move_delta.y) > 0) {
            float mouse_move_x = ge.input.mouse_move_delta.x * mouse_sensitivity;
            float mouse_move_y = ge.input.mouse_move_delta.y * mouse_sensitivity;

            camera->transform.rotation.y -= mouse_move_x;
            camera->transform.rotation.x -= mouse_move_y;
            camera->transform.rotation.x = std::clamp(camera->transform.rotation.x, -Engine::PI / 2, Engine::PI / 2);
        }
    }
};

int main() {
    ge.input.set_mouse_mode(Input::DISABLED);

    ge.input.set_action_list({
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_SPACE,
        GLFW_KEY_LEFT_CONTROL,
        GLFW_KEY_F1,
        GLFW_KEY_ESCAPE
    });

    std::cout << "starting" << std::endl;

    auto sun = ge.add<DirectionalLight>(Color::WHITE, 1.0f, Vector3{1.0f, -1.0f, 1.0f});

    auto shadow = ge.add_render_pass<DirectionalShadowPass>(sun);

    ShaderProgram shadow_map_display_sp(
        Shader("res_demo2/fullscreen_quad.glsl", Shader::VERTEX_SHADER),
        Shader("res_demo2/fullscreen_depth_map.glsl", Shader::FRAGMENT_SHADER)
        );

    std::cout << "transform" << shadow_map_display_sp.get_uniform_location("transform") << std::endl;

    auto shadow_map_display_mat = std::make_shared<Material>(shadow_map_display_sp);
    std::cout << shadow->shadow_map << std::endl;
    shadow_map_display_mat->set_uniform("depth_map", shadow->shadow_map);
    auto shadow_map_display = ge.add_render_pass<FullScreenQuadPass>(shadow_map_display_mat);

    ShaderProgram base_sp(
        Shader("res_demo2/vertex_shader_template.glsl", Shader::VERTEX_SHADER, "#define HAS_UV\n#define HAS_NORMALS\n"),
        Shader("res_demo2/phong_with_shadows.glsl", Shader::FRAGMENT_SHADER,"#define HAS_UV\n" + ge.are_bindless_textures_supported() ? "#define USE_BINDLESS\n" : "")
        );

    glUniformBlockBinding(base_sp.get_id(), glGetUniformBlockIndex(base_sp.get_id(), "LIGHT_MATRICES"), 2);

    // create base material with shadow support
    auto base_mat_with_shadows = std::make_shared<Material>(base_sp);
    base_mat_with_shadows->set_uniform("material.ambient", Vector3(0.2f));
    base_mat_with_shadows->set_uniform("material.diffuse", Color::WHITE.no_alpha());
    base_mat_with_shadows->set_uniform("material.specular", Color::WHITE.no_alpha());
    base_mat_with_shadows->set_uniform("material.shininess", 2.0f);
    base_mat_with_shadows->set_uniform("material.albedo_color", Color::WHITE.no_alpha());

    base_mat_with_shadows->set_uniform("albedo_texture", ge.shaders.get_placeholder_texture(Shaders::WHITE));
    base_mat_with_shadows->set_uniform("material.albedo_texture_scale", Vector2(1.0f));
    base_mat_with_shadows->set_uniform("shadow_map", shadow->shadow_map);

    auto cube_mat = base_mat_with_shadows->copy();
    cube_mat->set_uniform("material.diffuse", Color::ORANGE.no_alpha());

    auto cube = ge.add<MeshThing>(ge.meshes.get_cube(false), cube_mat);
    cube->transform.position.y = 1.0f;
    auto plane = ge.add<MeshThing>(ge.meshes.get_plane(false), base_mat_with_shadows);
    plane->transform.scale = Scale(10.0f);



    auto cam = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    cam->transform.position.z = 10.0f;
    cam->transform.position.y = 5.0f;
    cam->transform.rotation.x = Engine::PI / 8;
    ge.add<Spectator>(cam);
    //ge.lights.ambient_light = Color::WHITE;
    auto fo = ge.add_render_pass<ForwardOpaque3DPass>(cam);


    std::cout << "created everything"<< std::endl;

    ge.lights.ambient_light = Color{0.5, 0.5, 0.5};
    auto color = ge.add_render_pass<ColorPass>(Color{47.0f / 255.0f, 107.0f / 255.0f, 63.0f / 255.0f});

    bool shadow_map_view = false;

    while (ge.is_running()) {
        // rotate cube
        cube->transform.rotation.y += Engine::PI * ge.frame_delta * 0.1f;
        ge.update();

        if (ge.input.just_pressed(SHADOW_MAP_SWITCH)) {
            shadow_map_view = !shadow_map_view;
            if (shadow_map_view) {
                ge.input.set_mouse_mode(Input::NORMAL);
            } else {
                ge.input.set_mouse_mode(Input::DISABLED);
            }
        }

        if (ge.input.just_pressed(EXIT)) {
            break;
        }

        // render pipeline
        color->render();
        shadow->render();

        fo->render();
        if (shadow_map_view) {
            shadow_map_display->render();
        }
        ge.send_to_window();
    }

    return 0;
}