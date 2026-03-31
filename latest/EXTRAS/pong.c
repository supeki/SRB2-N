// Nozomi Pong!

#include "../console.h"
#include "../doomdef.h"
#include "../d_main.h"
#include "../d_netcmd.h"
#include "../g_game.h"
#include "../g_input.h"
#include "../g_state.h"
#include "../m_random.h"
#include "../m_menu.h"
#include "../r_draw.h"
#include "../r_main.h"
#include "../r_things.h"
#include "../s_sound.h"
#include "../st_lib.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "pong.h"

typedef struct {
    int x, y;
    int dx, dy;

    patch_t* skin;
} pongball_t;

typedef struct
{
	int score;
	int x;
	int y;
	int width;
	int height;
} pongpaddle_t;

// proto my types..
// paddles
pongpaddle_t paddle1;
pongpaddle_t paddle2;

// ball
pongball_t ball;

// score
int score1;
int score2;
char realscore1[16];
char realscore2[16];

// Command that starts our Pong gamestate.
static void Command_StartPong_f(void)
{
	if (gamestate != GS_NOZOMIPONG && wipegamestate != GS_NOZOMIPONG) {
		gamestate = GS_NOZOMIPONG;
		wipegamestate = -1;
	}

	CON_ToggleOff();
}

void resetBall(void)
{
	ball.x = BASEVIDWIDTH/2;
	ball.y = BASEVIDHEIGHT/2;
    ball.dx = (P_Random() & 1) ? 2 : -2;
    ball.dy = (P_Random() % 3) - 1;
}

void DrawBall(void) // more balls -xdf
{
	ball.skin = W_CacheLumpName("CHAOS1", PU_CACHE);
	V_DrawScaledPatch(ball.x, ball.y, 0, ball.skin);
}

// Initialize a bunch of stuff yaya! Nozomi 03-10-2026
void D_InitNozomiPong(void) 
{
	COM_AddCommand ("pong", Command_StartPong_f);

	memset(&ball, 0, sizeof(pongball_t));

    ball.x = BASEVIDWIDTH/2;
    ball.y = BASEVIDHEIGHT/2;
    ball.dx = 2;
    ball.dy = 1;

    paddle1.y = BASEVIDWIDTH/2;
	paddle2.y = BASEVIDHEIGHT/2;
	paddle1.x = 30;
	paddle2.x = BASEVIDWIDTH-38;
	paddle1.width = 8;
	paddle1.height = 32;
	paddle2.width = 8;
	paddle2.height = 32;
    paddle1.score = paddle2.score = 0;
}

void T_PongTicker(void)
{
    ball.x += ball.dx;
    ball.y += ball.dy;

    if (ball.y <= 16 || ball.y >= BASEVIDHEIGHT-16) 
		ball.dy = -ball.dy;

	// paddle left
	if (ball.x >= paddle1.x && ball.x <= paddle1.x + paddle1.width &&
		ball.y >= paddle1.y - paddle1.height/2 &&
		ball.y <= paddle1.y + paddle1.height/2)
	{
		ball.dx = -ball.dx;
		ball.x = paddle1.x + paddle1.width + 1;

        int hit = ball.y - paddle1.y;
        ball.dy = hit / 4;
	}

	// paddle right
	if (ball.x >= paddle2.x - paddle2.width && ball.x <= paddle2.x + paddle2.width &&
		ball.y >= paddle2.y - paddle2.height/2 &&
		ball.y <= paddle2.y + paddle2.height/2)
	{
		ball.dx = -ball.dx;
		ball.x = paddle2.x - paddle2.width - 1;

        int hit = ball.y - paddle2.y;
        ball.dy = hit / 4;
	}

    // score right
    if (ball.x < 0)
    {
        paddle2.score++;
		resetBall();
    }
    else if (ball.x > 320) // score left
    {
        paddle1.score++;
		resetBall();
    }

	// score
	int score1 = paddle1.score;
	int score2 = paddle2.score;
	sprintf(realscore1, "%d", score1);
	sprintf(realscore2, "%d", score2);

	// paddle controls
	if (gamekeydown[gamecontrol[gc_forward][0]] || gamekeydown[gamecontrol[gc_forward][1]])
		paddle1.y -= 4;
	if (gamekeydown[gamecontrol[gc_backward][0]] || gamekeydown[gamecontrol[gc_backward][1]])
		paddle1.y += 4;
	// player 2?
	if (gamekeydown[gamecontrol[gc_jump][0]] || gamekeydown[gamecontrol[gc_jump][1]])
		paddle2.y -= 4;
	if (gamekeydown[gamecontrol[gc_use][0]] || gamekeydown[gamecontrol[gc_use][1]])
		paddle2.y += 4;
}


void T_PongDrawer(void)
{
	int i;

	// Draw a black background so we don't have HOMs.
	V_DrawFill(0, 0, vid.width, vid.height, 0); 

	// paddle
	V_DrawFill(paddle1.x, paddle1.y-paddle1.height/2, paddle1.width, paddle1.height, 48);
	V_DrawFill(paddle2.x, paddle2.y-paddle2.height/2, paddle2.width, paddle2.height, 48); 

	V_DrawString(BASEVIDWIDTH/2-20, 20, realscore1);
	V_DrawString(BASEVIDWIDTH/2+20, 20, realscore2);

	DrawBall();

	for (i = 1; i < BASEVIDHEIGHT/15; i++) // dotted line in the center
		V_DrawStringWhite(BASEVIDWIDTH/2, i*16, ".");
}
