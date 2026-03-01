#include <algorithm>
#include <iostream>
#include <vector>

#include "graphicengine.hpp"

EngineSettings settings{.gamma_correction = true};
Engine ge{"demo project", 800, 600, settings};


enum Actions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

struct RenderContext {
    // render pipeline
    geRendRef<ForwardOpaque3DPass> fp;
    geRendRef<ColorPass> cp;
    MeshThing* debug_thing = nullptr;
    geRef<Camera> main_cam;
};


void update() {
    ge.update();
}

void render(RenderContext& rctx) {
    rctx.cp->render();
    rctx.fp->render();
    ge.send_to_window();
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
    explicit Player(const geRef<Camera>& cam) {
        player_cam = cam;
        player_cam->transform.position = Position{0.0f, 1.8f, 0.0f};
    };

    void update() override {
        Vector3 move_vec{0, 0, 0};

        if (ge.input.is_pressed(FORWARD))
            move_vec.z -= 1.0f;
        if (ge.input.is_pressed(BACKWARD))
            move_vec.z += 1.0f;

        if (ge.input.is_pressed(LEFT))
            move_vec.x -= 1.0f;
        if (ge.input.is_pressed(RIGHT))
            move_vec.x += 1.0f;

        Rotation::rotate_point(0, -player_cam->transform.rotation.y, 0, move_vec);
        move_vec *= ge.frame_delta * move_speed;
        player_cam->transform.position = player_cam->transform.position + move_vec;

        if (abs(ge.input.mouse_move_delta.x) > 0 || abs(ge.input.mouse_move_delta.y) > 0) {
            float mouse_move_x = ge.input.mouse_move_delta.x * mouse_sensitivity;
            float mouse_move_y = ge.input.mouse_move_delta.y * mouse_sensitivity;

            player_cam->transform.rotation.y -= mouse_move_x;
            player_cam->transform.rotation.x -= mouse_move_y;
            player_cam->transform.rotation.x = std::clamp(player_cam->transform.rotation.x, -Engine::PI / 2, Engine::PI / 2);
        }
    };
};

class SpinSpot : public SpotLight {
public:
    SpinSpot(Color color, float intensity, float angle) : SpotLight(color, intensity, angle) {
        transform.position.y = 5;
    };

    void update() override {
        transform.position.x = sinf(static_cast<float>(ge.get_game_time())) * 3.0f;
        transform.position.z = cosf(static_cast<float>(ge.get_game_time())) * 3.0f;
        //transform.rotation.z = static_cast<float>(ge.get_game_time()) * 0.1f;
    }
};

int main() {
    // setup actions
    ge.input.set_action_list(std::vector{
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D
    });
    ge.input.set_mouse_mode(Input::DISABLED);

    // PIPELINE INIT
    RenderContext r_ctx;

    // setup camera
    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    auto color = Color{0.6f, 0.8f, 1.0f};
    r_ctx.cp = ge.add_render_pass<ColorPass>(color);
    std::cout << "COLOR" << r_ctx.cp->color.r << " " << r_ctx.cp->color.g << " " << r_ctx.cp->color.b << std::endl;
    r_ctx.fp = ge.add_render_pass<ForwardOpaque3DPass>(camera);
    r_ctx.main_cam = camera;

    // load Spozna Palace
    auto sponza_model = std::make_shared<Model>("res_demo1/sponza/sponza.obj");
    auto sponza = ge.add<ModelThing>(sponza_model);
    sponza->transform.scale = Scale{0.02f, 0.02f, 0.02f};

    // cube
    auto cube_mesh = std::make_shared<Mesh>("res_demo1/box.obj");
    // coordinates
    auto Z_mat = ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL)->copy();
    auto z = ge.add<MeshThing>(cube_mesh, Z_mat);
    Z_mat->set_uniform("material.albedo_color", Color::BLUE.no_alpha());
    z->transform.position = Position{0, 0, 1};
    z->transform.scale = Scale{0.1f};

    auto X_mat = Z_mat->copy();
    X_mat->set_uniform("material.albedo_color", Color::RED.no_alpha());
    auto x = ge.add<MeshThing>(cube_mesh, X_mat);
    x->transform.position = Position{1, 0, 0};
    x->transform.scale = Scale{0.1f};

    // spawn FPS controller

    ge.lights.ambient_light = Color{0.2f, 0.2f, 0.2f};
    ge.add<DirectionalLight>(Color{0.9f, 0.9f, 0.9f, 1.0f}, 1.0f, Vector3{1.0f, -1.0f, 1.0f});
    ge.add<DirectionalLight>(Color::ORANGE, 1.0f, Vector3{1.0f, -0.2f, -1.0f});
    ge.add<DirectionalLight>(Color::TEAL, 1.0f, Vector3{1.0f, -0.2f, 1.0f});

    ge.add<SpinSpot>(Color::ORANGE, 1.0f, Engine::PI/4);


    //auto pl = ge.add<PointLight>(Color::WHITE, 1.0f);
    //pl->transform.position.y = 3;



    std::cout << "started running..." << std::endl;

    ge.add<Player>(camera);

    run(r_ctx);

    return 0;
}

