#ifndef FE_SCENE_PATH_H
#define FE_SCENE_PATH_H

#include "fe_scene.h"
#include "fe_player.h"
#include "bn_fixed_point.h"

namespace fe {
class Path {
private:
    Player* _player;
public:
    Path(Player& player);
    Scene execute(bn::fixed_point spawn);
};
}

#endif
