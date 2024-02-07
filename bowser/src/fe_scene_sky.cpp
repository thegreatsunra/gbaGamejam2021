#include "fe_scene_sky.h"

//butano
#include "bn_core.h"
#include "bn_log.h"
#include "bn_keypad.h"
#include "bn_string.h"
#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_affine_bg_ptr.h"
#include "bn_affine_bg_map_ptr.h"
#include "bn_optional.h"
#include "bn_span.h"
#include "bn_affine_bg_map_cell.h"

//fe code
#include "fe_level.h"
#include "fe_player.h"
#include "fe_scene.h"

//assets
#include "bn_sprite_items_bowser_sprite.h"
#include "bn_affine_bg_items_cliffs.h"
#include "bn_regular_bg_items_cliffs_bg.h"

#include "bn_sprite_text_generator.h"

#include "bn_music_items.h"
#include "bn_music_actions.h"

namespace fe
{

Sky::Sky(Player& player)
    : _player(&player) {}

Scene Sky::execute(bn::fixed_point spawn_location)
{
    bn::camera_ptr camera = bn::camera_ptr::create(spawn_location.x(), spawn_location.y());

    bn::music_items::piana.play();
    bn::music::set_volume(0.6);

    // map
    bn::affine_bg_ptr map = bn::affine_bg_items::cliffs.create_bg(512, 512);
    bn::regular_bg_ptr map_bg = bn::regular_bg_items::cliffs_bg.create_bg(512, 512);
    map_bg.set_priority(2);
    map.set_priority(1);
    fe::Level level = fe::Level(map);

    // camera
    map_bg.set_camera(camera);
    map.set_camera(camera);

    // bn::fixed max_cpu_usage;
    // int counter = 1;

    bn::vector<Enemy, 16> enemies = {};

    // _player
    _player->spawn(spawn_location, camera, map, enemies);
    while(true)
    {

        // max_cpu_usage = bn::max(max_cpu_usage, bn::core::last_cpu_usage());
        // --counter;
        // if(! counter)
        // {
        //     BN_LOG("cpu:" + bn::to_string<32>((max_cpu_usage * 100).right_shift_integer()));
        //     max_cpu_usage = 0;
        //     counter = 60;
        // }

        map_bg.set_x(map_bg.x() + 0.3);

        _player->update_position(map, level);
        _player->apply_animation_state();
        // BN_LOG(bn::to_string<32>(_player->pos().x())+" " + bn::to_string<32>(_player->pos().y()));

        if(bn::keypad::up_pressed())
        {
            if(_player->pos().x() < 185 && _player->pos().x() > 155) {
                if(_player->pos().y() < 644 && _player->pos().y() > 614) {
                    _player->delete_data();
                    return Scene::SKY_DUNGEON;
                }
            }

            if(_player->pos().x() < 170 && _player->pos().x() > 140) {
                if(_player->pos().y() < 294 && _player->pos().y() > 264) {
                    _player->delete_data();
                    return Scene::SKY_PATH;
                }
            }
        }


        bn::core::update();
    }
}
}
