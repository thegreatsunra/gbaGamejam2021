#include "fe_scene_house.h"

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
#include "bn_sprite_items_cat_sprite.h"
// #include "bn_sprite_items_box.h"
#include "bn_affine_bg_items_house_2x.h"
// #include "bn_regular_bg_items_house_bg.h"

// #include "bn_sprite_text_generator.h"
// #include "bn_music_items.h"
// #include "bn_music_actions.h"

namespace fe
{
    House::House(Player& player)
    : _player(&player){}

    Scene House::execute(bn::fixed_point spawn_location)
    {
        bn::camera_ptr camera = bn::camera_ptr::create(spawn_location.x(), spawn_location.y());

        // bn::music_items::piana.play();
        // bn::music::set_volume(0.7);

        // map
        // bn::regular_bg_ptr map_bg = bn::regular_bg_items::house_bg.create_bg(512, 512);
        bn::affine_bg_ptr map = bn::affine_bg_items::house_2x.create_bg(512, 512);
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
        enemies.push_back(Enemy(290, 169, camera, map, ENEMY_TYPE::SLIME, 2));
        enemies.push_back(Enemy(750, 480, camera, map, ENEMY_TYPE::SLIME, 2));
        enemies.push_back(Enemy(711, 224, camera, map, ENEMY_TYPE::SLIME, 2));
        enemies.push_back(Enemy(412, 440, camera, map, ENEMY_TYPE::SLIME, 2));
        enemies.push_back(Enemy(337, 792, camera, map, ENEMY_TYPE::SLIME, 2));

        // player
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

            // explain_attack.update();

            //elevator.update_position();
            _player->update_position(map,level);
            _player->apply_animation_state();
            // BN_LOG(bn::to_string<32>(_player->pos().x())+" " + bn::to_string<32>(_player->pos().y()));
            
            if(bn::keypad::up_pressed())
            {
                if(_player->pos().x() < 570 && _player->pos().x() > 540){
                    if(_player->pos().y() < 400 && _player->pos().y() > 360){
                        _player->delete_data();
                        return Scene::HOUSE_SKY;
                    }
                }
            }

            for(Enemy& enemy : enemies){
                if(bn::abs(enemy.pos().x() - camera.x()) < 200 && bn::abs(enemy.pos().y() - camera.y()) < 100){
                    enemy.update(_player->pos());
                } else {
                    enemy.set_visible(false);
                }
            }
            
            bn::core::update();
        }
    }
}
