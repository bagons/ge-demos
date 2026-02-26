#include <iostream>

#include "graphicengine.hpp"

Engine ge{"normal maps test", 500, 500};

int main() {
    ge.lights.ambient_light = Color::WHITE;

    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    camera->transform.position = Position{0.0f, 3.0f, 10.0f};
    camera->transform.rotation = Rotation{Engine::PI / 12, 0, 0.0f};
    auto opaque_pass = ge.add_render_layer<ForwardOpaque3DPass>(camera);

    auto cube_model = std::make_shared<Model>("res_demo2/box.obj");
    auto cube = ge.add<ModelThing>(cube_model);
    cube->transform.scale = Scale{2.0f};

    auto cube_mesh = std::make_shared<Mesh>("res_demo2/box.obj", true);

    auto point_light = ge.add<PointLight>(Color::WHITE, 30.0f);
    point_light->transform.position.z = 4.0f;
    auto light_indicator = ge.add<MeshThing>(cube_mesh, ge.shaders.get_base_material(true, true, true));
    light_indicator->transform.scale = Scale{0.1f};
    light_indicator->transform.position.z = point_light->transform.position.z;

    while (ge.is_running()) {
        cube->transform.rotation.y = static_cast<float>(ge.get_game_time() * 0.1);

        ge.update();

        // render

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.0745f, 0.659f, 1.0f, 0.353f);

        opaque_pass->render();

        ge.send_it_to_window();
    }

    return 0;
}