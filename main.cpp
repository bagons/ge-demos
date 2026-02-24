#include <algorithm>
#include <iostream>
#include <vector>

#include "graphicengine.hpp"
#include <glm/ext.hpp>

Engine ge{"demo project", 800, 600, 16, 3};

struct RenderContext {
    // render pipeline
    geRendRef<ForwardOpaque3DPass> fr;
    MeshThing* debug_thing = nullptr;
    geRef<Camera> main_cam;
};


void update() {
    ge.update();
}

void render(RenderContext& rctx) {
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
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
    geRef<ModelThing> sponza;
public:
    explicit Player(const geRef<Camera>& cam, const geRef<ModelThing>& _sponza) {
        player_cam = cam;
        player_cam->transform.position = Position{0.0f, 1.8f, 0.0f};
        sponza = _sponza;
    };

    void update() override {
        Vector3 move_vec{0, 0, 0};

        if (ge.input.is_pressed(0))
            move_vec.z -= 1.0f;
        if (ge.input.is_pressed(1))
            move_vec.z += 1.0f;

        if (ge.input.is_pressed(2))
            move_vec.x -= 1.0f;
        if (ge.input.is_pressed(3))
            move_vec.x += 1.0f;

        if (ge.input.is_pressed(4) and sponza.id != -1) {
            ge.queue_remove_thing(sponza.id);
            sponza.id = -1;
        }

        Rotation::rotate_point(0, -player_cam->transform.rotation.y, 0, move_vec);
        move_vec *= ge.frame_delta * move_speed;
        player_cam->transform.position = player_cam->transform.position + move_vec;

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
    r_ctx.fr = ge.add_render_layer<ForwardOpaque3DPass>(camera);
    r_ctx.main_cam = camera;

    // load Spozna Palace
    auto sponza_model = std::make_shared<Model>("res/sponza/sponza.obj");
    auto sponza = ge.add<ModelThing>(sponza_model);
    sponza->transform.scale = Scale{0.02f, 0.02f, 0.02f};

    // spawn FPS controller
    ge.add<Player>(camera, sponza);

    ge.add<DirectionalLight>(Color::WHITE, 0.8f, Vector3{1.0f, -1.0f, 1.0f});
    ge.add<DirectionalLight>(Color::ORANGE, 0.1f, Vector3{1.0f, -0.2f, -1.0f});
    ge.add<DirectionalLight>(Color::TEAL, 0.05f, Vector3{-1.0f, -0.2f, 1.0f});

    auto cube = ge.add<MeshThing>(ge.meshes.cube, ge.shaders.get_base_material(true, true));
    cube->transform.position = Position{0, 1, 0};

    std::cout << "started running..." << std::endl;

    auto point_light = ge.add<PointLight>(Color{1.0f, 1.0f, 1.0f}, 10.0f);
    point_light->transform.position.y = 5;

    run(r_ctx);

    return 0;
}

