#ifndef LAYER_DEF_H
#define LAYER_DEF_H

/**
 * A mapping of each asset type in the game to its render layer
 */

/* Background layer is same for all game states */
#define LAYER_BACKGROUND 0U

/* Start Screen */
#define LAYER_START_SCREEN_FLAMES 1U

/* Shared Group 1 */
#define LAYER_START_SCREEN_TITLE 8U
#define LAYER_START_SCREEN_TEXT_PROMPT 9U

/* Character Selection */
#define LAYER_CHAR_SELECT_SMALL_ICONS 1U
#define LAYER_CHAR_SELECT_HOVER_P1 2U
#define LAYER_CHAR_SELECT_HOVER_P2 3U

/* Shared Group 1 */
#define LAYER_CHAR_SELECT_NAME_P1 8U
#define LAYER_CHAR_SELECT_NAME_P2 9U

/* Shared Group 2 */
#define LAYER_CHAR_SELECT_HOVER_CONFIRM_P1 12U
#define LAYER_CHAR_SELECT_HOVER_CONFIRM_P2 13U

/* Shared Group 3 */
#define LAYER_CHAR_SELECT_LARGE_ICONS_P1 16U
#define LAYER_CHAR_SELECT_LARGE_ICONS_P2 17U

/* Stage Selection */
#define LAYER_STAGE_SELECT_HOVER_ICON 2U

/* Shared Group 1 */
#define LAYER_STAGE_SELECT_FLAGS_1 8U
#define LAYER_STAGE_SELECT_FLAGS_2 9U
#define LAYER_STAGE_SELECT_FLAGS_3 10U

/* Shared Group 2 */
/* Shared Group 3 */
#define LAYER_STAGE_SELECT_FLAGS_4 19U
#define LAYER_STAGE_SELECT_FLAGS_5 20U
#define LAYER_STAGE_SELECT_FLAGS_6 21U


/* Battle */
#define LAYER_BATTLE_FLOOR 1U
#define LAYER_BATTLE_P1 2U
#define LAYER_BATTLE_P2 3U

/* Shared Group 1 */
#define LAYER_BATTLE_KO_FLASH 8U

/* Shared Group 2 */
/* Shared Group 3 UI */
#define LAYER_BATTLE_UI_BACKGROUND 16U
#define LAYER_BATTLE_UI_HEALTH_BAR_P1 17U
#define LAYER_BATTLE_UI_HEALTH_BAR_P2 18U
#define LAYER_BATTLE_UI_STUN_BAR_P1 19U
#define LAYER_BATTLE_UI_STUN_BAR_P2 20U
#define LAYER_BATTLE_UI_DIGIT_1 21U
#define LAYER_BATTLE_UI_DIGIT_2 22U
#define LAYER_BATTLE_UI_BANNER 23U


/* Round Over */
#define LAYER_ROUND_OVER_FLOOR 1U
#define LAYER_ROUND_OVER_P1 2U
#define LAYER_ROUND_OVER_P2 3U

/* Shared Group 1 */
#define LAYER_ROUND_OVER_TEXT_TOP 8U
#define LAYER_ROUND_OVER_TEXT_BOTTOM 9U


/* Match Over */
#define LAYER_MATCH_OVER_FLAME_1 1U
#define LAYER_MATCH_OVER_FLAME_2 2U

/* Shared Group 1 */
#define LAYER_MATCH_OVER_NAME 8U

/* Shared Group 2 */
#define LAYER_MATCH_OVER_QUOTES 12U

/* Shared Group 3 */
#define LAYER_MATCH_OVER_LARGE_ICON 16U

/* End Credits */
#define LAYER_END_CREDITS_IMAGE 1U

#endif