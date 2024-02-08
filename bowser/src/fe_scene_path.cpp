#include "fe_scene_path.h"

//butano
#include "bn_affine_bg_map_cell.h"
#include "bn_affine_bg_map_ptr.h"
#include "bn_affine_bg_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_core.h"
#include "bn_fixed_point.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_math.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"
#include "bn_span.h"
#include "bn_sprite_ptr.h"
#include "bn_string.h"

//fe code
#include "fe_level.h"
#include "fe_player.h"
#include "fe_scene.h"
// #include "fe_tooltip.h"
#include "fe_enemy.h"
#include "fe_enemy_type.h"

//assets
#include "bn_sprite_items_bowser_sprite.h"
// #include "bn_sprite_items_box.h"
#include "bn_affine_bg_items_path.h"
// #include "bn_regular_bg_items_path_bg.h"

// #include "bn_sprite_text_generator.h"
// #include "bn_music_items.h"
// #include "bn_music_actions.h"

namespace fe {
Path::Path(Player& player)
    : _player(&player) {}

Scene Path::execute(bn::fixed_point spawn_lobowserion) {
    bn::camera_ptr camera = bn::camera_ptr::create(spawn_lobowserion.x(), spawn_lobowserion.y());

    // bn::music_items::piana.play();
    // bn::music::set_volume(0.7);

    // map
    // bn::regular_bg_ptr map_bg = bn::regular_bg_items::path_bg.create_bg(512, 512);
    bn::affine_bg_ptr map = bn::affine_bg_items::path.create_bg(512, 512);
    // map_bg.set_priority(2);
    map.set_priority(1);
    fe::Level level = fe::Level(map);

    // camera
    map.set_camera(camera);
    // map_bg.set_camera(camera);

    // bn::fixed max_cpu_usage;
    // int counter = 1;

    //Enemies
    bn::vector<Enemy, 16> enemies = {};
    enemies.push_back(Enemy(400, 500, camera, map, ENEMY_TYPE::MARIO, 2));
    enemies.push_back(Enemy(500, 500, camera, map, ENEMY_TYPE::MARIO, 2));
    enemies.push_back(Enemy(600, 500, camera, map, ENEMY_TYPE::MARIO, 2));
    enemies.push_back(Enemy(700, 500, camera, map, ENEMY_TYPE::MARIO, 2));
    enemies.push_back(Enemy(800, 500, camera, map, ENEMY_TYPE::MARIO, 2));

    // player
    _player->spawn(spawn_lobowserion, camera, map, enemies);
    while(true) {

        // max_cpu_usage = bn::max(max_cpu_usage, bn::core::last_cpu_usage());
        // --counter;
        // if(! counter)
        // {
        //     BN_LOG("cpu:" + bn::to_string<32>((max_cpu_usage * 100).right_shift_integer()));
        //     max_cpu_usage = 0;
        //     counter = 60;
        // }

        // explain_attack.update();

        //elevator.update_position();
        _player->update_position(map,level);
        _player->apply_animation_state();
        // BN_LOG(bn::to_string<32>(_player->pos().x())+" " + bn::to_string<32>(_player->pos().y()));

        // if(bn::keypad::up_pressed())
        // {
        //     if(_player->pos().x() < 570 && _player->pos().x() > 540){
        //         if(_player->pos().y() < 400 && _player->pos().y() > 360){
        //             _player->delete_data();
        //             return Scene::PATH_SKY;
        //         }
        //     }
        // }

        for(Enemy& enemy : enemies) {
            if(bn::abs(enemy.pos().x() - camera.x()) < 200 && bn::abs(enemy.pos().y() - camera.y()) < 100) {
                enemy.update(_player->pos());
            } else {
                enemy.set_visible(false);
            }
        }

        bn::core::update();
    }
}
}
