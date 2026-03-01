#include <iostream>

#include "graphicengine.hpp"

Engine ge{"normal maps test", 500, 500};

enum Actions {
    SWITCH
};

int main() {
    ge.input.set_action_list({
        GLFW_KEY_SPACE
    });

    ge.lights.ambient_light = Color::WHITE;

    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    camera->transform.position = Position{0.0f, 3.0f, 10.0f};
    camera->transform.rotation = Rotation{Engine::PI / 12, 0, 0.0f};
    auto opaque_pass = ge.add_render_pass<ForwardOpaque3DPass>(camera);
    auto bg = ge.add_render_pass<ColorPass>(Color::BLACK);

    auto cube_model = std::make_shared<Model>("res_demo2/box.obj");
    auto cube = ge.add<ModelThing>(cube_model);
    cube->transform.scale = Scale{2.0f};


    auto point_light = ge.add<PointLight>(Color::WHITE, 30.0f);
    point_light->transform.position.z = 4.0f;

    bool bump_map_switch = true;

    while (ge.is_running()) {
        ge.pool_inputs();
        if (ge.input.just_pressed(SWITCH)) {
            std::cout << "pressed SWITCH" << std::endl;
            bump_map_switch = !bump_map_switch;
            //mat->set_uniform("material.bump_map_strength", bump_map_switch ? 6.0f : 0.0f);
        }

        ge.update();

        cube->transform.rotation.y = static_cast<float>(ge.get_game_time() * 0.1);

        // render
        bg->render();
        opaque_pass->render();

        ge.send_to_window();
    }

    return 0;
}