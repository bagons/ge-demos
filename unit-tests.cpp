#include <iostream>
#include <algorithm>
#include "graphicengine.hpp"

EngineSettings settings = {.fullscreen = false};
Engine ge{"unit-tests", 800, 600, settings};

void id_recycling_test(const unsigned int entity_count) {
    std::vector<geRef<MeshThing>> mesh_refs;
    std::vector<unsigned int> mesh_ref_ids;
    for (unsigned int i = 0; i < entity_count; i++) {
        auto mesh_ref = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material(true, true));
        mesh_ref_ids.push_back(mesh_ref.id);
        mesh_refs.push_back(mesh_ref);
    }
    for (auto mesh_ref : mesh_refs) {
        mesh_ref.free();
    }
    for (unsigned int i = 0; i < entity_count; i++) {
        auto mesh_ref = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material(true, true));
        assert(mesh_ref.id == mesh_ref_ids[i]);
        mesh_refs[i] = mesh_ref;
    }
    for (auto mesh_ref : mesh_refs) {
        mesh_ref.free();
    }
    std::cout << "id_recycling_test: PASSED" << std::endl;
}

void shader_program_deletion_test() {
    auto vs = Shader("res_demo0/vertex.glsl", Shader::VERTEX_SHADER);
    auto fs = Shader("res_demo0/fragment.glsl", Shader::FRAGMENT_SHADER);
    unsigned int shader_program_id = 0;
    {
        auto sp = ShaderProgram(vs, fs);
        shader_program_id = sp.get_id();
        assert(ge.shaders.get_shader_use_by_id(sp.get_id()) == 1);
    }
    assert(ge.shaders.get_shader_use_by_id(shader_program_id) == -1);
    std::cout << "shader_program_deletion_test: PASSED" << std::endl;
}

class SpHolder {
public:
    ShaderProgram shader_program;
    explicit SpHolder(const ShaderProgram& sp) : shader_program(sp) {

    }
};

void shader_program_transport_test() {
    auto vs = Shader("res_demo0/vertex.glsl", Shader::VERTEX_SHADER);
    auto fs = Shader("res_demo0/fragment.glsl", Shader::FRAGMENT_SHADER);

    auto shader_program_copy = [](ShaderProgram sp) {
        //auto mat = std::make_shared<Material>(sp);
        std::cout << ge.shaders.get_shader_use_by_id(sp.get_id()) << std::endl;
        assert(ge.shaders.get_shader_use_by_id(sp.get_id()) == 2);
    };

    auto shader_program_const_ref = [](const ShaderProgram& sp) {
        sp.set_uniform("test", 10.0f);
        assert(ge.shaders.get_shader_use_by_id(sp.get_id()) == 1);
    };

    unsigned int shader_program_id = 0;
    {
        auto sp = ShaderProgram(vs, fs);
        shader_program_id = sp.get_id();
        shader_program_copy(sp);
        assert(ge.shaders.get_shader_use_by_id(sp.get_id()) == 1);
        shader_program_const_ref(sp);

        auto sp2 = sp;
        assert(ge.shaders.get_shader_use_by_id(sp2.get_id()) == 2);
    }
    assert(ge.shaders.get_shader_use_by_id(shader_program_id) == -1);

    {
        // Copy assignment
        auto sp1 = ShaderProgram(vs, fs);
        const unsigned int sp1_id = sp1.get_id();
        auto sp2 = ShaderProgram(vs, fs);
        const unsigned int sp2_id = sp2.get_id();
        sp1 = sp2;
        assert(ge.shaders.get_shader_use_by_id(sp1_id) == -1);
        assert(ge.shaders.get_shader_use_by_id(sp2_id) == 2);
    }

    // Temporaries
    auto shader_program_make = []() {
        auto vs = Shader("res_demo0/vertex.glsl", Shader::VERTEX_SHADER);
        auto fs = Shader("res_demo0/fragment.glsl", Shader::FRAGMENT_SHADER);
        auto sp = ShaderProgram(vs, fs);
        return sp;
    };

    ShaderProgram sp3 = shader_program_make();
    assert(ge.shaders.get_shader_use_by_id(sp3.get_id()) == 1);

    // Move construction
    {
        auto sp1 = ShaderProgram(vs, fs);
        auto sp2 = std::move(sp1);
        assert(ge.shaders.get_shader_use_by_id(sp2.get_id()) == 1);
        assert(sp1.get_id() == -1);
    }

    // Move assignment
    {
        auto sp1 = ShaderProgram(vs, fs);
        const unsigned int sp1_id = sp1.get_id();
        auto sp2 = ShaderProgram(vs, fs);
        sp1 = std::move(sp2);
        assert(ge.shaders.get_shader_use_by_id(sp1.get_id()) == 1);
        assert(ge.shaders.get_shader_use_by_id(sp1_id) == -1);
        assert(sp2.get_id() == -1);
    }

    // vector stuff
    {
        auto sp = shader_program_make();
        std::vector<ShaderProgram> sps = {};
        sps.emplace_back(sp);
        sps.emplace_back(sp);
        sps.emplace_back(sp);
        std::cout << sp.get_id() << " " << ge.shaders.get_shader_use_by_id(sp.get_id()) << std::endl;
        assert(ge.shaders.get_shader_use_by_id(sp.get_id()) == 4);
    }

    std::cout << "shader_program_transport_test: PASSED" << std::endl;
}


void light_removal_test(unsigned int light_count = 10) {
    std::vector<geRef<PointLight>> pl_lights;
    std::vector<geRef<DirectionalLight>> dl_lights;
    std::vector<geRef<SpotLight>> sl_lights;

    for (unsigned int i = 0; i < light_count; i++) {
        auto ptl = ge.add<PointLight>(Color::WHITE, 1.0f);
        pl_lights.push_back(ptl);
        auto dl = ge.add<DirectionalLight>(Color::WHITE, 1.0f, Vector3{0.0f, 0.0f, 1.0f});
        dl_lights.push_back(dl);
        auto sl = ge.add<SpotLight>(Color::WHITE, 1.0f, Engine::PI);
        sl_lights.push_back(sl);
    }

    for (unsigned int i = 0; i < light_count; i++) {
        pl_lights[i].free();
        dl_lights[i].free();
        sl_lights[i].free();
    }

    assert(ge.lights.get_point_light_count() == 0);
    assert(ge.lights.get_directional_light_count() == 0);
    assert(ge.lights.get_spot_light_count() == 0);
    std::cout << "light_removal_test: PASSED" << std::endl;
}


enum Actions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    EXIT,
    FULLSCREEN,
    CLICK
};

class FPC final : public Thing {
    float move_speed = 10.0f;
    float mouse_sensitivity = 0.002f;
    geRef<Camera> player_cam;
public:
    explicit FPC(const geRef<Camera>& cam) {
        player_cam = cam;
        player_cam->transform.position = Position{0.0f, 1.8f, 0.0f};
        ge.input.set_mouse_mode(Input::MouseMode::DISABLED);
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
            const float mouse_move_x = ge.input.mouse_move_delta.x * mouse_sensitivity;
            const float mouse_move_y = ge.input.mouse_move_delta.y * mouse_sensitivity;

            player_cam->transform.rotation.y -= mouse_move_x;
            player_cam->transform.rotation.x -= mouse_move_y;
            player_cam->transform.rotation.x = std::clamp(player_cam->transform.rotation.x, -Engine::PI / 2, Engine::PI / 2);
        }
    };
};

int main() {
    std::cout << "STARTING AUTO TESTS:" << std::endl;
    // DELETING ENTITIES
    id_recycling_test(10);

    // SHADER PROGRAM USE COUNTING
    shader_program_deletion_test();
    shader_program_transport_test();
    light_removal_test();

    std::cout << "STARTING MANUAL TESTS: " << std::endl;

    ge.input.set_action_list(std::vector{
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_ESCAPE,
        GLFW_KEY_F11,
        GLFW_MOUSE_BUTTON_LEFT
    });

    //
    ge.lights.ambient_light = Color{1.0f, 1.0f, 1.0f, 1.0f};
    auto camera = ge.add<Camera>(70.0f, 0.1f, 1000.0f);
    ge.add<FPC>(camera);
    auto color_pass = ge.add_render_pass<ColorPass>(Color::BLACK);
    auto forward_pass = ge.add_render_pass<ForwardOpaque3DPass>(camera);

    auto ground_mat = ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL)->copy();
    auto ground = ge.add<MeshThing>(ge.meshes.get_plane(), ground_mat);
    ground->transform.scale = Scale{30.0f, 1.0f, 30.0f};

    auto sphere_thing = ge.add<MeshThing>(ge.meshes.get_sphere(), ground_mat);

    sphere_thing->transform.position = Position{0.0f, 3.0f, 0.0f};

    auto light = ge.add<SpotLight>(Color::WHITE, 1.0f, Engine::PI / 4);
    light->transform.position = Position{0.0f, 10.0f, 0.0f};

    while (ge.is_running()){
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
        forward_pass->render();
        ge.send_to_window();
    }

    return 0;
}