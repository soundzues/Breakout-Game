#include <ncurses.h>	// For drawing shapes on the screen
#include <unistd.h>		// For usleep

#define DELAY 30000	// Passed to usleep() within main loop to control game speed
#define BALL_DELAY 4 // So ball doesn't move too fast compared to paddle (incremented once per loop iteration)
#define MAX_X 67	// Board width
#define MAX_Y 24	// Board height
#define PAD_Y 23	// Paddle distance from top 
#define BLOCK_W 17	// Block width
#define BLOCK_H 4	// Block height
#define BLOCK_SP 1	// Block spacing
#define BLOCK_ROWS 2 // Num of rows of blocks
#define BLOCK_COLS 4 // Num of columns of blocks

// Block sprites that keep track of where they are on an X/Y coordinate system
struct BlockMap {
	char sprite;
	char *grid_member;
};

// Blocks 2-dimensional array. 0 = not broken, 1 = broken
// These get expanded to the full blocks and placed in a grid of sprites (chars)
// according to the specified width & height of a block
char blocks[BLOCK_ROWS][BLOCK_COLS] = { 0 };

// Grid of sprites for keeping track of where to draw block sprites and create boundaries
struct BlockMap block_sprites[BLOCK_ROWS * BLOCK_H][BLOCK_COLS * BLOCK_W];

// Takes y and x coordinate of top left corner of block and adds it to a sprite grid char by char
void make_block (int y, int x) {
	for (int i = 0; i < BLOCK_H; ++i) {
		for (int j = 0; j < BLOCK_W; ++j) {
			// Top row of block
			if (i == 0) {
				if (j == 0 || j == BLOCK_W - 1) {
					block_sprites[y + i][x + j].sprite = '.'; // Top corner
				}
				else {
					block_sprites[y + i][x + j].sprite = '-'; // Top edge
				}
			}
			// Bottom row of block
			else if (i == BLOCK_H - 1) {
				if (j == 0 || j == BLOCK_W - 1) {
					block_sprites[y + i][x + j].sprite = '\''; // Bottom corner
				}
				else {
					block_sprites[y + i][x + j].sprite = '-'; // Bottom edge
				}
			}
			// Left or right edge
			else if (j == 0 || j == BLOCK_W - 1) {
				block_sprites[y + i][x + j].sprite = '|';
			}
			// Middle of block
			else {
				block_sprites[y + i][x + j].sprite = ' ';
			}
			// Point back to the row and column of the blocks array that these sprites correspond to
			block_sprites[y + i][x + j].grid_member = &blocks[y/BLOCK_H][x/BLOCK_W];
		}
	}
}

// Takes y and x coordinates of upper left corner of a block 
// and flips all of the block's sprites (chars) to nulls so it no longer displays
void break_block (int y, int x) {
	for (int i = 0; i < BLOCK_H; ++i) {
		for (int j = 0; j < BLOCK_W; ++j) {
			block_sprites[y + i][x + j].sprite = '\0';
		}
	}
}

// Loops through the block sprite grid and prints it out char by char
void print_blocks (void) {
	char sprite_buff[2] = { '\0' }; // Initialize buffer to nulls

	// Loop through sprite grid
	for (int i = 0; i < BLOCK_ROWS * BLOCK_H; ++i) {
		for (int j = 0; j < BLOCK_COLS * BLOCK_W; ++j) {
			sprite_buff[0] = block_sprites[i][j].sprite; // Create string (char followed by nul)
			mvprintw(i, j, sprite_buff); // Draw sprite using above-created string
		}
	}
}

// Checks to see whether all the blocks are broken (1 values in blocks array)
char game_won (void) {
	char zero_seen = 0; // Boolean. Have we encountered a zero in the blocks array?

	for (int i = 0; i < BLOCK_ROWS; ++i) {
		for (int j = 0; j < BLOCK_COLS; ++j) {
			if (blocks[i][j] == 0) {
				zero_seen = 1;
			}
		}
	}

	// If we saw a zero, return zero (game not won)
	return zero_seen ? 0 : 1;
}




int  input(int pad_x, int key)
{
                switch (key) {
                        case KEY_LEFT:
                                if (pad_x > 1)
                                        pad_x -= 1;
                                break;
                        case KEY_RIGHT:
                                if (pad_x < MAX_X - 3)
                                        pad_x += 1;
                                break;
                        default:
                                break;
                } 
	return pad_x;
}

int main(int argc, char *argv[]) {
	int ball_x = 0,
		ball_y = BLOCK_H * BLOCK_ROWS; // Ball x/y coords
	int pad_x = 20;				// Paddle x starting position
	int next_x = 0, next_y = 0;	// Next x/y coords of ball
	int x_direction = 1, y_direction = 1;	// Ball x/y direction
	int key;					// Key pressed
	// Iterators
	int i = 1,	// Delay timer		
		j = 1,	// Outter for loop iterator
		k = 0;	// Inner for loop iterator

	// Initialize screen
	initscr();
	// Setup keyboard
	keypad(stdscr, TRUE);
	// Don't echo inputs to screen
	noecho();
	// Don't display cursor
	curs_set(FALSE);
	// Don't block on getch()
	nodelay(stdscr, TRUE);
	cbreak();

	// Global var `stdscr` is created by the call to `initscr()`
	//getmaxyx(stdscr, MAX_Y, MAX_X);

	while(1) {
		clear();
		
		// Ceiling
		mvprintw(0, 0, ".");
		for (j = 1; j < MAX_X; ++j) {
			mvprintw(0, j, "-");
		}
		mvprintw(0, MAX_X, ".");

		// Left wall
		for (j = 1; j <= MAX_Y; ++j) {
			mvprintw(j, 0, "|");
		}
		// Right wall
		for (j = 1; j <= MAX_Y; ++j) {
			mvprintw(j, MAX_X, "|");
		}

		// Blocks
		for (j = 0; j < BLOCK_ROWS; ++j) {
			for (k = 0; k < BLOCK_COLS; ++k) {
				if ((int) blocks[j][k] == 0) {
					make_block(j * BLOCK_H, k * BLOCK_W);
				}
				else {
					break_block(j * BLOCK_H, k * BLOCK_W);
				}
			}
		}
		print_blocks();

		// Ball position
		mvprintw(ball_y, ball_x, "o");
		//Paddle position
		mvprintw(PAD_Y, pad_x, "---");
		// Refresh window
		refresh();

		// A little delay
		usleep(DELAY);

		// Set up next x/y coordinates for ball
		next_x = ball_x + x_direction;
		next_y = ball_y + y_direction;

		// Only execute this block if we've waited enough cycles (ball delay)
		if (i >= BALL_DELAY ) {
			// X Collisions
			if (next_x >= MAX_X || next_x <= 0) {
				x_direction *= -1; // Change L/R direction
			}
			// Side of block
			else if (next_y < BLOCK_ROWS * BLOCK_H && next_x < BLOCK_COLS * BLOCK_W && *(block_sprites[next_y][next_x].grid_member) == 0 && block_sprites[next_y][next_x].sprite == '|') {
				x_direction *= -1; // Change L/R direction
				// Don't break block for a dot. We'll do that on checking for y direction below
				*(block_sprites[next_y][next_x].grid_member) = (char) 1; // Break block
			}
			// No X collision
			else {
				ball_x += x_direction; // Continue in same direction
			}

			// Y Collisions
			if (next_y <= 0 || (next_y == PAD_Y && next_x >= pad_x && next_x <= pad_x + 3) ) {
				y_direction *= -1; // Change Up/Down direction
			}
			// Top/Bottom of block
			else if (next_y < BLOCK_ROWS * BLOCK_H && next_x < BLOCK_COLS * BLOCK_W && *(block_sprites[next_y][next_x].grid_member) == 0 && block_sprites[next_y][next_x].sprite == '-') {
				y_direction *= -1; // Change Up/Down direction
				*(block_sprites[next_y][next_x].grid_member) = (char) 1; // Break block
			}
			else if (next_y > MAX_Y) {
				// Ball fell through, end game
				endwin();
				printf("\n\t#################\n");
				printf("\t# You lose! >:( #\n");
				printf("\t#################\n\n");
				return 0;
			}
			// No Y collision
			else {
				ball_y += y_direction; // Continue in same direction
			}
			i = 0; // Reset delay timer
		}

		// All blocks gone?
		if (game_won()) {
			endwin();
			printf("\n\t################\n");
			printf("\t# You win! >:) #\n");
			printf("\t################\n\n");
			return 0;
		}

		// Keypress
		key = getch();
		pad_x = input(pad_x, key);
		
/*		switch (key) {
			case KEY_LEFT:
				if (pad_x > 1)
					pad_x -= 1;
				break;
			case KEY_RIGHT:
				if (pad_x < MAX_X - 3)
					pad_x += 1;
				break;
			default:
				break;
		}*/
		++i; // Increment delay timer
	}

	// If we somehow got here, exit out
	endwin();
	return 0;
}
