#include <iostream>
#include <__msvc_ostream.hpp>

#include "graphicengine.hpp"

Engine ge{"unit-tests", 500, 500};

int main() {
    // DELETING ENTITIES
    // 0 - ID recycling
    const unsigned int ENTITY_COUNT = 10;
    std::vector<geRef<MeshThing>> mesh_refs;
    std::vector<unsigned int> mesh_ref_ids;
    for (unsigned int i = 0; i < ENTITY_COUNT; i++) {
        auto mesh_ref = ge.add<MeshThing>(ge.meshes.cube, ge.shaders.get_base_material(true, true));
        mesh_ref_ids.push_back(mesh_ref.id);
        std::cout << mesh_ref.id << std::endl;
        mesh_refs.push_back(mesh_ref);
    }
    for (auto mesh_ref : mesh_refs) {
        mesh_ref.free();
    }
    for (unsigned int i = 0; i < ENTITY_COUNT; i++) {
        auto mesh_ref = ge.add<MeshThing>(ge.meshes.cube, ge.shaders.get_base_material(true, true));
        assert(mesh_ref.id == mesh_ref_ids[i]);
    }

    return 0;
}