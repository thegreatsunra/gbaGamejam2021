#include "bn_core.h"
#include "bn_math.h"
#include "bn_sprite_builder.h"
#include "bn_sprite_ptr.h"
#include "bn_keypad.h"
#include "bn_string.h"
#include "bn_regular_bg_ptr.h"
#include "bn_fixed_point.h"
#include "bn_regular_bg_item.h"
#include "bn_affine_bg_ptr.h"
#include "bn_affine_bg_item.h"
#include "bn_affine_bg_tiles_ptr.h"
#include "bn_affine_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_camera_actions.h"
#include "bn_sprite_animate_actions.h"

#include "bn_string_view.h"
#include "bn_vector.h"
#include "bn_log.h"
#include "bn_sprite_text_generator.h"

#include "fe_hitbox.h"
#include "fe_scene_sky.h"
#include "fe_scene_other.h"
#include "fe_scene_path.h"
#include "fe_scene_loading.h"
#include "fe_scene_title.h"
#include "fe_scene_gba.h"
#include "fe_scene_lab.h"
#include "fe_scene_lab_after.h"
#include "fe_scene.h"

#include "bn_sprite_items_bowser_sprite.h"

int main() {
    bn::core::init();
    // fe::Scene scene = fe::Scene::GBA;

    bn::sprite_ptr bowser_sprite = bn::sprite_items::bowser_sprite.create_sprite(0, 0);
    bowser_sprite.set_visible(false);
    fe::Player player = fe::Player(bowser_sprite);

    while (true) {
        fe::Path path = fe::Path(player);
        path.execute(bn::fixed_point(100, 540));
        // if(scene == fe::Scene::PATH_START){
        //     fe::Path path = fe::Path(player);
        //     scene = path.execute(bn::fixed_point(293, 368));
        // }
        // else if(scene == fe::Scene::PATH_SKY){
        //     fe::Sky sky = fe::Sky(player);
        //     scene = sky.execute(bn::fixed_point(153, 272));
        // }
        // else if(scene == fe::Scene::DUNGEON_SKY)
        // {
        //     fe::Sky sky = fe::Sky(player);
        //     scene = sky.execute(bn::fixed_point(169, 616));
        // }
        // else if(scene == fe::Scene::SKY_PATH)
        // {
        //     fe::Path path = fe::Path(player);
        //     scene = path.execute(bn::fixed_point(555, 384));
        // }
        // else if(scene == fe::Scene::TITLE)
        // {
        //     fe::Title title = fe::Title();
        //     scene = title.execute();
        // }
        // else if(scene == fe::Scene::GBA)
        // {
        //     fe::GBA gba = fe::GBA();
        //     scene = gba.execute();
        // }
        // else if(scene == fe::Scene::OTHER)
        // {
        //     fe::Other other = fe::Other(player);
        //     scene = other.execute(bn::fixed_point(235, 590));
        // }
        player.delete_data();
        player.hide();
        // if(scene != fe::Scene::TITLE){
        //     fe::Loading loading = fe::Loading();
        //     loading.execute(scene);
        // }
        bn::core::update();

    }

}
