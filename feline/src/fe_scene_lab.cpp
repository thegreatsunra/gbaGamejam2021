#include "fe_scene_lab.h"

//butano
#include "bn_core.h"
#include "bn_log.h"
#include "bn_math.h"
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

#include "bn_blending_actions.h"
#include "bn_sprite_actions.h"

//assets
#include "bn_sprite_items_cat_sprite.h"
#include "bn_affine_bg_items_lab.h"
#include "bn_regular_bg_items_lab_bg.h"

#include "bn_sprite_items_child.h"
#include "bn_sprite_items_blimp.h"

#include "bn_sprite_text_generator.h"
#include "variable_8x8_sprite_font.h"

#include "fe_tooltip.h"

#include "bn_music_items.h"
#include "bn_music_actions.h"

namespace fe
{
    Lab::Lab(Player& player)
    : _player(&player){}

    Scene Lab::execute(bn::fixed_point spawn_location)
    {
        bn::camera_ptr camera = bn::camera_ptr::create(spawn_location.x(), spawn_location.y());

        // spawn_location = bn::fixed_point(447, 661);
        bn::sprite_text_generator text_generator(variable_8x8_sprite_font);
        bn::music_items::wiskedaway.play();
        bn::music::set_volume(0.1);

        // map
        bn::affine_bg_ptr map = bn::affine_bg_items::lab.create_bg(512, 512);
        bn::regular_bg_ptr bg = bn::regular_bg_items::lab_bg.create_bg(0, 0);
        map.set_priority(1);
        fe::Level level = fe::Level(map);

        bn::sprite_ptr blimp = bn::sprite_items::blimp.create_sprite(60, 330);
        blimp.set_bg_priority(0);
        blimp.put_above();
        blimp.set_z_order(0);
        blimp.set_camera(camera);
        // // bn::sprite_ptr glow = bn::sprite_items::glow.create_sprite(903, 152);
        
        // // glow.set_camera(camera);
        // // glow.set_bg_priority(0);
        // // glow.set_scale(0.1);
        // // bn::optional<bn::sprite_scale_to_action> scale_action;
        bn::optional<bn::blending_transparency_alpha_to_action> transparency_action;
        // // glow.put_above();

        // int kill_timer = 0;

        // camera
        // bg.set_camera(camera);
        bg.set_camera(camera);
        map.set_camera(camera);
        
        // bn::fixed max_cpu_usage;
        // int counter = 1;


        bn::vector<Enemy, 16> enemies = {};
        enemies.push_back(Enemy(173, 424, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(507, 328, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(585, 424, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(523, 176, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(885, 528, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(932, 608, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(874, 768, camera, map, ENEMY_TYPE::RAT, 5));
        enemies.push_back(Enemy(-1000, -1000, camera, map, ENEMY_TYPE::MUTANT, 5));

        Enemy& mutant = enemies[7];
        // enemies.push_back(Enemy(673, 384, camera, map, ENEMY_TYPE::BOSS, 5));
        // enemies.push_back(Enemy(292, 288, camera, map, ENEMY_TYPE::BOSS, 5));
        // enemies.push_back(Enemy(337, 104, camera, map, ENEMY_TYPE::BOSS, 5));

        // _player
        _player->spawn(spawn_location, camera, map, enemies);
        _player->set_can_teleport(true);

        bool detected = false;

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

            // int enemy_count = 0;
            // for(Enemy& enemy : enemies){
            //     if(enemy.hp() > 0){
            //         ++enemy_count;
            //     }
            // }
            // if(enemy_count > 0){
            //     if(explain_kill.check_trigger(_player->pos())){
            //         _player->set_listening(true);
            //         explain_kill.update();
            //     }
            //     else {
            //         _player->set_listening(false);
            //     }
            // }
            
            if(!detected){
                for(Enemy& enemy : enemies){
                    if(bn::abs(enemy.pos().x() - camera.x()) < 400 && bn::abs(enemy.pos().y() - camera.y()) < 200){
                        enemy.update(_player->pos());
                        if(enemy.spotted_player() && !detected){
                            detected = true;
                            map.set_blending_enabled(true);
                            transparency_action = bn::blending_transparency_alpha_to_action(60, 0);
                        }
                    } else {
                        enemy.set_visible(false);
                    }
                }

                _player->update_position(map, level);
                _player->apply_animation_state();
            }

            if(girls.check_trigger(_player->pos()))
            {
                if(bn::keypad::up_pressed()){
                    _player->set_listening(true);
                    girls.talk();
                }else if(!girls.is_talking()){
                    _player->set_listening(false);
                }
            } else if(lab_pc.check_trigger(_player->pos()))
            {
                if(bn::keypad::up_pressed()){
                    _player->set_listening(true);
                    lab_pc.talk();
                }else if(!lab_pc.is_talking()){
                    _player->set_listening(false);
                }
            }  else if(computerstuff.check_trigger(_player->pos()))
            {
                if(bn::keypad::up_pressed()){
                    _player->set_listening(true);
                    computerstuff.talk();
                }else if(!computerstuff.is_talking()){
                    _player->set_listening(false);
                }
            }  else if(potion.check_trigger(_player->pos()))
            {
                if(bn::keypad::up_pressed()){
                    _player->set_listening(true);
                    potion.talk();
                }else if(!potion.is_talking()){
                    _player->set_listening(false);
                }
            }  else if(pewpew.check_trigger(_player->pos()))
            {
                if(bn::keypad::up_pressed()){
                    _player->set_listening(true);
                    pewpew.talk();
                }else if(!pewpew.is_talking()){
                    _player->set_listening(false);
                }
            } 
            else if(explain_sneak.check_trigger(_player->pos())){
                _player->set_listening(true);
                explain_sneak.update();
            }
            else {
                _player->set_listening(false);
            }

            girls.update();
            lab_pc.update();
            computerstuff.update();
            potion.update();
            pewpew.update();

            if(transparency_action.has_value() && !transparency_action.value().done()){
                transparency_action.value().update();
            }else if(transparency_action.has_value() && transparency_action.value().done()){
                bn::blending::set_transparency_alpha(1);
                _player->delete_data();
                return Scene::LAB;
            }
            
            // BN_LOG(bn::to_string<32>(_player->pos().x())+", " + bn::to_string<32>(_player->pos().y()));
            
            // if(_player->pos().y() > 700){
            //     _player->delete_data();
            //     return Scene::OTHER;
            // }

            if(mutant.hp() < 1){
                _player->delete_data();
                return Scene::LAB_AFTER;
            }

            bn::core::update();
        }
    }
}
