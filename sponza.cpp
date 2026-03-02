#include <algorithm>
#include <iostream>
#include <vector>

#include "graphicengine.hpp"

EngineSettings settings{.MAX_NR_POINT_LIGHTS = 1, .MAX_NR_DIRECTIONAL_LIGHTS = 3, .MAX_NR_SPOT_LIGHTS = 1};
Engine ge{"demo project", 800, 600, settings};

constexpr float GRAVITY = 32.0f;

enum Actions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    JUMP,
    SPRINT,
    EXIT,
    CLICK,
    FULLSCREEN
};

class Player : public Thing {
    float move_speed = 6.0f;
    float sprint_multiplier = 1.5f;
    float mouse_sensitivity = 0.002f;
    float height = 1.8f;
    geRef<Camera> camera;

    float y_velocity = 0.0f;
public:
    explicit Player(const geRef<Camera>& cam, const float player_height = 1.8f) {
        height = player_height;
        camera = cam;
        camera->transform.position = Position{-20.0f, height, 0.0f};
        camera->transform.rotation = Rotation{0, Engine::PI / 2, 0.0f};
    };

    void update() override {
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

        if (ge.input.just_pressed(JUMP) and abs(camera->transform.position.y - height) < 0.1) {
            y_velocity = 10.0f;
        }

        Rotation::rotate_point(0, -camera->transform.rotation.y, 0, move_vec);
        move_vec.normalize();
        move_vec *= ge.frame_delta * (ge.input.is_pressed(SPRINT) ? move_speed * sprint_multiplier : move_speed);

        if (abs(y_velocity) > 0.1f) {
            move_vec.y += ge.frame_delta * y_velocity;
        }

        camera->transform.position = camera->transform.position + move_vec;

        if (camera->transform.position.y <= height) {
            camera->transform.position.y = height;
            y_velocity = 0;
        } else {
            y_velocity -= ge.frame_delta * GRAVITY;
        }


        // looking around
        if (ge.input.get_mouse_mode() == Input::DISABLED and (abs(ge.input.mouse_move_delta.x) > 0 || abs(ge.input.mouse_move_delta.y) > 0)) {
            float mouse_move_x = ge.input.mouse_move_delta.x * mouse_sensitivity;
            float mouse_move_y = ge.input.mouse_move_delta.y * mouse_sensitivity;

            camera->transform.rotation.y -= mouse_move_x;
            camera->transform.rotation.x -= mouse_move_y;
            camera->transform.rotation.x = std::clamp(camera->transform.rotation.x, -Engine::PI / 2, Engine::PI / 2);
        }
        // bounds
        camera->transform.position.x = std::clamp(camera->transform.position.x, -28.3f, 25.7f);
        camera->transform.position.z = std::clamp(camera->transform.position.z, -12.4f, 11.3f);
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
    }
};

int main() {
    // setup actions
    ge.input.set_action_list(std::vector{
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_SPACE,
        GLFW_KEY_LEFT_SHIFT,
        GLFW_KEY_ESCAPE,
        GLFW_MOUSE_BUTTON_LEFT,
        GLFW_KEY_F11,
    });
    ge.input.set_mouse_mode(Input::DISABLED);

    // setup camera
    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    auto color_pass = ge.add_render_pass<ColorPass>(Color{0.6f, 0.8f, 1.0f});
    auto forward_pass = ge.add_render_pass<ForwardOpaque3DPass>(camera);

    // load Spozna Palace
    auto sponza_model = std::make_shared<Model>("res_demo1/sponza/sponza.obj");
    auto sponza = ge.add<ModelThing>(sponza_model);
    sponza->transform.scale = Scale{0.02f, 0.02f, 0.02f};

    // lighting
    ge.lights.ambient_light = Color{0.2f, 0.2f, 0.2f};
    ge.add<DirectionalLight>(Color{0.9f, 0.9f, 0.9f, 1.0f}, 1.0f, Vector3{1.0f, -1.0f, 1.0f});
    ge.add<DirectionalLight>(Color::ORANGE, 1.0f, Vector3{1.0f, -0.2f, -1.0f});
    ge.add<DirectionalLight>(Color::TEAL, 1.0f, Vector3{1.0f, -0.2f, 1.0f});

    ge.add<SpinSpot>(Color::ORANGE, 1.0f, Engine::PI/4);

    std::cout << "started running..." << std::endl;
    ge.add<Player>(camera);
    //ge.window.set_fullscreen(true);

    while (ge.is_running()) {
        ge.pool_inputs();

        if (ge.input.just_pressed(EXIT)) {
            ge.input.set_mouse_mode(Input::MouseMode::NORMAL);
        }

        if (ge.input.just_pressed(CLICK)) {
            ge.input.set_mouse_mode(Input::MouseMode::DISABLED);
        }

        if (ge.input.just_pressed(FULLSCREEN)) {
            ge.window.set_fullscreen(!ge.window.is_fullscreen());
        }

        ge.update();
        color_pass->render();
        forward_pass->render();
        ge.send_to_window();
    }

    return 0;
}

