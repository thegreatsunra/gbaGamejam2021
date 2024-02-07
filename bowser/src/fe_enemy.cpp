#include "fe_enemy.h"
#include "fe_enemy_type.h"
#include "fe_hitbox.h"
#include "fe_collision.h"

#include "bn_optional.h"
#include "bn_math.h"
#include "bn_log.h"
#include "bn_display.h"
#include "bn_keypad.h"
#include "bn_random.h"
#include "bn_sprite_items_bat_sprite.h"
#include "bn_sprite_items_mario_sprite.h"
#include "bn_sprite_items_mutant.h"
#include "bn_sprite_items_mario_sprite_2.h"
#include "bn_sprite_items_child.h"
#include "bn_sprite_items_wall.h"
#include "bn_sprite_items_rat.h"
#include "bn_sprite_items_box.h"
#include "bn_affine_bg_map_ptr.h"

#include "bn_sound_items.h"

namespace fe
{
enum directions {up, down, left, right};

[[nodiscard]] int _get_map_cell(bn::fixed x, bn::fixed y, bn::affine_bg_ptr& map, bn::span<const bn::affine_bg_map_cell> cells)
{
    int map_size = map.dimensions().width();
    int cell =  modulo((y.safe_division(8).right_shift_integer() * map_size/8 + x/8), map_size*8).integer();
    return cells.at(cell);
}

[[nodiscard]] bool _contains_cell(int tile, bn::vector<int, 32> tiles)
{
    for(int index = 0; index < tiles.size(); ++index)
    {
        if(tiles.at(index) == tile)
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool _check_collisions_map(bn::fixed_point pos, Hitbox hitbox, directions direction, bn::affine_bg_ptr& map, fe::Level level, bn::span<const bn::affine_bg_map_cell> cells)
{

    bn::fixed l = pos.x() - hitbox.width() / 2 + hitbox.x();
    bn::fixed r = pos.x() + hitbox.width() / 2 + hitbox.x();
    bn::fixed u = pos.y() - hitbox.height() / 2 + hitbox.y();
    bn::fixed d = pos.y() + hitbox.height() / 2 + hitbox.y();

    bn::vector<int, 32> tiles;
    if(direction == down) {
        tiles = level.floor_tiles();
    } else if(direction == left || direction == right) {
        tiles = level.wall_tiles();
    } else if(direction == up) {
        tiles = level.ceil_tiles();
    }

    if(_contains_cell(_get_map_cell(l, u, map, cells), tiles) ||
            _contains_cell(_get_map_cell(l, d, map, cells), tiles) ||
            _contains_cell(_get_map_cell(r, u, map, cells), tiles) ||
            _contains_cell(_get_map_cell(l, d, map, cells), tiles)) {
        return true;
    } else {
        return false;
    }
}

constexpr const bn::fixed gravity = 0.2;
constexpr const bn::fixed wall_run_speed =0.25;
constexpr const bn::fixed jump_power = 4;
constexpr const bn::fixed acc = 0.1;
constexpr const bn::fixed max_dy = 6;
constexpr const bn::fixed friction = 0.85;

int boss_tele_timer;
int jump_timer = 0;
int time_to_jump = 180;
bool is_tired = false;
int time_to_sleep = 600;
int sleep_timer = 0;

bn::random random = bn::random();

Enemy::Enemy(int x, int y, bn::camera_ptr camera, bn::affine_bg_ptr map, ENEMY_TYPE type, int hp) :
    _pos(x, y), _camera(camera), _type(type), _hp(hp), _map(map), _level(Level(map))
{
    _map_cells = map.map().cells_ref().value();
    _dir = 1;

    boss_tele_timer = 0;

    if(_type == ENEMY_TYPE::BAT)
    {
        _sprite = bn::sprite_items::bat_sprite.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 4, bn::sprite_items::bat_sprite.tiles_item(), 0,1,0,1);
    } else if (_type == ENEMY_TYPE::MARIO) {
        _sprite = bn::sprite_items::mario_sprite.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 10, bn::sprite_items::mario_sprite.tiles_item(), 0,1,0,1);
    } else if (_type == ENEMY_TYPE::SLIMEO) {
        _sprite = bn::sprite_items::mario_sprite_2.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 20, bn::sprite_items::mario_sprite_2.tiles_item(), 0,1,0,1);
    }
    else if (_type == ENEMY_TYPE::WALL) {
        _sprite = bn::sprite_items::wall.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 20, bn::sprite_items::wall.tiles_item(), 0,1,0,1);
    }
    else if (_type == ENEMY_TYPE::BOSS) {
        _sprite = bn::sprite_items::child.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 20, bn::sprite_items::child.tiles_item(), 1,2,3,4);
    }
    else if (_type == ENEMY_TYPE::RAT) {
        _sprite = bn::sprite_items::rat.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(1);
        _action = bn::create_sprite_animate_action_forever(
                      _sprite.value(), 20, bn::sprite_items::rat.tiles_item(), 0,1,2,3);
    }
    else if (_type == ENEMY_TYPE::MUTANT) {
        _sprite = bn::sprite_items::mutant.create_sprite(_pos.x(), _pos.y());
        _sprite.value().set_camera(_camera);
        _sprite.value().set_bg_priority(0);
        _sprite.value().put_above();
        _mutant_action = bn::create_sprite_animate_action_forever(
                             _sprite.value(), 2, bn::sprite_items::mutant.tiles_item(), 3,4,5,6,7,8,9,10,11,12);
    }
    _sprite.value().set_visible(true);

}

void Enemy::set_visible(bool visiblity) {
    _sprite.value().set_visible(visiblity);
}

bool Enemy::is_vulnerable() {
    if(_type == ENEMY_TYPE::MUTANT) {
        return is_tired;
    } else {
        return !_invulnerable;
    }
}

bool Enemy::damage_from_left(int damage) {
    if(_type != ENEMY_TYPE::MUTANT) {
        if(_type == ENEMY_TYPE::BOSS && !_invulnerable) {
            teleport();
        } else if(_type == ENEMY_TYPE::SLIMEO) {
            _dy-=0.4;
            // _dx-=1;
        } else {
            _dy-=0.4;
            _dx-=1;
        }
        _dir = 1;
        _direction_timer = 0;
        _grounded = false;
        _sprite.value().set_horizontal_flip(true);
    }
    return _take_damage(damage);
}

bool Enemy::damage_from_right(int damage) {
    if(_type != ENEMY_TYPE::MUTANT) {
        if(_type == ENEMY_TYPE::BOSS && !_invulnerable) {
            teleport();
        } else if(_type == ENEMY_TYPE::SLIMEO) {
            _dy-=0.4;
        } else {
            _dy-=0.4;
            _dx+=1;
        }
        _dir = -1;
        _direction_timer = 0;
        _grounded = false;
        _sprite.value().set_horizontal_flip(false);
    }
    return _take_damage(damage);
}

void Enemy::teleport() {
    bool pos_found = false;
    bool right = !_sprite.value().horizontal_flip();

    int x = 12;

    while(!pos_found) {
        if(right) {
            pos_found = _fall_check(_pos.x() + x*8, _pos.y());
        } else {
            pos_found = _fall_check(_pos.x() - x*8, _pos.y());
        }
        if(!pos_found) {
            --x;
            if(x < -12) {
                pos_found = true;
            }
        }
    }
    boss_tele_timer = 0;
    bn::sound_items::teleport.play();
    if(right) {
        _pos.set_x(_pos.x() +x*8);
    } else {
        _pos.set_x(_pos.x() -x*8);
    }
    _dx = 0;
    _dir = -_dir;
    _direction_timer = 0;
    _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
}

bool Enemy::_take_damage(int damage) {
    if(!_invulnerable || (_type == ENEMY_TYPE::MUTANT && !is_tired))
    {
        _hp -= damage;
        _invulnerable = true;
        if(_type == ENEMY_TYPE::MUTANT) {
            _mutant_action = bn::create_sprite_animate_action_once(
                                 _sprite.value(), 4, bn::sprite_items::mutant.tiles_item(), 17,17,17,17,17,17,17,17,17,17);
            bn::sound_items::mutant_hit.play(1);
        }
        if(_hp <= 0) {
            bn::sound_items::death.play();
            if(_type == ENEMY_TYPE::MARIO) {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 5, bn::sprite_items::mario_sprite.tiles_item(), 2,3,3,3);
            }
            else if (_type == ENEMY_TYPE::SLIMEO)
            {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 5, bn::sprite_items::mario_sprite_2.tiles_item(), 2,3,3,3);
            }
            else if (_type == ENEMY_TYPE::BAT)
            {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 5, bn::sprite_items::bat_sprite.tiles_item(), 2,3,3,3);
            }
            else if (_type == ENEMY_TYPE::BOSS)
            {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 5, bn::sprite_items::child.tiles_item(), 5,6,7,8);
            }
            else if (_type == ENEMY_TYPE::WALL)
            {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 5, bn::sprite_items::wall.tiles_item(), 2,2,3,3);
            }
            else if (_type == ENEMY_TYPE::RAT)
            {
                _action = bn::create_sprite_animate_action_once(
                              _sprite.value(), 10, bn::sprite_items::rat.tiles_item(), 4,5,6,7);
            }

            return true;
        }
    }
    return false;
}

bool Enemy::is_hit(Hitbox attack)
{
    if(!_dead) {
        if(_type == ENEMY_TYPE::BOSS) {
            return check_collisions_bb(attack, _pos.x(), _pos.y(), 8, 16);
        } else if(_type == ENEMY_TYPE::MUTANT) {
            return check_collisions_bb(attack, _pos.x(), _pos.y()+16, 50, 32);
        } else {
            return check_collisions_bb(attack, _pos.x(), _pos.y(), 8, 8);
        }

    } else {
        return false;
    }
}

bool Enemy::_will_hit_wall()
{

    if(_check_collisions_map(_pos, Hitbox(-4, 0, 12, 12), directions::left, _map, _level, _map_cells)) {
        return true;
    }
    if(_check_collisions_map(_pos, Hitbox(10, 0, 12, 12), directions::right, _map, _level, _map_cells)) {
        return true;
    }
    return false;
}

//boss for teleporting
bool Enemy::_fall_check(bn::fixed x, bn::fixed y)
{
    if(_check_collisions_map(bn::fixed_point(x, y), Hitbox(0,16,4,9), directions::down, _map, _level, _map_cells)) {
        return true;
    } else {
        return false;
    }
}

bool Enemy::_will_fall() {
    if(_type == ENEMY_TYPE::BOSS) {
        if(_dx < 0) { // left
            if(!_check_collisions_map(_pos, Hitbox(-4,16,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        } else { //right
            if(!_check_collisions_map(_pos, Hitbox(4,16,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        }
    } else if(_type == ENEMY_TYPE::MUTANT) {
        if(_dx < 0) { // left
            if(!_check_collisions_map(_pos, Hitbox(-16,32,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        } else { //right
            if(!_check_collisions_map(_pos, Hitbox(16,32,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        }
    } else {
        if(_dx < 0) { // left
            if(!_check_collisions_map(_pos, Hitbox(-4,8,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        } else { //right
            if(!_check_collisions_map(_pos, Hitbox(4,8,4,8), directions::down, _map, _level, _map_cells)) {
                return true;
            }
        }
    }

    return false;
}

bn::fixed_point Enemy::pos() {
    return _pos;
}

int Enemy::hp() {
    return _hp;
}

void Enemy::set_pos(bn::fixed_point pos) {
    if(_type == ENEMY_TYPE::MUTANT) {
        bn::sound_items::growl.play();
    }
    _pos = pos;
}

bool Enemy::spotted_player() {
    return _spotted_player;
}

ENEMY_TYPE Enemy::type() {
    return _type;
}

void Enemy::update( bn::fixed_point player_pos) {
    if(!_dead)
    {
        if(!_sprite.value().visible())
        {
            _sprite.value().set_visible(true);
        }

        // dead check
        if(_action.has_value() && _action.value().done()) {
            _sprite.value().set_visible(false);
            _dead = true;
        }

        if(_invulnerable) {
            ++_inv_timer;
            if(_inv_timer > 20) {
                _inv_timer = 0;
                _invulnerable = false;
            }
        }

        //apply gravity
        if(_type != ENEMY_TYPE::BAT)
        {
            _dy += gravity;
        }

        // Labrat spot player
        if(_type == ENEMY_TYPE::RAT && !_spotted_player) {
            //left
            if(_sprite.value().horizontal_flip()) {
                if(check_collisions_bb(Hitbox(_pos.x() -40, _pos.y(), 80, 8), player_pos.x(), player_pos.y(), 16,8)) {
                    _spotted_player = true;
                    bn::sound_items::eek.play(1);
                }
            }
            else //right
            {
                if(check_collisions_bb(Hitbox(_pos.x() +40, _pos.y(), 80, 8), player_pos.x(), player_pos.y(), 16,8)) {
                    _spotted_player = true;
                    bn::sound_items::eek.play(1);
                }
            }

        }

        if(_type == ENEMY_TYPE::MARIO|| _type == ENEMY_TYPE::SLIMEO || _type == ENEMY_TYPE::RAT || _type == ENEMY_TYPE::BOSS || _type == ENEMY_TYPE::MUTANT) {
            if(!_invulnerable && _grounded && _direction_timer > 30 && _type != ENEMY_TYPE::MUTANT) {
                if(_will_fall() || _will_hit_wall()) {
                    _dx = 0;
                    _dir = -_dir;
                    _direction_timer = 0;
                    _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
                }
            } else if (!_invulnerable && _direction_timer > 80 && _type == ENEMY_TYPE::MUTANT) {
                if(_will_hit_wall()) {
                    _dx = 0;
                    _dir = -_dir;
                    _direction_timer = 0;
                    _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
                }
            }
            if(_type == ENEMY_TYPE::MARIO|| _type == ENEMY_TYPE::SLIMEO) {
                if((_action.value().current_index() == 1 || _action.value().current_index() == 3)  && !_invulnerable && _grounded) {
                    if(_type == ENEMY_TYPE::MARIO) {
                        _dx += _dir*acc;
                    } else {
                        _dx += _dir*acc*1.5;
                    }
                    // 10 (or 20) is a magic number
                    // maybe bind to animate_forever value
                    if(_sound_timer > 10) {
                        bn::sound_items::steps.play(0.3);
                        _sound_timer = 0;
                    } else {
                        ++_sound_timer;
                    }
                }
            } else {
                if(player_pos.y() - 8 == _pos.y()) {
                    //wut is this... come on Jono
                    if(player_pos.x() < _pos.x() && bn::abs(player_pos.x() - _pos.x()) > 8 && !_sprite.value().horizontal_flip()) {
                        _dx = 0;
                        _dir = -_dir;
                        _direction_timer = 0;
                        _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
                    } else if(player_pos.x() > _pos.x() && bn::abs(player_pos.x() - _pos.x()) < 8 && _sprite.value().horizontal_flip()) {
                        _dx = 0;
                        _dir = -_dir;
                        _direction_timer = 0;
                        _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
                    }
                    //boss teleport
                    if(bn::abs(player_pos.x() - _pos.x()) < 100 && bn::abs(player_pos.x() - _pos.x()) > 16 && boss_tele_timer > 60) {
                        // bn::fixed diff = player_pos.x() - _pos.x();
                        if(_fall_check(player_pos.x(), _pos.y()) && _hp>0 && !_target_locked) {
                            _target.set_x(player_pos.x());
                            _target.set_y(player_pos.y() - 8);
                            _target_locked = true;
                            _invulnerable = true;
                            boss_tele_timer = 0;
                            _action = bn::create_sprite_animate_action_forever(
                                          _sprite.value(), 20, bn::sprite_items::child.tiles_item(), 9,9,9,9);
                        }
                    } else {
                        ++boss_tele_timer;
                        if(_target_locked && boss_tele_timer > 30) {
                            boss_tele_timer = 0;
                            _target_locked = false;
                            _pos.set_x(_target.x());
                            _pos.set_y(_target.y());
                            bn::sound_items::teleport.play();
                            _dx = 0;
                            _invulnerable = false;
                            _dir = -_dir;
                            _direction_timer = 0;
                            _action = bn::create_sprite_animate_action_forever(
                                          _sprite.value(), 5, bn::sprite_items::child.tiles_item(), 1,2,3,4);
                        }

                    }
                }

                if(_type ==ENEMY_TYPE::MUTANT) {
                    ++jump_timer;
                    if(jump_timer > time_to_jump) {
                        if(!is_tired) {
                            time_to_jump = random.get() % 32 * 8 + 100;
                            _dy = -6;
                            _grounded = false;
                        }
                        jump_timer = 0;

                    }
                    ++sleep_timer;
                    if(is_tired && sleep_timer > 180) {
                        is_tired = false;
                        sleep_timer = 0;
                    } else if(!is_tired && sleep_timer > time_to_sleep) {
                        is_tired = true;
                        sleep_timer = 0;
                    }
                }

                if(!_invulnerable && _grounded && _type != ENEMY_TYPE::MUTANT) {
                    _dx += _dir*acc;
                }

                if(_type == ENEMY_TYPE::MUTANT && !is_tired) {
                    _dx += _dir*acc*5;
                }
            }

        }
        else if(_type == ENEMY_TYPE::BAT) {
            if(_direction_timer > 60) {
                if(_will_hit_wall()) {
                    _dx = 0;
                    _dir = -_dir;
                    _direction_timer = 0;
                    _sprite.value().set_horizontal_flip(!_sprite.value().horizontal_flip());
                }
            }
            if(!_invulnerable) {
                _dx += _dir*acc;
            }
        }

        _dx = _dx * friction;

        if(_dx > 0) {
            _sprite.value().set_horizontal_flip(false);
        } else if(_dx < 0) {
            _sprite.value().set_horizontal_flip(true);
        }

        //fall
        if(_dy > 0) {
            if(_type == ENEMY_TYPE::BOSS) {
                if(_check_collisions_map(_pos, Hitbox(0,16,8,0), directions::down, _map, _level, _map_cells)) {
                    _dy = 0;
                    // BN_LOG(bn::to_string<32>(_pos.x())+" " + bn::to_string<32>(_pos.y()));
                    _pos.set_y(_pos.y() - modulo(_pos.y(),8));
                    _grounded = true;
                } else {
                    _grounded = false;
                }
            } else if(_type == ENEMY_TYPE::MUTANT) {
                if(_check_collisions_map(_pos, Hitbox(0,32,32,0), directions::down, _map, _level, _map_cells)) {
                    _dy = 0;
                    // BN_LOG(bn::to_string<32>(_pos.x())+" " + bn::to_string<32>(_pos.y()));
                    _pos.set_y(_pos.y() - modulo(_pos.y(),8));
                    _grounded = true;
                } else {
                    _grounded = false;
                }
            } else {
                if(_check_collisions_map(_pos, Hitbox(0,8,8,0), directions::down, _map, _level, _map_cells)) {
                    _dy = 0;
                    // BN_LOG(bn::to_string<32>(_pos.x())+" " + bn::to_string<32>(_pos.y()));
                    _pos.set_y(_pos.y() - modulo(_pos.y(),8));
                    _grounded = true;
                } else {
                    _grounded = false;
                }
            }

        }

        //bounce?
        if(bn::abs(_dx) > 0) {
            if(_check_collisions_map(_pos, Hitbox(0, 0, 4, 8), directions::left, _map, _level, _map_cells)) {
                _dx = -_dx;
                // _direction_timer = 0;
            }
        }

        //ANIMATION
        if(_type == ENEMY_TYPE::MUTANT) {
            if(is_tired) {
                if(_mutant_action.value().graphics_indexes().front() == 17 && _mutant_action.value().done()) {
                    is_tired = false;
                    bn::sound_items::growl.play(1);

                } else if(_mutant_action.value().graphics_indexes().front() != 15 && _mutant_action.value().graphics_indexes().front() != 17) {
                    _mutant_action = bn::create_sprite_animate_action_forever(
                                         _sprite.value(), 10, bn::sprite_items::mutant.tiles_item(), 15,16,15,16,15,16,15,16,15,16);
                }
            } else {
                if(!_grounded) {
                    if(_dy <0) {
                        _mutant_action = bn::create_sprite_animate_action_forever(
                                             _sprite.value(), 2, bn::sprite_items::mutant.tiles_item(), 13,13,13,13,13,13,13,13,13,13);
                    } else {
                        _mutant_action = bn::create_sprite_animate_action_forever(
                                             _sprite.value(), 2, bn::sprite_items::mutant.tiles_item(), 14,14,14,14,14,14,14,14,14,14);
                    }
                } else if(_mutant_action.value().graphics_indexes().front() != 3) {
                    _mutant_action = bn::create_sprite_animate_action_forever(
                                         _sprite.value(), 2, bn::sprite_items::mutant.tiles_item(), 3,4,5,6,7,8,9,10,11,12);
                } else if(_mutant_action.value().current_index() == 3) {
                    bn::sound_items::steps.play(1);
                }
            }
        }

        //max
        if(_dy > max_dy) {
            _dy = max_dy;
        }

        if(_hp >= 0) {
            if(_type == ENEMY_TYPE::BOSS && _invulnerable) {

            } else {
                _pos.set_x(_pos.x() + _dx);
                _pos.set_y(_pos.y() + _dy);
                _sprite.value().set_position(_pos);
                _sprite.value().set_y(_sprite.value().y() - 1);
            }
        }

        if(_action.has_value() && !_action.value().done()) {
            _action.value().update();
        }

        if(_mutant_action.has_value() && !_mutant_action.value().done()) {
            _mutant_action.value().update();
        }

        if(_direction_timer < 121) {
            _direction_timer+=1;
        }
    }
}
}
