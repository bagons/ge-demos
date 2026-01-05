#include <algorithm>
#include <iostream>
#include <vector>

#include "graphicengine.hpp"
#include <glm/ext.hpp>

Engine ge{"demo project", 800, 600};

struct RenderContext {
    // render pipeline
    geRendRef<ForwardRenderer3DLayer> fr;
    MeshThing* debug_thing = nullptr;
    geRef<Camera> main_cam;
};


void update() {
    ge.update();
}

void render(RenderContext& rctx) {
    glClearColor(0.4f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rctx.fr->render();

    ge.send_it_to_window();
}

void run(RenderContext& rctx) {
    while (ge.is_running()) {
        update();
        render(rctx);
    }
}

/* Player class:
 * moves camera based on user input to create a FPS controller
 */
class Player : public Thing {
    float move_speed = 5.0f;
    float mouse_sensitivity = 0.002f;
    geRef<Camera> player_cam;
public:
    explicit Player(const geRef<Camera>& cam) : Thing(false, true) {
        player_cam = cam;
        player_cam->transform.position = Position{0, 1.8, 0};
    };

    void update() override {
        glm::vec3 move_vec{0, 0, 0};

        if (ge.input.is_pressed(0))
            move_vec.z -= 1.0f;
        if (ge.input.is_pressed(1))
            move_vec.z += 1.0f;

        if (ge.input.is_pressed(2))
            move_vec.x -= 1.0f;
        if (ge.input.is_pressed(3))
            move_vec.x += 1.0f;

        Rotation::rotate_point(0, -player_cam->transform.rotation.y, 0, move_vec);
        player_cam->transform.position = player_cam->transform.position + (move_vec * ge.frame_delta * move_speed);

        if (abs(ge.input.mouse_move_delta.x) > 0 || abs(ge.input.mouse_move_delta.y) > 0) {
            float mouse_move_x = static_cast<float>(ge.input.mouse_move_delta.x) * mouse_sensitivity;
            float mouse_move_y = static_cast<float>(ge.input.mouse_move_delta.y) * mouse_sensitivity;

            player_cam->transform.rotation.y -= mouse_move_x;
            player_cam->transform.rotation.x -= mouse_move_y;
            player_cam->transform.rotation.x = std::clamp(player_cam->transform.rotation.x, -glm::pi<float>() / 2, glm::pi<float>() / 2);
        }
    };
};


int main() {
    // setup actions
    ge.input.set_action_list(std::vector{
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_E
    });
    ge.input.set_mouse_mode(GLFW_CURSOR_DISABLED);

    // PIPELINE INIT
    RenderContext r_ctx;

    // setup camera
    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    r_ctx.fr = ge.add_render_layer<ForwardRenderer3DLayer>(camera);
    r_ctx.main_cam = camera;

    // spawn cube
    std::cout << "random mesh" << std::endl;
    auto special_cube = std::make_shared<Mesh>("res/mesh.obj");
    ge.add<MeshThing>(special_cube, ge.shaders.get_base_material(true, true));

    // spawn FPS controller
    ge.add<Player>(camera);

    // load Spozna Palace
    auto sponza_model = std::make_shared<Model>("res/sponza/sponza.obj");
    auto sponza = ge.add<ModelThing>(sponza_model);
    sponza->transform.scale = glm::vec3(0.02, 0.02, 0.02);

    std::cout << "started running..." << std::endl;

    auto point_light = ge.add<PointLight>(glm::vec3{1.0, 1.0, 1.0}, 10.0);
    point_light->transform.position.y = 5;

    run(r_ctx);

    return 0;
}

