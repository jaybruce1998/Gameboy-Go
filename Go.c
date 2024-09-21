#include <ctype.h>
#include <gb/gb.h>
#include "square.h"
char board[19][19];
char last[19][19];
char temp[19][19];
char player;
UINT8 black, white, x, y, scrolled, showing, pass, playing, size, hp, vp;
unsigned char icon[3];
void play(UINT8 r, UINT8 c);
UINT8 capture(UINT8 r, UINT8 c);
void showScore(UINT8 n, UINT8 i)
{
	icon[0]=n/100+0xb;
	icon[1]=n/10%10+0xb;
	icon[2]=n%10+0xb;
	if(icon[0]==0xb)
	{
		icon[0]=0x0;
		if(icon[1]==0xb)
			icon[1]=0x0;
	}
	set_win_tiles(1, i, 3, 1, icon);
}
void init(void)
{
	UINT8 i, j;
	for(i=0; i<19; i++)
		for(j=0; j<19; j++)
			board[i][j]='.';
	player='b';
	black=0;
	white=0;
	x=8;
	y=16;
	showing=0;
	pass=0;
	hp=size*8;
	vp=hp+8*(size<19);
	move_sprite(0, 0, 0);
	move_sprite(1, 0, 0);
	set_bkg_tiles(0, 0, 19, 19, square_map);
	showScore(black, 0);
	showScore(white, 1);
	HIDE_WIN;
}
void uncapture(void)
{
	UINT8 i, j;
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			board[i][j]=tolower(board[i][j]);
}
UINT8 dead(UINT8 r, UINT8 c)
{
	if(r>=size||c>=size)
		return 1;
	if(board[r][c]!=player)
		return board[r][c]!='.';
	board[r][c]=toupper(board[r][c]);
	return dead(r-1, c)&&dead(r+1, c)&&dead(r, c-1)&&dead(r, c+1);
}
UINT8 canDie(UINT8 r, UINT8 c)
{
	if(board[r][c]!=player)
		return 0;
	r=dead(r, c);
	uncapture();
	return r;
}
UINT8 noKo(UINT8 r, UINT8 c)
{
	UINT8 i, j;
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			temp[i][j]=board[i][j];
	icon[2]=0;
	player=player=='b'?'w':'b';
	capture(r-1, c)+capture(r+1, c)+capture(r, c-1)+capture(r, c+1);
	player=player=='b'?'w':'b';
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
		{
			icon[2]=icon[2]||(board[i][j]!=last[i][j]);
			board[i][j]=temp[i][j];
		}
	return icon[2];
}
UINT8 canPlay(UINT8 r, UINT8 c)
{
	if(board[r][c]!='.')
		return 0;
	board[r][c]=player;
	player=player=='b'?'w':'b';
	UINT8 b=(r>0&&canDie(r-1, c))||(r<size-1&&canDie(r+1, c))||(c>0&&canDie(r, c-1))||(c<size-1&&canDie(r, c+1));
	player=player=='b'?'w':'b';
	b=(b||!canDie(r, c))&&noKo(r, c);
	board[r][c]='.';
	return b;
}
UINT8 capture(UINT8 r, UINT8 c)
{
	UINT8 i, j, n=0;
	if(dead(r, c))
	{
		for(i=0; i<19; i++)
			for(j=0; j<19; j++)
				if(isupper(board[i][j]))
				{
					board[i][j]='.';
					if(icon[2])
					{
						icon[0]=i==0?j==0?1:j==size-1?3:2:i==size-1?j==0?8:j==size-1?0xa:9:j==0?4:j==size-1?6:size==19&&(i==3||i==10||i==16)&&(j==3||j==10||j==16)||
						size==13&&(i==3&&j==3||i==3&&j==9||i==6&&j==6||i==9&&j==3||i==9&&j==9)||
						size==9&&(i==2&&j==2||i==2&&j==6||i==4&&j==4||i==6&&j==2||i==6&&j==6)?7:5;
						set_bkg_tiles(j, i, 1, 1, icon);
					}
					n++;
				}
	}
	else
		uncapture();
	return n;
}
void switchPlayers(void)
{
	if(player=='b')
	{
		player='w';
		move_sprite(0, 0, 0);
		move_sprite(1, x, y);
	}
	else
	{
		player='b';
		move_sprite(0, x, y);
		move_sprite(1, 0, 0);
	}
}
void play(UINT8 r, UINT8 c)
{
	UINT8 i, j;
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			last[i][j]=board[i][j];
	board[r][c]=player;
	pass=0;
	switchPlayers();
	UINT8 n=capture(r-1, c)+capture(r+1, c)+capture(r, c-1)+capture(r, c+1);
	if(player=='b')
	{
		white+=n;
		set_bkg_tiles(c, r, 1, 1, white_piece_map);
		showScore(white, 1);
	}
	else
	{
		black+=n;
		set_bkg_tiles(c, r, 1, 1, black_piece_map);
		showScore(black, 0);
	}
}
UINT8 owner(UINT8 r, UINT8 c, char p)
{
	if(r>=size||c>=size||board[r][c]==p||board[r][c]=='x')
		return 1;
	if(board[r][c]=='.')
	{
		board[r][c]='x';
		return owner(r-1, c, p)&&owner(r+1, c, p)&&owner(r, c-1, p)&&owner(r, c+1, p);
	}
	return 0;
}
UINT8 points(UINT8 r, UINT8 c, char p)
{
	if(r>=size||c>=size)
		return 0;
	UINT8 n=0, i, j;
	if(owner(r, c, p))
	{
		p=toupper(p);
		for(i=0; i<size; i++)
			for(j=0; j<size; j++)
				if(board[i][j]=='x')
				{
					board[i][j]=p;
					n++;
				}
	}
	else
		for(i=0; i<size; i++)
			for(j=0; j<size; j++)
				if(board[i][j]=='x')
					board[i][j]='X';
	return n;
}
void addTerritory(void)
{
	UINT8 i, j;
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			if(board[i][j]=='b')
				black+=points(i-1, j, 'b')+points(i+1, j, 'b')+points(i, j-1, 'b')+points(i, j+1, 'b');
			else if(board[i][j]=='w')
				white+=points(i-1, j, 'w')+points(i+1, j, 'w')+points(i, j-1, 'w')+points(i, j+1, 'w');
}
void fixBackground(void)
{
	init();
	if(size==9)
	{
		set_bkg_tiles(8, 0, 1, 9, nine_vertical);
		set_bkg_tiles(0, 8, 8, 1, nine_horizontal);
		set_bkg_tiles(3, 3, 1, 1, normal);
		set_bkg_tiles(2, 2, 1, 1, special);
		set_bkg_tiles(6, 2, 1, 1, special);
		set_bkg_tiles(4, 4, 1, 1, special);
		set_bkg_tiles(2, 6, 1, 1, special);
		set_bkg_tiles(6, 6, 1, 1, special);
	}
	else if(size==13)
	{
		set_bkg_tiles(12, 0, 1, 13, thirteen_vertical);
		set_bkg_tiles(0, 12, 12, 1, thirteen_horizontal);
		set_bkg_tiles(2, 2, 1, 1, normal);
		set_bkg_tiles(6, 2, 1, 1, normal);
		set_bkg_tiles(4, 4, 1, 1, normal);
		set_bkg_tiles(2, 6, 1, 1, normal);
		set_bkg_tiles(10, 3, 1, 1, normal);
		set_bkg_tiles(3, 10, 1, 1, normal);
		set_bkg_tiles(6, 6, 1, 1, special);
		set_bkg_tiles(3, 3, 1, 1, special);
		set_bkg_tiles(9, 3, 1, 1, special);
		set_bkg_tiles(3, 9, 1, 1, special);
		set_bkg_tiles(9, 9, 1, 1, special);
	}
}
void checkInput(void) {
	if(joypad()&J_A&&playing)
	{
		icon[0]=y/8+scrolled-2;
		icon[1]=x/8-1;
		if(canPlay(icon[0], icon[1]))
			play(icon[0], icon[1]);
		waitpadup();
	}
	if(joypad()&J_B&&playing)
	{
		if(pass)
		{
			move_sprite(0, 0, 0);
			move_sprite(1, 0, 0);
			addTerritory();
			showScore(black, 0);
			showScore(white, 1);
			playing=0;
		}
		else
		{
			pass=1;
			switchPlayers();
		}
		waitpadup();
	}
	if(joypad()&J_START)
	{
		move_sprite(2, 0, 0);
		ENABLE_RAM_MBC1;
		char* saved=(char*)0xa000;
		if(playing)
		{
			saved[0]=size;
			saved[1]=player;
			saved[2]=black;
			saved[3]=white;
			saved[4]=pass;
			icon[0]=5;
			for(icon[1]=0; icon[1]<13; icon[1]++)
				for(icon[2]=0; icon[2]<19; icon[2]++)
					saved[icon[0]++]=board[icon[1]][icon[2]];
			saved=(char*)0xa0fc;
			icon[0]=0;
			for(icon[1]=13; icon[1]<19; icon[1]++)
				for(icon[2]=0; icon[2]<19; icon[2]++)
					saved[icon[0]++]=board[icon[1]][icon[2]];
		}
		else
		{
			if(saved[0])
			{
				size=saved[0];
				saved[0]=0;
				fixBackground();
				player=saved[1];
				if(player=='w')
				{
					player='b';
					switchPlayers();
				}
				black=saved[2];
				white=saved[3];
				showScore(black, 0);
				showScore(white, 1);
				pass=saved[4];
				icon[0]=5;
				for(icon[1]=0; icon[1]<13; icon[1]++)
					for(icon[2]=0; icon[2]<19; icon[2]++)
					{
						board[icon[1]][icon[2]]=saved[icon[0]++];
						if(board[icon[1]][icon[2]]=='b')
							set_bkg_tiles(icon[2], icon[1], 1, 1, black_piece_map);
						else if(board[icon[1]][icon[2]]=='w')
							set_bkg_tiles(icon[2], icon[1], 1, 1, white_piece_map);
					}
				saved=(char*)0xa0fc;
				icon[0]=0;
				for(icon[1]=13; icon[1]<19; icon[1]++)
					for(icon[2]=0; icon[2]<19; icon[2]++)
					{
						board[icon[1]][icon[2]]=saved[icon[0]++];
						if(board[icon[1]][icon[2]]=='b')
							set_bkg_tiles(icon[2], icon[1], 1, 1, black_piece_map);
						else if(board[icon[1]][icon[2]]=='w')
							set_bkg_tiles(icon[2], icon[1], 1, 1, white_piece_map);
					}
			}
			else
				fixBackground();
			if(size<19)
			{
				set_bkg_tiles(size, 0, 10, 18, empty);
				set_bkg_tiles(0, size, 18, 10, empty);
			}
			playing=1;
		}
		DISABLE_RAM_MBC1;
		waitpadup();
	}
	if(joypad()&J_SELECT)
	{
		if(playing)
		{
			if(showing)
				HIDE_WIN;
			else
				SHOW_WIN;
			showing=!showing;
		}
		else
		{
			if(pass)
				SHOW_WIN;
			if(size==19)
			{
				size=9;
				set_bkg_tiles(8, 0, 1, 9, nine_vertical);
				set_bkg_tiles(0, 8, 8, 1, nine_horizontal);
			}
			else if(size==9)
			{
				size=13;
				set_bkg_tiles(8, 0, 1, 9, fixed_vertical);
				set_bkg_tiles(0, 8, 8, 1, fixed_horizontal);
				set_bkg_tiles(12, 0, 1, 13, thirteen_vertical);
				set_bkg_tiles(0, 12, 12, 1, thirteen_horizontal);
			}
			else
			{
				size=19;
				set_bkg_tiles(12, 0, 1, 13, fixed_vertical);
				set_bkg_tiles(0, 12, 12, 1, fixed_horizontal);
			}
		}
		waitpadup();
	}
	if (joypad() & J_UP) {
		if(y==16)
		{
			if(scrolled)
			{
				scroll_bkg(0, -8);
				scrolled=0;
			}
			else
			{
				if(size==19)
				{
					scroll_bkg(0, 8);
					scrolled=1;
				}
				y=vp;
			}
		}
		else
			y-=8;
		waitpadup();
	}
	if (joypad() & J_DOWN) {
		if(y==vp)
		{
			if(scrolled)
			{
				scroll_bkg(0, -8);
				scrolled=0;
				y=16;
			}
			else if(size==19)
			{
				scroll_bkg(0, 8);
				scrolled=1;
			}
			else
				y=16;
		}
		else
			y+=8;
		waitpadup();
	}
	if (joypad() & J_LEFT) {
		if(x==8)
			x=hp;
		else
			x-=8;
		waitpadup();
	}
	if (joypad() & J_RIGHT) {
		if(x==hp)
			x=8;
		else
			x+=8;
		waitpadup();
	}
	move_sprite(playing?player=='w':2, x, y);
}
void updateSwitches(void) {
	SHOW_SPRITES;
	SHOW_BKG;
}
void main(void)
{
	scrolled=0;
	size=19;
	init();
	set_bkg_data(0, 21, square_tiles);
	set_bkg_data(21, 1, black_tile);
	set_bkg_data(22, 1, white_tile);
	set_bkg_palette(0, 1, square_palettes);
	set_sprite_data(0, 1, black_tile);
	set_sprite_data(1, 1, white_tile);
	set_sprite_data(2, 1, x_tile);
	set_sprite_tile(0, 0);
	set_sprite_tile(1, 1);
	set_sprite_tile(2, 2);
	move_win(136, 128);
	set_win_tiles(0, 0, 1, 1, black_piece_map);
	set_win_tiles(0, 1, 1, 1, white_piece_map);
	DISPLAY_ON;
	while(1) {
		checkInput();
		updateSwitches();
		wait_vbl_done();
	}
}
/*
void main(void) {
	printf("A: Send\nB: Receive\n\n");
	_io_out = 1;
	while (1) {
		if (joypad() == J_A) {
			waitpadup();
			send_byte(); // send _io_out
			printf("Sending...\n");
			while (_io_status == IO_SENDING); // Wait for Send
			if (_io_status == IO_IDLE) // If IO status returns to Idle then success
				printf("Sent %d\n", (int)_io_out);
			else
				printf("Error\n"); // Else print error code
			_io_out++;
		}
		if (joypad() == J_B) {
			waitpadup();
			receive_byte(); // receive _io_in
			printf("Receiving...\n");
			while (_io_status == IO_RECEIVING); // Wait for Receive
			if (_io_status == IO_IDLE) // If IO status returns to Idle then success
				printf("Received %d\n", (int)_io_in);
			else
				printf("Error\n"); // Else print error
		}
	}
}
*/