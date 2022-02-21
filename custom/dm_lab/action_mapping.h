//
// Created by dewe on 10/9/21.
//

#ifndef GYMENV_ACTION_MAPPING_H
#define GYMENV_ACTION_MAPPING_H

#define DEFAULT_ACTION_SET {{ \
{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 0}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 1},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},               \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 0}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", -1},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},                \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 0}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", -1},                                               \
{"MOVE_BACK_FORWARD", 0},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},               \
\
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 0}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 1},                                               \
{"MOVE_BACK_FORWARD", 0},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},               \
\
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", -20}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 0},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},                                             \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 20}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 0},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},                                                                      \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", -20}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 1},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},                                                                                   \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 20}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 1},                                               \
{"FIRE", 0},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}},                                                                                               \
                              \
{{"LOOK_LEFT_RIGHT_PIXELS_PER_FRAME", 0}, \
{"LOOK_DOWN_UP_PIXELS_PER_FRAME", 0},                                   \
{"STRAFE_LEFT_RIGHT", 0},                                               \
{"MOVE_BACK_FORWARD", 0},                                               \
{"FIRE", 1},                                                            \
{"JUMP", 0},                                                            \
{"CROUCH", 0}}                                                          \
}

#endif //GYMENV_ACTION_MAPPING_H
