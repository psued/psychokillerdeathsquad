#include "character.h"
#include "move_def.h"
#include "character_animation.h"
#include "texture.h"

/**
 * ANIMATIONS CHARACTER FREDRIK
 */
const AnimationFrame fredrik_idle[7] = {
	{ 0,0,64,112,  9,4,22,97,  0,0,0,0,  24,102,  6,  true },
	{ 64,0,64,112,  10,4,22,97,  0,0,0,0,  24,102,  6,  true },
	{ 128,0,64,112,  11,3,22,97,  0,0,0,0,  24,102,  8,  true },
	{ 192,0,64,112,  13,4,22,97,  0,0,0,0,  24,102,  6,  true },
	{ 256,0,64,112,  14,5,22,97,  0,0,0,0,  24,102,  8,  true },
	{ 320,0,64,112,  15,6,22,97,  0,0,0,0,  24,102,  8,  true },
	{ 384,0,64,112,  15,6,22,97,  0,0,0,0,  24,102,  8,  true },
};
const AnimationFrame fredrik_block[1] = {
	{ 0,0,48,112,  7,8,28,97,  0,0,0,0,  16,100,  6,  false },
};
const AnimationFrame fredrik_block_stun[1] = {
	{ 0,0,52,112,  7,6,29,102,  0,0,0,0,  20,100,  6,  false },
};
const AnimationFrame fredrik_dizzy[4] = {
	{ 0,0,48,112,  5,5,24,105,  0,0,0,0,  18,100,  4,  false },
	{ 48,0,48,112,  10,6,20,104,  0,0,0,0,  18,100,  4,  false },
	{ 96,0,48,112,  11,10,20,100,  0,0,0,0,  18,100,  8,  false },
	{ 48,0,48,112,  10,6,20,104,  0,0,0,0,  18,100,  4,  false },
};
const AnimationFrame fredrik_grab[3] = {
	{ 0,0,84,128,  10,29,25,95,  0,0,0,0,  22,120,  8,  false },
	{ 84,0,84,128,  10,32,26,91,  60,20,24,29,  22,120,  4,  false },
	{ 168,0,84,128,  10,29,25,95,  0,0,0,0,  22,120,  18,  false },
};
const AnimationFrame fredrik_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,96,112,  29,2,26,102,  0,0,0,0,  40,104,  6,  false },
	[THROWN_POSE_LIFTED] = { 96,0,96,112,  22,34,34,62,  0,0,0,0,  40,104,  6,  false },
	[THROWN_POSE_AIR] = { 192,0,96,112,  10,71,81,29,  0,0,0,0,  40,104,  6,  false },
	[THROWN_POSE_IMPACT] = { 288,0,96,112,  8,81,84,24,  0,0,0,0,  40,104,  6,  false },
};
const AnimationFrame fredrik_hit[5] = {
	{ 0,0,64,112,  16,15,30,89,  0,0,0,0,  30,104,  4,  false },
	{ 64,0,64,112,  16,15,30,89,  0,0,0,0,  30,104,  4,  false },
	{ 128,0,64,112,  16,15,30,89,  0,0,0,0,  28,104,  8,  false },
	{ 64,0,64,112,  16,15,30,89,  0,0,0,0,  30,104,  4,  false },
	{ 0,0,64,112,  16,15,30,89,  0,0,0,0,  30,104,  4,  false },
};
const AnimationFrame fredrik_hpunch[4] = {
	{ 0,0,80,128,  9,38,23,81,  0,0,0,0,  22,120,  5,  false },
	{ 80,0,80,128,  12,24,24,98,  0,0,0,0,  22,120,  6,  false },
	{ 160,0,80,128,  13,28,28,93,  36,39,44,19,  22,120,  5,  false },
	{ 240,0,80,128,  12,34,25,87,  0,0,0,0,  22,120,  20,  false },
};
const AnimationFrame fredrik_jump[4] = {
	{ 0,0,80,144,  14,38,25,100,  0,0,0,0,  30,136,  4,  false },
	{ 80,0,80,144,  19,39,19,102,  0,0,0,0,  30,136,  4,  false },
	{ 160,0,80,144,  17,33,23,107,  0,0,0,0,  30,136,  8,  false },
	{ 240,0,80,144,  19,32,29,107,  0,0,0,0,  30,136,  8,  false },
};
const AnimationFrame fredrik_knockdown[1] = {
	{ 0,0,112,48,  2,3,107,24,  0,0,0,0,  58,20,  6,  false },
};
const AnimationFrame fredrik_lpunch[4] = {
	{ 0,0,80,124,  9,9,23,111,  0,0,0,0,  24,116,  2,  false },
	{ 80,0,80,124,  10,10,23,111,  0,0,0,0,  24,116, 3,  false },
	{ 160,0,80,124,  11,11,26,111,  31,23,37,26,  24,116,  4,  false },
	{ 240,0,80,124,  12,10,26,111,  0,0,0,0,  24,116,  8,  false },
};
const ThrowConnectFrame fredrik_throw[8] = {
	{{ 0,0,100,128,  23,11,36,92,  0,0,0,0,  32,100,  1,  false }, {60,0, THROWN_POSE_GRABBED}},
	{{ 0,0,100,128,  23,11,36,92,  0,0,0,0,  32,100,  1,  false }, {50,-30, THROWN_POSE_GRABBED}},
	{{ 100,0,100,128,  19,14,32,88,  0,0,0,0,  32,100,  1,  false }, {41,-60, THROWN_POSE_LIFTED}},
	{{ 100,0,100,128,  19,14,32,88,  0,0,0,0,  32,100,  1,  false }, {30,-60, THROWN_POSE_LIFTED}},
	{{ 200,0,100,128,  18,14,28,91,  0,0,0,0,  32,100,  1,  false }, {15,-30, THROWN_POSE_AIR}},
	{{ 200,0,100,128,  18,14,28,91,  0,0,0,0,  32,100,  1,  false }, {0,-15, THROWN_POSE_AIR}},
	{{ 300,0,100,128,  12,23,30,80,  0,0,0,0,  32,100,  1,  false }, {-80,0, THROWN_POSE_IMPACT}},
	{{ 300,0,100,128,  12,23,30,80,  0,0,0,0,  32,100,  12,  false }, {-100,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame fredrik_wakeup[1] = {
	{ 0,0,64,64,  6,12,49,43,  0,0,0,0,  24,50,  6,  false },
};
const AnimationFrame fredrik_walk[5] = {
	{ 0,0,48,112,  10,7,24,97,  0,0,0,0,  22,104,  4,  true },
	{ 48,0,48,112,  12,8,24,97,  0,0,0,0,  22,104,  4,  true },
	{ 96,0,48,112,  12,7,24,97,  0,0,0,0,  22,104,  4,  true },
	{ 144,0,48,112,  12,7,24,97,  0,0,0,0,  22,104,  4,  true },
	{ 192,0,48,112,  15,9,24,97,  0,0,0,0,  22,104,  4,  true },
};

/**
 * ANIMATIONS CHARACTER JENS
 */
const AnimationFrame jens_idle[6] = {
	{ 0,0,80,112,  17,14,33,91,  0,0,0,0,  32,102,  4,  true },
	{ 80,0,80,112,  18,13,33,91,  0,0,0,0,  32,102,  4,  true },
	{ 160,0,80,112,  19,14,33,91,  0,0,0,0,  32,102,  4,  true },
	{ 240,0,80,112,  20,13,33,91,  0,0,0,0,  32,102,  4,  true },
	{ 320,0,80,112,  19,14,33,91,  0,0,0,0,  32,102,  4,  true },
	{ 400,0,80,112,  18,14,33,91,  0,0,0,0,  32,102,  4,  true },
};
const AnimationFrame jens_block[1] = {
	{ 0,0,64,96,  17,18,26,73,  0,0,0,0,  26,88,  6,  false },
};
const AnimationFrame jens_block_stun[1] = {
	{ 0,0,48,96,  6,13,28,76,  0,0,0,0,  26,88,  6,  false },
};
const AnimationFrame jens_dizzy[4] = {
	{ 0,0,80,112,  27,12,25,92,  0,0,0,0,  40,102,  4,  false },
	{ 80,0,80,112,  14,9,31,96,  0,0,0,0,  40,102,  4,  false },
	{ 160,0,80,112,  28,15,26,89,  0,0,0,0,  40,102,  4,  false },
	{ 240,0,80,112,  30,22,34,78,  0,0,0,0,  40,102,  8,  false },
};
const AnimationFrame jens_grab[4] = {
	{ 0,0,100,112,  16,15,38,90,  0,0,0,0,  32,102,  4,  false },
	{ 100,0,100,112,  25,20,40,82,  0,0,0,0,  32,102,  6,  false },
	{ 200,0,100,112,  28,25,40,77,  68,29,31,37,  30,102,  6,  false },
	{ 300,0,100,112,  30,32,38,73,  0,0,0,0,  30,102,  15,  false },
};
const AnimationFrame jens_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,100,112,  31,17,32,91,  0,0,0,0,  44,106,  6,  false },
	[THROWN_POSE_LIFTED] = { 100,0,100,112,  32,43,41,64,  0,0,0,0,  44,106,  6,  false },
	[THROWN_POSE_AIR] = { 200,0,100,112,  11,65,80,40,  0,0,0,0,  44,106,  6,  false },
	[THROWN_POSE_IMPACT] = { 300,0,100,112,  8,73,80,38,  0,0,0,0,  44,96,  6,  false },
};
const AnimationFrame jens_hit[5] = {
	{ 0,0,80,112,  28,8,26,96,  0,0,0,0,  38,102,  4,  false },
	{ 80,0,80,112,  28,8,24,96,  0,0,0,0,  38,102,  4,  false },
	{ 160,0,80,112,  26,20,26,89,  0,0,0,0,  38,102,  8,  false },
	{ 80,0,80,112,  28,8,24,96,  0,0,0,0,  38,102,  4,  false },
	{ 0,0,80,112,  23,8,31,96,  0,0,0,0,  38,102,  4,  false },
};
const AnimationFrame jens_hpunch[4] = {
	{ 0,0,80,128,  16,8,21,94,  0,0,0,0,  22,102,  8,  false },
	{ 80,0,80,128,  12,16,35,85,  0,0,0,0,  22,102,  5,  false },
	{ 160,0,80,128,  11,8,37,92,  36,12,44,43,  22,102,  4,  false },
	{ 240,0,80,128,  14,8,30,92,  0,0,0,0,  22,102,  18,  false },
};
const AnimationFrame jens_jump[4] = {
	{ 0,0,80,112,  7,23,31,81,  0,0,0,0,  18,104,  2,  false },
	{ 80,0,80,112,  1,39,38,67,  0,0,0,0,  18,104,  2,  false },
	{ 160,0,80,112,  12,23,32,81,  0,0,0,0,  18,104,  2,  false },
	{ 240,0,80,112,  13,13,35,89,  0,0,0,0,  20,100,  8,  false },
};
const AnimationFrame jens_knockdown[1] = {
	{ 0,0,96,48,  11,9,85,36,  0,0,0,0,  48,30,  6,  false },
};
const AnimationFrame jens_lpunch[4] = {
	{ 0,0,80,112,  10,12,33,91,  0,0,0,0,  24,102,  2,  false },
	{ 80,0,80,112,  11,13,33,91,  0,0,0,0,  24,102,  2,  false },
	{ 160,0,80,112,  12,14,33,91,  39,20,34,27,  24,102,  4,  false },
	{ 240,0,80,112,  14,14,33,91,  0,0,0,0,  24,102,  6,  false },
};
const ThrowConnectFrame jens_throw[8] = {
	{{ 0,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  2,  false }, {50,0, THROWN_POSE_GRABBED}},
	{{ 0,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {40,0, THROWN_POSE_GRABBED}},
	{{ 80,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {28,-40, THROWN_POSE_LIFTED}},
	{{ 80,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 160,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {0,-78, THROWN_POSE_AIR}},
	{{ 160,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {-29,-73, THROWN_POSE_AIR}},
	{{ 240,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  1,  false }, {-80,-30, THROWN_POSE_IMPACT}},
	{{ 240,0,80,112,  9,25,45,79,  0,0,0,0,  34,104,  12,  false }, {-90,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame jens_wakeup[1] = {
	{ 0,0,80,64,  13,3,62,53,  0,0,0,0,  40,48,  6,  false },
};
const AnimationFrame jens_walk[6] = {
	{ 0,0,80,96,  24,13,35,72,  0,0,0,0,  34,86,  3,  true },
	{ 80,0,80,96,  25,11,33,77,  0,0,0,0,  34,86,  3,  true },
	{ 160,0,80,96,  19,6,33,82,  0,0,0,0,  34,86,  3,  true },
	{ 240,0,80,96,  25,11,28,78,  0,0,0,0,  34,86,  3,  true },
	{ 320,0,80,96,  24,17,30,72,  0,0,0,0,  34,86,  3,  true },
	{ 400,0,80,96,  23,19,29,70,  0,0,0,0,  34,86,  3,  true },
};

/**
 * ANIMATIONS CHARACTER EIRIK
 */
const AnimationFrame eirik_idle[6] = {
	{ 0,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
	{ 80,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
	{ 160,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
	{ 240,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
	{ 320,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
	{ 400,0,80,144,  34,21,17,112,  0,0,0,0,  42,128,  4,  true },
};
const AnimationFrame eirik_block[1] = {
	{ 0,0,80,144,  26,19,26,114,  0,0,0,0,  36,124,  6,  false },
};
const AnimationFrame eirik_block_stun[1] = {
	{ 0,0,80,144,  23,23,26,114,  0,0,0,0,  42,124,  6,  false },
};
const AnimationFrame eirik_dizzy[4] = {
	{ 0,0,48,128,  17,17,22,107,  0,0,0,0,  28,118,  4,  false },
	{ 48,0,48,128,  17,17,22,107,  0,0,0,0,  28,118,  4,  false },
	{ 96,0,48,128,  22,17,22,107,  0,0,0,0,  28,118,  8,  false },
	{ 48,0,48,128,  17,17,22,107,  0,0,0,0,  28,118,  4,  false },
};
const AnimationFrame eirik_grab[4] = {
	{ 0,0,80,144,  19,20,19,110,  0,0,0,0,  26,126,  4,  false },
	{ 80,0,80,144,  19,20,19,110,  0,0,0,0,  26,126,  6,  false },
	{ 160,0,80,144,  23,20,19,110,  35,15,45,37,  26,126,  6,  false },
	{ 240,0,80,144,  23,20,19,110,  0,0,0,0,  26,126,  15,  false },
};
const AnimationFrame eirik_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,100,112,  42,14,32,93,  0,0,0,0,  50,106,  6,  false },
	[THROWN_POSE_LIFTED] = { 100,0,100,112,  21,18,65,61,  0,0,0,0,  46,83,  6,  false },
	[THROWN_POSE_AIR] = { 200,0,100,112,  12,73,75,37,  0,0,0,0,  44,107,  6,  false },
	[THROWN_POSE_IMPACT] = { 300,0,100,112,  2,79,98,33,  0,0,0,0,  44,102,  6,  false },
};
const AnimationFrame eirik_hit[5] = {
	{ 0,0,64,128,  17,10,22,111,  0,0,0,0,  30,112,  4,  false },
	{ 64,0,64,128,  17,10,22,111,  0,0,0,0,  30,112,  4,  false },
	{ 128,0,64,128,  17,10,22,111,  0,0,0,0,  30,112,  8,  false },
	{ 64,0,64,128,  17,10,22,111,  0,0,0,0,  30,112,  4,  false },
	{ 0,0,64,128,  17,10,22,111,  0,0,0,0,  30,112,  4,  false },
};
const AnimationFrame eirik_hpunch[4] = {
	{ 0,0,64,128,  1,3,20,116,  0,0,0,0,  10,112,  8,  false },
	{ 64,0,64,128,  1,3,20,116,  0,0,0,0,  10,112,  5,  false },
	{ 128,0,64,128,  1,3,20,116,  19,10,38,34,  10,112,  3,  false },
	{ 192,0,64,128,  1,3,20,116,  0,0,0,0,  10,112,  14,  false },
};
const AnimationFrame eirik_jump[4] = {
	{ 0,0,64,112,  11,11,26,100,  0,0,0,0,  22,104,  4,  false },
	{ 64,0,64,112,  11,11,26,100,  0,0,0,0,  22,104,  4,  false },
	{ 128,0,64,112,  11,11,26,100,  0,0,0,0,  22,104,  8,  false },
	{ 192,0,64,112,  11,11,26,100,  0,0,0,0,  22,104,  8,  false },
};
const AnimationFrame eirik_knockdown[1] = {
	{ 0,0,100,48,  3,10,96,28,  0,0,0,0,  48,32,  6,  false },
};
const AnimationFrame eirik_lpunch[4] = {
	{ 0,0,80,128,  16,12,18,115,  0,0,0,0,  26,118,  2,  false },
	{ 80,0,80,128,  16,12,18,115,  0,0,0,0,  26,118,  2,  false },
	{ 160,0,80,128,  16,12,18,115,  28,30,46,22,  26,118,  3,  false },
	{ 240,0,80,128,  16,12,18,115,  0,0,0,0,  26,118,  10,  false },
};
const ThrowConnectFrame eirik_throw[8] = {
	{{ 0,0,100,144,  26,35,30,95,  0,0,0,0,  44,128,  2,  false }, {50,0, THROWN_POSE_GRABBED}},
	{{ 0,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {40,0, THROWN_POSE_GRABBED}},
	{{ 100,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {28,-40, THROWN_POSE_LIFTED}},
	{{ 100,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 200,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {0,-78, THROWN_POSE_AIR}},
	{{ 200,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {-29,-73, THROWN_POSE_AIR}},
	{{ 300,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  1,  false }, {-80,-30, THROWN_POSE_IMPACT}},
	{{ 300,0,100,144,  26,35,30,95,   0,0,0,0,  44,128,  12,  false }, {-100,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame eirik_wakeup[1] = {
	{ 0,0,64,64,  5,6,46,51,  0,0,0,0,  26,52,  6,  false },
};
const AnimationFrame eirik_walk[6] = {
	{ 0,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
	{ 80,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
	{ 160,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
	{ 240,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
	{ 320,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
	{ 400,0,80,144,  25,27,23,103,  0,0,0,0,  36,126,  4,  true },
};

/**
 * ANIMATIONS CHARACTER PERNILLE
 */
const AnimationFrame pernille_idle[5] = {
	{ 0,0,64,128,  12,18,22,104,  0,0,0,0,  22,114,  4,  true },
	{ 64,0,64,128,  12,18,22,104,  0,0,0,0,  22,114,  4,  true },
	{ 128,0,64,128,  12,18,22,104,  0,0,0,0,  22,114,  4,  true },
	{ 192,0,64,128,  12,18,22,104,  0,0,0,0,  22,114,  4,  true },
	{ 256,0,64,128,  12,18,22,104,  0,0,0,0,  22,114,  4,  true },
};
const AnimationFrame pernille_block[1] = {
	{ 0,0,64,128,  14,27,28,90,  0,0,0,0,  28,110,  6,  false },
};
const AnimationFrame pernille_block_stun[1] = {
	{ 0,0,64,128,  20,17,26,98,  0,0,0,0,  28,110,  6,  false },
};
const AnimationFrame pernille_dizzy[4] = {
	{ 0,0,64,128,  23,13,17,101,  0,0,0,0,  28,110,  4,  false },
	{ 64,0,64,128,  23,13,17,101,  0,0,0,0,  28,110,  4,  false },
	{ 128,0,64,128,  23,13,17,101,  0,0,0,0,  28,110,  8,  false },
	{ 192,0,64,128,  23,13,17,101,  0,0,0,0,  28,110,  4,  false },
};
const AnimationFrame pernille_grab[4] = {
	{ 0,0,80,128,  17,15,26,97,  0,0,0,0,  28,110,  4,  false },
	{ 80,0,80,128,  17,15,26,97,  0,0,0,0,  28,110,  6,  false },
	{ 160,0,80,128,  17,15,26,97,  35,16,38,29,  28,110,  6,  false },
	{ 240,0,80,128,  17,15,26,97,  0,0,0,0,  28,110,  15,  false },
};
const AnimationFrame pernille_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,118,112,  31,12,34,67,  0,0,0,0,  44,80,  6,  false },
	[THROWN_POSE_LIFTED] = { 118,0,118,112,  31,12,34,67,  0,0,0,0,  48,80,  6,  false },
	[THROWN_POSE_AIR] = { 236,0,118,112,  23,43,58,44,  0,0,0,0,  46,80,  6,  false },
	[THROWN_POSE_IMPACT] = { 354,0,118,112,  10,66,100,28,  0,0,0,0,  54,84,  6,  false },
};
const AnimationFrame pernille_hit[5] = {
	{ 0,0,80,128,  20,20,25,97,  0,0,0,0,  36,110,  4,  false },
	{ 80,0,80,128,  20,20,25,97,  0,0,0,0,  36,110,  4,  false },
	{ 160,0,80,128,  20,20,25,97,  0,0,0,0,  36,110,  8,  false },
	{ 80,0,80,128,  20,20,25,97,  0,0,0,0,  36,110,  4,  false },
	{ 0,0,80,128,  20,20,25,97,  0,0,0,0,  36,110,  4,  false },
};
const AnimationFrame pernille_hpunch[4] = {
	{ 0,0,80,144,  11,22,29,108,  0,0,0,0,  30,126,  5,  false },
	{ 80,0,80,144,  11,22,29,108,  0,0,0,0,  30,126,  5,  false },
	{ 160,0,80,144,  11,22,29,108,  33,22,47,52,  30,126,  4,  false },
	{ 240,0,80,144,  11,22,29,108,  0,0,0,0,  30,126,  15,  false },
};
const AnimationFrame pernille_jump[4] = {
	{ 0,0,64,128,  11,33,27,80,  0,0,0,0,  24,110,  4,  false },
	{ 64,0,64,128,  11,33,27,80,  0,0,0,0,  24,110,  4,  false },
	{ 128,0,64,128,  11,33,27,80,  0,0,0,0,  24,110,  4,  false },
	{ 192,0,64,128,  11,33,27,80,  0,0,0,0,  24,110,  8,  false },
};
const AnimationFrame pernille_knockdown[1] = {
	{ 0,0,118,32,  6,4,108,21,  0,0,0,0,  58,18,  6,  false },
};
const AnimationFrame pernille_lpunch[4] = {
	{ 0,0,80,128,  20,18,20,104,  0,0,0,0,  34,112,  2,  false },
	{ 80,0,80,128,  20,18,20,104,  0,0,0,0,  34,112,  2,  false },
	{ 160,0,80,128,  20,18,20,104,  37,27,42,35,  34,112,  4,  false },
	{ 240,0,80,128,  20,18,20,104,  0,0,0,0,  34,112,  8,  false },
};
const ThrowConnectFrame pernille_throw[8] = {
	{{ 0,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  2,  false }, {50,0, THROWN_POSE_GRABBED}},
	{{ 0,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  2,  false }, {50,0, THROWN_POSE_GRABBED}},
	{{ 64,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 64,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 128,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  1,  false }, {0,-78, THROWN_POSE_AIR}},
	{{ 128,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  1,  false }, {0,-78, THROWN_POSE_AIR}},
	{{ 192,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  1,  false }, {-80,-40, THROWN_POSE_IMPACT}},
	{{ 192,0,64,128,  19,31,28,92,  0,0,0,0,  32,118,  12,  false }, {-90,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame pernille_wakeup[1] = {
	{ 0,0,64,64,  8,4,47,52,  0,0,0,0,  38,52,  6,  false },
};
const AnimationFrame pernille_walk[5] = {
	{ 0,0,64,128,  19,13,26,105,  0,0,0,0,  32,110,  4,  true },
	{ 64,0,64,128,  19,13,26,105,  0,0,0,0,  32,110,  4,  true },
	{ 128,0,64,128,  19,13,26,105,  0,0,0,0,  32,110,  4,  true },
	{ 192,0,64,128,  19,13,26,105,  0,0,0,0,  32,110,  4,  true },
	{ 256,0,64,128,  19,13,26,105,  0,0,0,0,  32,110,  4,  true },
};

/**
 * ANIMATIONS CHARACTER SONDRE
 */
const AnimationFrame sondre_idle[5] = {
	{ 0,0,48,112,  9,22,26,82,  0,0,0,0,  22,104,  3,  true },
	{ 48,0,48,112,  9,22,26,82,  0,0,0,0,  22,104,  3,  true },
	{ 96,0,48,112,  9,22,26,82,  0,0,0,0,  22,104,  3,  true },
	{ 144,0,48,112,  9,22,26,82,  0,0,0,0,  22,104,  3,  true },
	{ 192,0,48,112,  9,22,26,82,  0,0,0,0,  22,104,  3,  true },
};
const AnimationFrame sondre_block[1] = {
	{ 0,0,64,96,  3,5,36,82,  0,0,0,0,  20,88,  6,  false },
};
const AnimationFrame sondre_block_stun[1] = {
	{ 0,0,64,96,  6,9,42,77,  0,0,0,0,  28,88,  6,  false },
};
const AnimationFrame sondre_dizzy[4] = {
	{ 0,0,64,112,  19,22,24,80,  0,0,0,0,  30,104,  4,  false },
	{ 64,0,64,112,  19,22,24,80,  0,0,0,0,  30,104,  4,  false },
	{ 128,0,64,112,  19,22,24,80,  0,0,0,0,  30,104,  8,  false },
	{ 64,0,64,112,  19,22,24,80,  0,0,0,0,  30,104,  4,  false },
};
const AnimationFrame sondre_grab[4] = {
	{ 0,0,96,96,  4,8,35,76,  0,0,0,0,  26,84,  4,  false },
	{ 96,0,96,96,  4,8,35,76,  0,0,0,0,  26,84,  6,  false },
	{ 192,0,96,96,  4,8,35,76,  40,11,54,37,  26,84,  6,  false },
	{ 288,0,96,96,  4,8,35,76,  0,0,0,0,  26,84,  15,  false },
};
const AnimationFrame sondre_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,96,112,  43,13,28,92,  0,0,0,0,  56,100,  6,  false },
	[THROWN_POSE_LIFTED] = { 96,0,96,112,  20,41,46,58,  0,0,0,0,  40,104,  6,  false },
	[THROWN_POSE_AIR] = { 192,0,96,112,  7,65,63,42,  0,0,0,0,  30,100,  6,  false },
	[THROWN_POSE_IMPACT] = { 288,0,96,112,  5,65,79,37,  0,0,0,0,  46,98,  6,  false },
};
const AnimationFrame sondre_hit[5] = {
	{ 0,0,64,128,  17,28,33,76,  0,0,0,0,  36,106,  4,  false },
	{ 64,0,64,128,  17,28,33,76,  0,0,0,0,  36,106,  4,  false },
	{ 128,0,64,128,  17,28,33,76,  0,0,0,0,  36,106,  8,  false },
	{ 64,0,64,128,  17,28,33,76,  0,0,0,0,  36,106,  4,  false },
	{ 0,0,64,128,  17,28,33,76,  0,0,0,0,  36,106,  4,  false },
};
const AnimationFrame sondre_hpunch[4] = {
	{ 0,0,80,112,  11,30,22,73,  0,0,0,0,  18,106,  6,  false },
	{ 80,0,80,112,  11,30,22,73,  0,0,0,0,  18,106,  6,  false },
	{ 160,0,80,112,  11,30,22,73,  32,35,46,24,  18,106,  5,  false },
	{ 240,0,80,112,  11,30,22,73,  0,0,0,0,  18,106,  14,  false },
};
const AnimationFrame sondre_jump[4] = {
	{ 0,0,80,112,  15,22,36,78,  0,0,0,0,  32,104,  2,  false },
	{ 80,0,80,112,  15,22,36,78,  0,0,0,0,  32,104,  2,  false },
	{ 160,0,80,112,  15,22,36,78,  0,0,0,0,  32,104,  2,  false },
	{ 240,0,80,112,  15,22,36,78,  0,0,0,0,  32,104,  8,  false },
};
const AnimationFrame sondre_knockdown[1] = {
	{ 0,0,96,64,  6,18,80,36,  0,0,0,0,  46,42,  6,  false },
};
const AnimationFrame sondre_lpunch[4] = {
	{ 0,0,80,112,  8,29,27,74,  0,0,0,0,  20,104,  2,  false },
	{ 80,0,80,112,  8,29,27,74,  0,0,0,0,  20,104,  4,  false },
	{ 160,0,80,112,  8,29,27,74,  28,21,42,30,  20,104,  3,  false },
	{ 240,0,80,112,  8,29,27,74,  0,0,0,0,  20,104,  7,  false },
};
const ThrowConnectFrame sondre_throw[8] = {
	{{ 0,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {40,0, THROWN_POSE_GRABBED}},
	{{ 0,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {40,0, THROWN_POSE_GRABBED}},
	{{ 96,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 96,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 192,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {-29,-73, THROWN_POSE_AIR}},
	{{ 192,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {-29,-73, THROWN_POSE_AIR}},
	{{ 288,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  1,  false }, {-70,-30, THROWN_POSE_IMPACT}},
	{{ 288,0,96,112,  33,28,29,77,  0,0,0,0,  42,104,  12,  false }, {-100,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame sondre_wakeup[1] = {
	{ 0,0,64,64,  7,9,44,47,  0,0,0,0,  30,48,  6,  false },
};
const AnimationFrame sondre_walk[5] = {
	{ 0,0,64,112,  21,16,21,90,  0,0,0,0,  30,104,  3,  true },
	{ 64,0,64,112,  21,16,21,90,  0,0,0,0,  30,104,  3,  true },
	{ 128,0,64,112,  21,16,21,90,  0,0,0,0,  30,104,  3,  true },
	{ 192,0,64,112,  21,16,21,90,  0,0,0,0,  30,104,  3,  true },
	{ 256,0,64,112,  21,16,21,90,  0,0,0,0,  30,104,  3,  true },
};

/**
 * ANIMATIONS CHARACTER ARI
 */
const AnimationFrame ari_idle[5] = {
	{ 0,0,80,128,  20,8,31,114,  0,0,0,0,  38,118,  6,  true },
	{ 80,0,80,128,  20,8,31,114,  0,0,0,0,  38,118,   6,  true },
	{ 160,0,80,128,  20,8,31,114,  0,0,0,0,  38,118,   6,  true },
	{ 240,0,80,128,  20,8,31,114,  0,0,0,0,  38,118,   6,  true },
	{ 320,0,80,128,  20,8,31,114,  0,0,0,0,  38,118,   6,  true },
};
const AnimationFrame ari_block[1] = {
	{ 0,0,80,144,  24,22,30,96,  0,0,0,0,  40,118,  6,  false },
};
const AnimationFrame ari_block_stun[1] = {
	{ 0,0,80,144,  10,20,38,93,  0,0,0,0,  36,114,  6,  false },
};
const AnimationFrame ari_dizzy[4] = {
	{ 0,0,80,144,  28,28,22,104,  0,0,0,0,  44,126,  4,  false },
	{ 80,0,80,144,  28,28,22,104,  0,0,0,0,  44,126,  4,  false },
	{ 160,0,80,144,  28,28,22,104,  0,0,0,0,  44,126,  8,  false },
	{ 80,0,80,144,  28,28,22,104,  0,0,0,0,  44,126,  4,  false },
};
const AnimationFrame ari_grab[4] = {
	{ 0,0,86,144,  10,23,30,101,  0,0,0,0,  30,124,  4,  false },
	{ 86,0,86,144,  14,23,30,101,  0,0,0,0,  30,124,  6,  false },
	{ 172,0,86,144,  25,29,51,96,  55,30,28,38,  30,124,  6,  false },
	{ 258,0,86,144,  33,25,33,99,  0,0,0,0,  30,124,  15,  false },
};
const AnimationFrame ari_grabbed[NUM_THROWN_POSES] = {
	[THROWN_POSE_GRABBED] = { 0,0,90,144,  30,22,30,98,  0,0,0,0,  42,116,  6,  false },
	[THROWN_POSE_LIFTED] = { 90,0,90,144,  21,50,43,52,  0,0,0,0,  42,116,  6,  false },
	[THROWN_POSE_AIR] = { 180,0,90,144,  10,75,72,39,  0,0,0,0,  42,116,  6,  false },
	[THROWN_POSE_IMPACT] = { 270,0,90,144,  10,83,72,39,  0,0,0,0,  42,116,  6,  false },
};
const AnimationFrame ari_hit[5] = {
	{ 0,0,80,144,  23,32,38,94,  0,0,0,0,  40,124,  4,  false },
	{ 80,0,80,144,  23,32,38,94,  0,0,0,0,  40,124,  4,  false },
	{ 160,0,80,144,  23,32,38,94,  0,0,0,0,  40,124,  8,  false },
	{ 80,0,80,144,  23,32,38,94,  0,0,0,0,  40,124,  4,  false },
	{ 0,0,80,144,  23,32,38,94,  0,0,0,0,  40,124,  4,  false },
};
const AnimationFrame ari_hpunch[4] = {
	{ 0,0,90,144,  25,29,31,87,  0,0,0,0,  32,116,  12,  false },
	{ 90,0,90,144,  25,29,31,87,  45,26,42,30,  32,116,  5,  false },
	{ 180,0,90,144,  25,29,31,87,  0,0,0,0,  32,116,  10,  false },
	{ 270,0,90,144,  25,29,31,87,  0,0,0,0,  32,116,  8,  false },
};
const AnimationFrame ari_jump[4] = {
	{ 240,0,80,144,  24,36,32,78,  0,0,0,0,  40,126,  4,  false },
	{ 0,0,80,144,  24,36,32,78,  0,0,0,0,  40,126,  4,  false },
	{ 80,0,80,144,  24,16,32,78,  0,0,0,0,  40,116,  8,  false },
	{ 160,0,80,144,  24,16,32,78,  0,0,0,0,  40,126,  8,  false },
};
const AnimationFrame ari_knockdown[1] = {
	{ 0,0,100,48,  5,9,93,33,  0,0,0,0,  56,34,  6,  false },
};
const AnimationFrame ari_lpunch[4] = {
	{ 0,0,80,144,  18,9,28,110,  0,0,0,0,  30,118,  3,  false },
	{ 80,0,80,144,  18,9,28,110,  0,0,0,0,  30,118,  3,  false },
	{ 160,0,80,144,  18,9,28,110,  38,21,41,23,  30,118,  4,  false },
	{ 240,0,80,144,  18,9,28,110,  0,0,0,0,  30,118,  9,  false },
};
const ThrowConnectFrame ari_throw[8] = {
	{{ 0,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {50,0, THROWN_POSE_GRABBED}},
	{{ 0,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {40,0, THROWN_POSE_GRABBED}},
	{{ 100,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {28,-40, THROWN_POSE_LIFTED}},
	{{ 100,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {28,-82, THROWN_POSE_LIFTED}},
	{{ 200,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {0,-78, THROWN_POSE_AIR}},
	{{ 200,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {-29,-73, THROWN_POSE_AIR}},
	{{ 300,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  1,  false }, {-60,-30, THROWN_POSE_IMPACT}},
	{{ 300,0,100,144,  34,34,31,92,  0,0,0,0,  46,122,  12,  false }, {-90,0, THROWN_POSE_IMPACT}},
};
const AnimationFrame ari_wakeup[1] = {
	{ 0,0,80,80,  11,15,48,52,  0,0,0,0,  40,65,  6,  false },
};
const AnimationFrame ari_walk[6] = {
	{ 0,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
	{ 80,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
	{ 160,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
	{ 240,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
	{ 320,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
	{ 400,0,80,144,  22,22,30,101,  0,0,0,0,  38,126,  4,  true },
};

/* SPECIAL MOVE SEQUENCES */
static const InputDirection h_punch_motion[] = { DIR_RIGHT };

/* Stats for every character */
const CharacterDef character_table[NUM_CHARACTERS] = {
	[CHAR_FREDRIK] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = fredrik_idle,
			[ANIM_WALK] = fredrik_walk,
			[ANIM_STUNNED] = fredrik_dizzy,
			[ANIM_HIT_STUNNED] = fredrik_hit,
			[ANIM_BLOCK] = fredrik_block,
			[ANIM_BLOCK_STUNNED] = fredrik_block_stun,
			[ANIM_JUMP] = fredrik_jump,
			[ANIM_THROWN] = fredrik_grabbed,
			[ANIM_KNOCKDOWN] = fredrik_knockdown,
			[ANIM_WAKEUP] = fredrik_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_FREDRIK_IDLE,
			[ANIM_WALK] = TEX_FREDRIK_WALK,
			[ANIM_STUNNED] = TEX_FREDRIK_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_FREDRIK_HIT,
			[ANIM_BLOCK] = TEX_FREDRIK_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_FREDRIK_BLOCK_STUN,
			[ANIM_JUMP] = TEX_FREDRIK_JUMP,
			[ANIM_THROWN] = TEX_FREDRIK_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_FREDRIK_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_FREDRIK_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 7,
			[ANIM_WALK] = 5,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 10,
				.blockstun_duration = 10,
				.hitstun_duration = 12,
				.frames = fredrik_lpunch,
				.frame_count = 4,
				.image_id = TEX_FREDRIK_LPUNCH,
				.pushback = 5,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 18,
				.blockstun_duration = 15,
				.hitstun_duration = 18,
				.frames = fredrik_hpunch,
				.frame_count = 4,
				.image_id = TEX_FREDRIK_HPUNCH,
				.pushback = 8,
				.stun = 30,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = fredrik_grab,
			.startup_frame_count = 3,
			.startup_image_id = TEX_FREDRIK_GRAB,

			.connect_frames = fredrik_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_FREDRIK_THROW,

			.damage = 20,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 6,
		},
	},
	[CHAR_JENS] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = jens_idle,
			[ANIM_WALK] = jens_walk,
			[ANIM_STUNNED] = jens_dizzy,
			[ANIM_HIT_STUNNED] = jens_hit,
			[ANIM_BLOCK] = jens_block,
			[ANIM_BLOCK_STUNNED] = jens_block_stun,
			[ANIM_JUMP] = jens_jump,
			[ANIM_THROWN] = jens_grabbed,
			[ANIM_KNOCKDOWN] = jens_knockdown,
			[ANIM_WAKEUP] = jens_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_JENS_IDLE,
			[ANIM_WALK] = TEX_JENS_WALK,
			[ANIM_STUNNED] = TEX_JENS_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_JENS_HIT,
			[ANIM_BLOCK] = TEX_JENS_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_JENS_BLOCK_STUN,
			[ANIM_JUMP] = TEX_JENS_JUMP,
			[ANIM_THROWN] = TEX_JENS_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_JENS_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_JENS_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 6,
			[ANIM_WALK] = 6,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 10,
				.blockstun_duration = 10,
				.hitstun_duration = 13,
				.frames = jens_lpunch,
				.frame_count = 4,
				.image_id = TEX_JENS_LPUNCH,
				.pushback = 5,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 20,
				.blockstun_duration = 17,
				.hitstun_duration = 20,
				.frames = jens_hpunch,
				.frame_count = 4,
				.image_id = TEX_JENS_HPUNCH,
				.pushback = 6,
				.stun = 30,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = jens_grab,
			.startup_frame_count = 4,
			.startup_image_id = TEX_JENS_GRAB,

			.connect_frames = jens_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_JENS_THROW,

			.damage = 20,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 6,
		},
	},
	[CHAR_EIRIK] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = eirik_idle,
			[ANIM_WALK] = eirik_walk,
			[ANIM_STUNNED] = eirik_dizzy,
			[ANIM_HIT_STUNNED] = eirik_hit,
			[ANIM_BLOCK] = eirik_block,
			[ANIM_BLOCK_STUNNED] = eirik_block_stun,
			[ANIM_JUMP] = eirik_jump,
			[ANIM_THROWN] = eirik_grabbed,
			[ANIM_KNOCKDOWN] = eirik_knockdown,
			[ANIM_WAKEUP] = eirik_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_EIRIK_IDLE,
			[ANIM_WALK] = TEX_EIRIK_WALK,
			[ANIM_STUNNED] = TEX_EIRIK_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_EIRIK_HIT,
			[ANIM_BLOCK] = TEX_EIRIK_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_EIRIK_BLOCK_STUN,
			[ANIM_JUMP] = TEX_EIRIK_JUMP,
			[ANIM_THROWN] = TEX_EIRIK_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_EIRIK_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_EIRIK_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 6,
			[ANIM_WALK] = 6,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 12,
				.blockstun_duration = 11,
				.hitstun_duration = 14,
				.frames = eirik_lpunch,
				.frame_count = 4,
				.image_id = TEX_EIRIK_LPUNCH,
				.pushback = 4,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 20,
				.blockstun_duration = 16,
				.hitstun_duration = 19,
				.frames = eirik_hpunch,
				.frame_count = 4,
				.image_id = TEX_EIRIK_HPUNCH,
				.pushback = 6,
				.stun = 20,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = eirik_grab,
			.startup_frame_count = 4,
			.startup_image_id = TEX_EIRIK_GRAB,

			.connect_frames = eirik_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_EIRIK_THROW,

			.damage = 18,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 6,
		},
	},
	[CHAR_PERNILLE] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = pernille_idle,
			[ANIM_WALK] = pernille_walk,
			[ANIM_STUNNED] = pernille_dizzy,
			[ANIM_HIT_STUNNED] = pernille_hit,
			[ANIM_BLOCK] = pernille_block,
			[ANIM_BLOCK_STUNNED] = pernille_block_stun,
			[ANIM_JUMP] = pernille_jump,
			[ANIM_THROWN] = pernille_grabbed,
			[ANIM_KNOCKDOWN] = pernille_knockdown,
			[ANIM_WAKEUP] = pernille_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_PERNILLE_IDLE,
			[ANIM_WALK] = TEX_PERNILLE_WALK,
			[ANIM_STUNNED] = TEX_PERNILLE_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_PERNILLE_HIT,
			[ANIM_BLOCK] = TEX_PERNILLE_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_PERNILLE_BLOCK_STUN,
			[ANIM_JUMP] = TEX_PERNILLE_JUMP,
			[ANIM_THROWN] = TEX_PERNILLE_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_PERNILLE_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_PERNILLE_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 5,
			[ANIM_WALK] = 5,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 10,
				.blockstun_duration = 12,
				.hitstun_duration = 12,
				.frames = pernille_lpunch,
				.frame_count = 4,
				.image_id = TEX_PERNILLE_LPUNCH,
				.pushback = 5,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 18,
				.blockstun_duration = 17,
				.hitstun_duration = 20,
				.frames = pernille_hpunch,
				.frame_count = 4,
				.image_id = TEX_PERNILLE_HPUNCH,
				.pushback = 7,
				.stun = 25,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = pernille_grab,
			.startup_frame_count = 4,
			.startup_image_id = TEX_PERNILLE_GRAB,

			.connect_frames = pernille_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_PERNILLE_THROW,

			.damage = 22,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 4,
		},
	},
	[CHAR_ARI] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = ari_idle,
			[ANIM_WALK] = ari_walk,
			[ANIM_STUNNED] = ari_dizzy,
			[ANIM_HIT_STUNNED] = ari_hit,
			[ANIM_BLOCK] = ari_block,
			[ANIM_BLOCK_STUNNED] = ari_block_stun,
			[ANIM_JUMP] = ari_jump,
			[ANIM_THROWN] = ari_grabbed,
			[ANIM_KNOCKDOWN] = ari_knockdown,
			[ANIM_WAKEUP] = ari_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_ARI_IDLE,
			[ANIM_WALK] = TEX_ARI_WALK,
			[ANIM_STUNNED] = TEX_ARI_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_ARI_HIT,
			[ANIM_BLOCK] = TEX_ARI_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_ARI_BLOCK_STUN,
			[ANIM_JUMP] = TEX_ARI_JUMP,
			[ANIM_THROWN] = TEX_ARI_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_ARI_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_ARI_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 5,
			[ANIM_WALK] = 6,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 12,
				.blockstun_duration = 10,
				.hitstun_duration = 10,
				.frames = ari_lpunch,
				.frame_count = 4,
				.image_id = TEX_ARI_LPUNCH,
				.pushback = 4,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 24,
				.blockstun_duration = 15,
				.hitstun_duration = 19,
				.frames = ari_hpunch,
				.frame_count = 4,
				.image_id = TEX_ARI_HPUNCH,
				.pushback = 6,
				.stun = 15,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = ari_grab,
			.startup_frame_count = 4,
			.startup_image_id = TEX_ARI_GRAB,

			.connect_frames = ari_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_ARI_THROW,

			.damage = 20,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 6,
		},
	},
	[CHAR_SONDRE] = {
		.max_health = 200,
		.animations = {
			[ANIM_IDLE] = sondre_idle,
			[ANIM_WALK] = sondre_walk,
			[ANIM_STUNNED] = sondre_dizzy,
			[ANIM_HIT_STUNNED] = sondre_hit,
			[ANIM_BLOCK] = sondre_block,
			[ANIM_BLOCK_STUNNED] = sondre_block_stun,
			[ANIM_JUMP] = sondre_jump,
			[ANIM_THROWN] = sondre_grabbed,
			[ANIM_KNOCKDOWN] = sondre_knockdown,
			[ANIM_WAKEUP] = sondre_wakeup,
		},
		.animation_images = {
			[ANIM_IDLE] = TEX_SONDRE_IDLE,
			[ANIM_WALK] = TEX_SONDRE_WALK,
			[ANIM_STUNNED] = TEX_SONDRE_DIZZY,
			[ANIM_HIT_STUNNED] = TEX_SONDRE_HIT,
			[ANIM_BLOCK] = TEX_SONDRE_BLOCK,
			[ANIM_BLOCK_STUNNED] = TEX_SONDRE_BLOCK_STUN,
			[ANIM_JUMP] = TEX_SONDRE_JUMP,
			[ANIM_THROWN] = TEX_SONDRE_GRABBED,
			[ANIM_KNOCKDOWN] = TEX_SONDRE_KNOCKDOWN,
			[ANIM_WAKEUP] = TEX_SONDRE_WAKEUP,
		},
		.anim_frame_count = {
			[ANIM_IDLE] = 5,
			[ANIM_WALK] = 5,
			[ANIM_STUNNED] = 4,
			[ANIM_HIT_STUNNED] = 5,
			[ANIM_BLOCK] = 1,
			[ANIM_BLOCK_STUNNED] = 1,
			[ANIM_JUMP] = 4,
			[ANIM_THROWN] = NUM_THROWN_POSES,
			[ANIM_KNOCKDOWN] = 1,
			[ANIM_WAKEUP] = 1,
		},
		.moves = {
			[MOVE_LIGHT_PUNCH] = {
				.damage = 14,
				.blockstun_duration = 11,
				.hitstun_duration = 14,
				.frames = sondre_lpunch,
				.frame_count = 4,
				.image_id = TEX_SONDRE_LPUNCH,
				.pushback = 7,
				.stun = 10,
				.knockdown_duration = 0,
			},
			[MOVE_HEAVY_PUNCH] = {
				.damage = 26,
				.blockstun_duration = 18,
				.hitstun_duration = 20,
				.frames = sondre_hpunch,
				.frame_count = 4,
				.image_id = TEX_SONDRE_HPUNCH,
				.pushback = 6,
				.stun = 25,
				.knockdown_duration = 0,
			},
		},
		.specials = {
			{   
				.sequence = h_punch_motion, 
				.sequence_length = 1,
				.max_ticks_ago = 15,
				.required_button = BUTTON_1,
				.triggers_move = MOVE_HEAVY_PUNCH,
			},
		},
		.special_count = 1,
		.stun_threshold = 60,
		.stun_decay_interval = 25,
		.dizzy_duration = 240,
		.jump_velocity = -120,
		.jump_horizontal_speed = 3,
		.gravity = 4,
		.walk_speed = 2,
		.pushbox = {
			.width = 44,
			.height = 90,
		},
		.throw = {
			.startup_frames = sondre_grab,
			.startup_frame_count = 4,
			.startup_image_id = TEX_SONDRE_GRAB,

			.connect_frames = sondre_throw,
			.connect_frame_count = 8,
			.connect_image_id = TEX_SONDRE_THROW,

			.damage = 30,
			.stun = 20,
			.damage_frame = 7,
			.knockdown_duration = 2,
		},
	},
};
