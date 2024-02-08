#include "fe_scene_loading.h"

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
#include "bn_sprite_animate_actions.h"
#include "bn_random.h"

//fe code
#include "fe_scene.h"

//assets
#include "bn_sprite_items_bowser_sprite.h"
#include "bn_sprite_items_bowser_sprite_other.h"
#include "bn_regular_bg_items_loading_bg.h"
#include "bn_regular_bg_items_lab_bg.h"
#include "bn_regular_bg_items_loading_bg_other.h"

#include "bn_music_items.h"
#include "bn_music_actions.h"

#include "bn_music_items.h"

namespace fe {
void Loading::execute(Scene next_scene) {
    bn::fixed_point init_pos = bn::fixed_point(0, 0);

    if(next_scene == Scene::OTHER) {
        bn::music_items::crystal.play();
    } else {
        bn::music::set_volume(0.3);
    }

    // player sprite
    bn::sprite_ptr bowser_sprite1 = bn::sprite_items::bowser_sprite.create_sprite(init_pos.x(), init_pos.y()-50);
    bn::sprite_ptr bowser_sprite2 = bn::sprite_items::bowser_sprite.create_sprite(init_pos.x(), init_pos.y()-25);
    bn::sprite_ptr bowser_sprite3 = bn::sprite_items::bowser_sprite.create_sprite(init_pos.x(), init_pos.y()+25);
    bn::sprite_ptr bowser_sprite4 = bn::sprite_items::bowser_sprite.create_sprite(init_pos.x(), init_pos.y()+50);
    bn::sprite_ptr bowser_sprite5 = bn::sprite_items::bowser_sprite.create_sprite(init_pos.x(), init_pos.y());

    bn::sprite_animate_action<10> action1 = bn::create_sprite_animate_action_forever(
            bowser_sprite1, 2, bn::sprite_items::bowser_sprite.tiles_item(), 8, 9,10,11, 2, 3, 4, 5, 6,7);
    bn::sprite_animate_action<10> action2 = bn::create_sprite_animate_action_forever(
            bowser_sprite2, 2, bn::sprite_items::bowser_sprite.tiles_item(), 8, 9,10,11, 2, 3, 4, 5, 6,7);
    bn::sprite_animate_action<10> action3 = bn::create_sprite_animate_action_forever(
            bowser_sprite3, 2, bn::sprite_items::bowser_sprite.tiles_item(), 8, 9,10,11, 2, 3, 4, 5, 6,7);
    bn::sprite_animate_action<10> action4 = bn::create_sprite_animate_action_forever(
            bowser_sprite4, 2, bn::sprite_items::bowser_sprite.tiles_item(), 8, 9,10,11, 2, 3, 4, 5, 6,7);
    bn::sprite_animate_action<10> action5 = bn::create_sprite_animate_action_forever(
            bowser_sprite5, 2, bn::sprite_items::bowser_sprite.tiles_item(), 8, 9,10,11, 2, 3, 4, 5, 6,7);

    bn::camera_ptr camera = bn::camera_ptr::create(init_pos.x()+100, init_pos.y());

    // map
    bn::regular_bg_ptr map = bn::regular_bg_items::loading_bg.create_bg(512, 512);
    // map.set_horizontal_scale(2);

    if(next_scene == Scene::OTHER) {
        bowser_sprite1.set_position(200, init_pos.y()-50);
        bowser_sprite2.set_position(200, init_pos.y()+50);
        bowser_sprite3.set_position(200, init_pos.y()-25);
        bowser_sprite4.set_position(200, init_pos.y()+25);
        bowser_sprite5.set_position(200, init_pos.y());

        action1 = bn::create_sprite_animate_action_forever(
                      bowser_sprite1, 4, bn::sprite_items::bowser_sprite_other.tiles_item(), 7, 6,5,4, 3, 2, 11, 10, 9,8);
        action2 = bn::create_sprite_animate_action_forever(
                      bowser_sprite2, 4, bn::sprite_items::bowser_sprite_other.tiles_item(), 7, 6,5,4, 3, 2, 11, 10, 9,8);
        action3 = bn::create_sprite_animate_action_forever(
                      bowser_sprite3, 4, bn::sprite_items::bowser_sprite_other.tiles_item(), 7, 6,5,4, 3, 2, 11, 10, 9,8);
        action4 = bn::create_sprite_animate_action_forever(
                      bowser_sprite4, 4, bn::sprite_items::bowser_sprite_other.tiles_item(), 7, 6,5,4, 3, 2, 11, 10, 9,8);
        action5 = bn::create_sprite_animate_action_forever(
                      bowser_sprite5, 4, bn::sprite_items::bowser_sprite_other.tiles_item(), 7, 6,5,4, 3, 2, 11, 10, 9,8);

        map = bn::regular_bg_items::loading_bg_other.create_bg(512, 512);
    } else if(next_scene == Scene::LAB) {
        map = bn::regular_bg_items::lab_bg.create_bg(100, 0);
    } else if(next_scene == Scene::LAB_AFTER) {
        return;
    }

    // camera
    bowser_sprite1.set_camera(camera);
    bowser_sprite2.set_camera(camera);
    bowser_sprite3.set_camera(camera);
    bowser_sprite4.set_camera(camera);
    bowser_sprite5.set_camera(camera);
    map.set_camera(camera);

    for(int i = 0; i < 160; ++i) {
        if(next_scene != Scene::OTHER) {
            bowser_sprite1.set_x(bowser_sprite1.x() + 1.3);
            bowser_sprite2.set_x(bowser_sprite2.x() + 2);
            bowser_sprite3.set_x(bowser_sprite3.x() + 1.5);
            bowser_sprite4.set_x(bowser_sprite4.x() + 2.2);
            bowser_sprite5.set_x(bowser_sprite5.x() + 1.7);
        } else {
            bowser_sprite1.set_x(bowser_sprite1.x() - 1.3);
            bowser_sprite2.set_x(bowser_sprite2.x() - 2);
            bowser_sprite3.set_x(bowser_sprite3.x() - 1.5);
            bowser_sprite4.set_x(bowser_sprite4.x() - 2.2);
            bowser_sprite5.set_x(bowser_sprite5.x() - 1.7);
        }

        action1.update();
        action2.update();
        action3.update();
        action4.update();
        action5.update();
        bn::core::update();
    }
}
}
