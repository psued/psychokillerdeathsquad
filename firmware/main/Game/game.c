#include "game.h"

#include "audio.h"
#include "input.h"
#include "fpga.h"

#include "game_state.h"
#include "game_audio.h"
#include "player_game_input.h"
#include "render_builder.h"
#include "screen_animation.h"
#include "stage_def.h"
#include "battle.h"
#include "battle_render.h"
#include "round_over.h"
#include "match_over.h"

static RenderBuilder builder;
static Game game;

static void reset_game_state(Game *game){
    game->p1 = (Player){0};
    game->p2 = (Player){0};

    game->p1_char_selection = (CharacterSelectionState){.hovered_char_id = 0};
    game->p2_char_selection = (CharacterSelectionState){.hovered_char_id = 1};

    game->stage_selection = (StageSelectionState){ .hovered_stage_id = STAGE_CHINA};
    game->match_state = (MatchState){0};
}

static void enter_state(Game *game) {
    animation_clear_all();
    audio_free();

    switch (game->state) {
        case GS_START_SCREEN:
            reset_game_state(game);
            enter_start_screen(&game->start_state);
            break;
        case GS_CHARACTER_SELECTION:
            enter_char_select(&game->p1_char_selection, &game->p2_char_selection);
            break;
        case GS_STAGE_SELECT:
            enter_stage_select(&game->stage_selection);
            break;
        case GS_BATTLE:
            enter_battle_state(&game->match_state);
            break;
        case GS_ROUND_OVER:
            enter_round_over();
            break;
        case GS_MATCH_OVER:
            enter_match_over();
            break;
        default:
            break;
    }
}

static void game_update(Game *game, PlayerGameInput p1_input, PlayerGameInput p2_input){
    derive_pressed_state(&p1_input, &game->p1_input_prev);
    update_input_buffer(&game->p1_input_buffer, &p1_input);

    derive_pressed_state(&p2_input, &game->p2_input_prev);
    update_input_buffer(&game->p2_input_buffer, &p2_input);

    /* Update the new previous player inputs to this tick's current */
    game->p1_input_prev = p1_input;
    game->p2_input_prev = p2_input;

    /* Update screen animations */
    animation_update();

    /* States modify requested state instead of game_state directly */
    GameState requested_state = game->state;

    switch (game->state) {
        case GS_START_SCREEN:
            update_start_screen(&requested_state, &game->start_state,  &p1_input, &p2_input);
            break;

        case GS_CHARACTER_SELECTION:
            update_char_select(
                &requested_state,
                &game->p1_char_selection,
                &game->p2_char_selection,
                &p1_input,
                &p2_input
            );
            break;

        case GS_STAGE_SELECT:
            update_stage_select(
                &requested_state,
                &game->match_state,
                &game->stage_selection,
                &game->p1,
                &game->p2,
                &game->p1_char_selection,
                &game->p2_char_selection,
                &p1_input,
                &p2_input
            );
            break;

        case GS_BATTLE:
            update_battle_state(
                &requested_state,
                &game->match_state,
                &stage_table[game->match_state.stage_id],
                &game->p1,
                &game->p2,
                &p1_input,
                &p2_input,
                &game->p1_input_buffer,
                &game->p2_input_buffer
            );
            break;

        case GS_ROUND_OVER:
            update_round_over(
                &requested_state,
                &game->match_state,
                &stage_table[game->match_state.stage_id],
                &game->p1,
                &game->p2,
                &p1_input,
                &p2_input
            );
            break;

        case GS_MATCH_OVER:
            update_match_over(
                &requested_state,
                &p1_input,
                &p2_input
            );
            break;

        default:
            break;
    }

    /* Will fire if a game state updated the requested state */
    if (requested_state != game->state){
        game->state = requested_state;
        enter_state(game);
    }
}

static void game_build_render(const Game *game, RenderBuilder *builder){
    render_begin(builder);

    /* Render based on game state */
    switch (game->state) {
        case GS_START_SCREEN:
            build_start_screen_render(builder);
            break;

        case GS_CHARACTER_SELECTION:
            build_char_select_render(
                builder,
                &game->p1_char_selection,
                &game->p2_char_selection
            );
            break;

        case GS_STAGE_SELECT:
            build_stage_select_render(
                builder,
                &game->stage_selection
            );
            break;

        case GS_BATTLE:
            build_battle_render(
                builder,
                &stage_table[game->match_state.stage_id],
                &game->match_state,
                &game->p1,
                &game->p2
            );
            break;

        case GS_ROUND_OVER:
            build_round_over_render(
                builder,
                &stage_table[game->match_state.stage_id],
                &game->match_state,
                &game->p1,
                &game->p2
            );
            break;

        case GS_MATCH_OVER:
            build_match_over_render(
                builder,
                &game->match_state,
                &game->p1,
                &game->p2
            );
            break;

        default:
            break;
    }
}

void game_init(){
    game = (Game){ .state = GS_START_SCREEN };
    init_game_volume_level();

    enter_state(&game);
}

void game_tick(void)
{
	const ControllerState player1 = input_get(INPUT_PLAYER_1);
	const ControllerState player2 = input_get(INPUT_PLAYER_2);

	const PlayerGameInput p1_input = read_controller_input(player1);
	const PlayerGameInput p2_input = read_controller_input(player2);

	game_update(&game, p1_input, p2_input);
	game_build_render(&game, &builder);
	fpga_submit(&builder.snapshot);
}
