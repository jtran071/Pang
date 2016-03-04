#include <avr/io.h>
#include "C:\Users\student\Desktop\io.c"
#include <avr/interrupt.h>
//#include "C:\Users\student\Desktop\bit.h"
#include "C:\Users\student\Desktop\timer.h"
#include "C:\Users\student\Desktop\scheduler.h"
#include <util/delay.h>
#include <avr/pgmspace.h>
// #include <time.h>
// #include <stdlib.h>

//randomize
//srand(time(null));

// global variables
unsigned long score = 0;
unsigned char hit_enemy = 0;
unsigned char hit_player = 0;
unsigned char player_pos_x = 0;
unsigned char player_pos_y = 0;
//unsigned char player_pos_count = 0;
unsigned char enemy_pos_x = 0;
unsigned char enemy_pos_y = 0;
unsigned char proj_pos_x = 0;
unsigned char proj_pos_y = 0;
unsigned char proj_count = 0;
unsigned char bounce = 0;

unsigned char enemy1_death = 0;
unsigned char respawn_timer1 = 0;

unsigned char enemy1_death2 = 0;
unsigned char respawn_timer2 = 0;
unsigned char enemy_pos_x2 = 0;
unsigned char enemy_pos_y2 = 0;
unsigned char hit_enemy2 = 0;
unsigned char bounce2 = 0;

unsigned char startFlag = 0;
unsigned char updateFlag = 0;
unsigned char game_over = 0;
unsigned char gg_menu_flag = 0;

unsigned char reset = 0;


//double gg_music[] = {174.61, 164.81, 146.83, 116.54};

typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
};



//task scheduler
task tasks[7];

const unsigned char tasksNum = 7;
///////
const unsigned long periodPlayer = 125;
const unsigned long periodDisplay = 1;
const unsigned long periodProjectile = 125;
const unsigned long periodEnemy = 125;
const unsigned long periodEDisplay = 1;
const unsigned long periodLCD = 125;
const unsigned long periodEnemy2 = 125;
//////////////////////////////////////////////////////////////////////////

const unsigned long tasksPeriodGCD = 1;

void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from ?Shift? register to ?Storage? register
	PORTC |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}
void set_PWM(double frequency) {
	
	
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {
		
		if (!frequency) TCCR3B &= 0x08; //stops timer/counter
		else TCCR3B |= 0x03; // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		// prevents OCR3A from underflowing, using prescaler 64                 // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) OCR3A = 0x0000;
		
		// set OCR3A based on desired frequency
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;
		
		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	//set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

//found here by Scienceprog.com-http://ohm.bu.edu/~pbohn/Cell_Stretcher/Firmware/Programs_for_AVR-MT-C/test_programs/cellstretcher_firmware/lcd_lib.c
void LCDdefinechar(const uint8_t *pc,uint8_t char_code){
	uint8_t a, pcc;
	uint16_t i;
	a=(char_code<<3)|0x40;
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&pc[i]);
		LCD_WriteCommand(a++);
		LCD_WriteData(pcc);
	}
}

//custom characters
//made with-http://omerk.github.io/lcdchargen/ by Omer Kilic
const char custom_player[8] PROGMEM= {
	0b01110,
	0b11111,
	0b01110,
	0b01110,
	0b00101,
	0b01111,
	0b11111,
	0b01110
};
const char custom_player_right[8] PROGMEM= {
	0b01110,
	0b11111,
	0b01110,
	0b01110,
	0b00100,
	0b01111,
	0b01100,
	0b00100
};

const char custom_bubble[8] PROGMEM= {
	0b00000,
	0b01110,
	0b10011,
	0b10111,
	0b11111,
	0b11111,
	0b01110,
	0b00000
};

const char custom_hook_up[8] PROGMEM= {
	0b00100,
	0b01110,
	0b11111,
	0b00100,
	0b01000,
	0b00100,
	0b00010,
	0b00100
};

const char custom_hook_right[8] PROGMEM= {
	0b00000,
	0b00000,
	0b00010,
	0b00011,
	0b11111,
	0b00010,
	0b00000,
	0b00000
};

const char custom_skull [8] PROGMEM= {
	0b10101,
	0b01110,
	0b01110,
	0b10101,
	0b10101,
	0b01010,
	0b01010,
	0b10101
};

const char custom_thumbsup[8] PROGMEM= {
	0b01000,
	0b01000,
	0b01000,
	0b01111,
	0b11110,
	0b11111,
	0b11110,
	0b11111
};

// States ////////////////////////////////////////////////////////////////
enum player_char{init_player, wait_player, left_player, right_player, shoot_player, shoot_player_off, player_reset} player_state;
int player_tick(int state)
{
	unsigned char B_shoot = ~PINB & 0x01;
	unsigned char B_left = ~PINB & 0x04;
	unsigned char B_right = ~PINB & 0x02;
	unsigned char B_reset = ~PINB & 0x08;
	
	//Transitions
	switch(state){
		case -1:
		state = init_player;
		break;
		case init_player:
		state = wait_player;
		break;
		case wait_player:
		//proj_count = 0;
		if(B_left && !B_right)
		{
			state = left_player;
		}
		else if(B_right && !B_left)
		{
			state = right_player;
		}
		else if(B_shoot && proj_count == 0)
		{
			state = shoot_player;
		}
		else if(B_reset)
		{
			state = player_reset;
		}
		else
		{
			
			state = wait_player;
		}
		break;
		case left_player:
		if(B_left)
		{
			state = left_player;
		}
		else{
			state = wait_player;
		}
		break;
		case right_player:
		if(B_right)
		{
			state = right_player;
		}
		else {
			state = wait_player;
		}
		break;
		case shoot_player:
		if(proj_count == 0)
		{
			state = shoot_player_off;
		}
		else if(proj_count == 1)
		{
			state = wait_player;
		}
		break;
		case shoot_player_off:
		state = wait_player;
		break;
		case player_reset:
		state = init_player;
		default:
		state = -1;
		break;
	}  //Transitions
	
	//Actions
	switch(state)
	{
		case init_player:
		player_pos_x = 0x08;
		player_pos_y = 0x01;
		startFlag = 0;
		game_over = 0;
		reset = 0;
		//player_pos_count = 3;
		break;
		case wait_player:
		set_PWM(0);
		if(player_pos_x == enemy_pos_x && player_pos_y == enemy_pos_y)
		{
			game_over = 1;
			gg_menu_flag = 1;
			player_pos_x = 0x00;
			player_pos_y = 0x00;
			set_PWM(80);
			
		}
		if(player_pos_x == enemy_pos_x2 && player_pos_y == enemy_pos_y2)
		{
			game_over = 1;
			gg_menu_flag = 1;
			player_pos_x = 0x00;
			player_pos_y = 0x00;
			set_PWM(80);
			
		}
		break;
		case left_player:
		if(player_pos_x < 0x80)
		{
			set_PWM(300);
			player_pos_x = player_pos_x << 1;
			//player_pos_count++;
		}
		break;
		case right_player:
		if(player_pos_x > 0x01)
		{
			set_PWM(300);
			player_pos_x = player_pos_x >> 1;
			//player_pos_count--;
		}
		break;
		case shoot_player:
		if(B_shoot)
		{
			startFlag = 1;
		}
		if(B_shoot && proj_count == 0)
		{
			set_PWM(161.63);
		
			proj_pos_y = player_pos_y;
			proj_pos_x = player_pos_x;
			transmit_data(proj_pos_y);
			proj_count = 1;
		}
		break;
		case shoot_player_off:
		//delay_ms(1000);
		proj_count = 0;
		set_PWM(0);
		break;
		case player_reset:
		reset = 1;
		PORTD = ~0x18;
		transmit_data(0x18);
		delay_ms(200);
		default:
		break;
	}// Actions
	player_state = state;
	return state;
}

enum projectile{init_proj, wait_proj, proj_increment, reset_proj} proj_state;
int proj_tick(int state)
{
	switch(state) //Transitions
	{
		case -1:
		state = init_proj;
		break;
		case init_proj:
		state = wait_proj;
		break;
		case wait_proj:
		if(proj_count == 1)
		{
			state = proj_increment;
		}
		else if(reset == 1)
		{
			state = reset_proj;
		}
		else
		{
			state = wait_proj;
			
		}
		break;
		case proj_increment:
		if(proj_count == 1)
		{
			state = proj_increment;
		}
		state = wait_proj;
		break;
		
		case reset_proj:
		state = init_proj;
		default:
		state = -1;
		break;
		
	} // Transitions
	
	switch(state) // Actions
	{
		case init_proj:
		break;
		case wait_proj:
		proj_count = 0;
		proj_pos_y = 0;
		proj_pos_x = 0;
		break;
		case proj_increment:
		while(proj_pos_y < 0x80)
		{
			proj_pos_y = proj_pos_y + 1;
			delay_ms(1);
			transmit_data(proj_pos_y);
		}
		proj_count = 0;
		break;
		case reset_proj:
		break;
		default:
		break;
		
	}//Actions
	proj_state = state;
	return state;
}

enum enemy{init_enemy, wait_enemy, enemy_mv, enemy1_respawn, reset_enemy1} enemy_state;
int enemy_tick(int state)
{
	unsigned char wait_hit = 0;
	unsigned char move_hit = 0;
	switch(state) // Transitions
	{
		case -1:
		state = init_enemy;
		break;
		case init_enemy:
		state = wait_enemy;
		break;
		case wait_enemy:
		state = enemy_mv;
		if(enemy1_death == 1)
		{
			state = enemy1_respawn;
		}
		if(reset == 1)
		{
			state = reset_enemy1;
		}
		break;
		case enemy_mv:
		state = wait_enemy;
		break;
		case enemy1_respawn:
		if(enemy1_death == 1)
		{
			state = init_enemy;
		}
		break;
		case reset_enemy1:
		{
			state = init_enemy;
		}
		default :
		state = -1;
		break;
		
	}// Transitions
	
	switch(state) // Actions
	{
		case init_enemy:
		if(score == 0)
		{
			enemy_pos_x = 0x04;
		}
		else if(score % 2 == 0)
		{
			enemy_pos_x = 0x40;
		}
		else if(score % 3 == 0)
		{
			enemy_pos_x = 0x20;
		}
		else if(score % 5 == 0)
		{
			enemy_pos_x = 0x01;
		}
		else
		{
			enemy_pos_x = 0x04;
		}
		enemy_pos_y = 0x80;
		enemy1_death = 0;
		bounce = 0;
		break;
		case wait_enemy:
		hit_enemy = 0;
		
		
		if(enemy_pos_y <= proj_pos_y && enemy_pos_x == proj_pos_x && hit_enemy == 0 && move_hit == 0 && updateFlag == 0)
		{
			set_PWM(500);
			hit_enemy = 1;
			enemy_pos_x = 0x00;
			enemy_pos_y = 0x00;
			score += 101;
			updateFlag = 1;
			enemy1_death = 1;
			wait_hit = 1;
		}
		move_hit = 0;

		break;
		case enemy_mv:
		if(enemy_pos_y <= proj_pos_y && enemy_pos_x == proj_pos_x && hit_enemy == 0 && wait_hit == 0 && updateFlag == 0)
		{
			set_PWM(500);
			hit_enemy = 1;
			enemy_pos_x = 0x00;
			enemy_pos_y = 0x00;
			//score += 100;
			updateFlag = 1;
			enemy1_death = 1;
			move_hit = 1;
		}
		wait_hit = 0;
		if(enemy_pos_y > 0x01 && bounce == 0)
		{
			enemy_pos_y = enemy_pos_y >> 1;
		}
		else
		{
			enemy_pos_y = enemy_pos_y << 1;
			bounce = 1;
			if(enemy_pos_y == 0x10)
			{
				bounce = 0;
			}
		}
		
		break;
		case enemy1_respawn:
		break;
		case reset_enemy1:
		break;
		default:
		break;
	} // Actions
	
	enemy_state = state;
	return state;
}

enum enemy2{init_enemy2, wait_enemy2, enemy_mv2, enemy1_respawn2, reset_enemy2} enemy_state2;
int enemy_tick2(int state)
{
	unsigned char wait_hit = 0;
	unsigned char move_hit = 0;
	switch(state) // Transitions
	{
		case -1:
		state = init_enemy2;
		break;
		case init_enemy2:
		state = wait_enemy2;
		break;
		case wait_enemy2:
		state = enemy_mv2;
		if(enemy1_death2 == 1)
		{
			state = enemy1_respawn2;
		}
		if(reset == 1)
		{
			state = reset_enemy2;
		}
		break;
		case enemy_mv2:
		state = wait_enemy2;
		break;
		case enemy1_respawn2:
		if(enemy1_death2 == 1)
		{
			state = init_enemy2;
		}
		break;
		case reset_enemy2:
		{
			state = init_enemy2;
		}
		default :
		state = -1;
		break;
		
	}// Transitions
	
	switch(state) // Actions
	{
		case init_enemy2:
		if(score == 0)
		{
			enemy_pos_x2 = 0x08;
		}
		else if(score % 2 == 0)
		{
			enemy_pos_x2 = 0x80;
		}
		else if(score % 3 == 0)
		{
			enemy_pos_x2 = 0x10;
		}
		else if(score % 5 == 0)
		{
			enemy_pos_x2 = 0x02;
		}
		else
		{
			enemy_pos_x2 = 0x08;
		}
		enemy_pos_y2 = 0x40;
		enemy1_death2 = 0;
		bounce2 = 0;
		break;
		case wait_enemy2:
		hit_enemy2 = 0;
		
		
		if(enemy_pos_y2 <= proj_pos_y && enemy_pos_x2 == proj_pos_x && hit_enemy2 == 0 && move_hit == 0 && updateFlag == 0)
		{
			set_PWM(500);
			hit_enemy2 = 1;
			enemy_pos_x2 = 0x00;
			enemy_pos_y2 = 0x00;
			score += 101;
			updateFlag = 1;
			enemy1_death2 = 1;
			wait_hit = 1;
		}
		move_hit = 0;

		break;
		case enemy_mv:
		if(enemy_pos_y2 <= proj_pos_y && enemy_pos_x2 == proj_pos_x && hit_enemy2 == 0 && wait_hit == 0 && updateFlag == 0)
		{
			set_PWM(500);
			hit_enemy2 = 1;
			enemy_pos_x2 = 0x00;
			enemy_pos_y2 = 0x00;
			//score += 100;
			updateFlag = 1;
			enemy1_death2 = 1;
			move_hit = 1;
		}
		wait_hit = 0;
		if(enemy_pos_y2 > 0x01 && bounce2 == 0)
		{
			enemy_pos_y2 = enemy_pos_y2 >> 1;
		}
		else
		{
			enemy_pos_y2 = enemy_pos_y2 << 1;
			bounce2 = 1;
			if(enemy_pos_y2 == 0x08)
			{
				bounce2 = 0;
			}
		}
		
		break;
		case enemy1_respawn2:
		break;
		case reset_enemy2:
		break;
		default:
		break;
	} // Actions
	
	enemy_state2 = state;
	return state;
}

enum display{d_init, d_Player, reset_display} display_state;
int display_tick(int state)
{
	
	switch(state) // Transitions
	{
		case -1:
		state = d_init;
		break;
		case d_init:
		state = d_Player;
		break;
		case d_Player:
		if(reset == 1)
		{
			state = reset_display;
		}
		state = d_Player;
		break;
		case reset_display:
		state = d_init;
		break;
		default:
		state = -1;
		break;
	} // Transitions
	
	switch(state) // Actions
	{
		case d_init:
		break;
		
		case d_Player:
		PORTD = ~player_pos_x;
		transmit_data(0x01);
		transmit_data(0x00);
		break;
		
		case reset_display:
		break;
		default:
		break;
		
	} // Actions
	display_state = state;
	return state;
}

enum display_enemy{d_Einit, d_Enemy, reset_d_enemy1} display_estate;
int display_Etick(int state)
{
	
	switch(state) // Transitions
	{
		case -1:
		state = d_Einit;
		break;
		case d_Einit:
		state = d_Enemy;
		break;
		case d_Enemy:
		if(respawn_timer1 == 1000)
		{
			state = d_Einit;
		}
		else if(reset == 1)
		{
			state = reset_d_enemy1;
		}
		else
		{
			state = d_Enemy;
		}
		
		break;
		case reset_d_enemy1:
		state = d_Einit;
		break;
		default:
		state = -1;
		break;
	} // Transitions
	
	switch(state) // Actions
	{
		case d_Einit:
		respawn_timer1 = 0;
		break;
		case d_Enemy:
		if (enemy1_death == 1)
		{
			++respawn_timer1;
		}
		
		delay_ms(1);
		PORTD = ~enemy_pos_x;
		transmit_data(enemy_pos_y);
		transmit_data(0x00);
		
		PORTD = ~enemy_pos_x2;
		transmit_data(enemy_pos_y2);
		transmit_data(0x00);
		
		if(hit_enemy == 1)
		{
			PORTD = enemy_pos_x;
			transmit_data(enemy_pos_y);
			transmit_data(0x00);
		}
		if(hit_enemy2 == 1)
		{
			PORTD = enemy_pos_x2;
			transmit_data(enemy_pos_y2);
			transmit_data(0x00);
		}
		break;
		case reset_d_enemy1:
		break;
		default:
		break;
		
	} // Actions
	display_estate = state;
	return state;
}


enum display_LCD{menu, LCD_score, gg_menu, reset_LCD} LCD_state;
int LCD_tick(int state)
{
	unsigned char B_menu = ~PINB & 0x01;
	switch(state)//Transitions
	{
		case -1:
		state = menu;
		break;
		case menu:
			if(startFlag)
			{
				state = LCD_score;
			}
			else if(game_over == 1 && gg_menu_flag == 1)
			{
				state = gg_menu;
			}
			else if(reset == 1)
			{
				state = reset_LCD;
			}
			break;
		case LCD_score:
			if(game_over == 1 && gg_menu_flag == 1)
			{
				state = gg_menu;
			}
			else if(reset == 1)
			{
				state = reset_LCD;
			}
			else
			{
				state = LCD_score;
			}
			break;
		case gg_menu:
			if(B_menu) //|| startFlag == 1)
			{
				state = menu;
			}
			else if(reset == 1)
			{
				state = reset_LCD;
			}
// 			else
// 			{
// 				state = gg_menu;
// 			}
			break;
		case reset_LCD:
		state = menu;
		break;
		default:
		state = -1;
		break;
		
	} // Transitions
	switch(state) // Action
	{
		case menu:
			if (startFlag == 0)
			{
				LCD_DisplayString(3, " PLAY PANG!");
				LCDdefinechar(custom_player, 0);
				LCD_Cursor(18);
				LCD_WriteData(0b00001000);
				LCDdefinechar(custom_hook_up, 1);
				LCD_Cursor(2);
				LCD_WriteData(0b00001001);
				LCDdefinechar(custom_bubble, 2);
				LCD_Cursor(15);
				LCD_WriteData(0b00001010);
				LCD_Cursor(31);
				LCD_WriteData(0b00001001);
				LCD_Cursor(24);
			}
			
			break;
		case LCD_score:
			if(updateFlag == 1)
			{
				LCD_DisplayString(1, "SCORE: ");
				LCDdefinechar(custom_player_right,3);
				LCD_Cursor(9);
				LCD_WriteData(0b00001011);
				LCDdefinechar(custom_hook_right, 4);
				LCD_Cursor(10);
				LCD_WriteData(0b00001100);
				LCD_Cursor(11);
				LCD_WriteData(0b00001010);
				
				
				LCD_Cursor(17);
				LCD_WriteData( score%1000000 % 100000 /10000 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 /1000 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 /100 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 % 100 /10 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 % 100 % 10 + '0' );
			}
			updateFlag = 0;
			break;
		case gg_menu:
			if(gg_menu_flag == 1)
			{
				LCD_DisplayString(3, " GAME OVER!");
				
				LCDdefinechar(custom_skull,5);
				LCD_Cursor(2);
				LCD_WriteData(0b00001101);
				LCD_Cursor(15);
				LCD_WriteData(0b00001101);
				
				LCD_Cursor(17);
				LCD_WriteData('S');
				LCD_WriteData('C');
				LCD_WriteData('O');
				LCD_WriteData('R');
				LCD_WriteData('E');
				LCD_WriteData(':');
				LCD_WriteData(' ');
				LCD_WriteData( score%1000000 % 100000 /10000 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 /1000 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 /100 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 % 100 /10 + '0' );
				LCD_WriteData( score%1000000 % 100000 % 10000 % 1000 % 100 % 10 + '0' );
				
				LCDdefinechar(custom_thumbsup, 6);
				LCD_Cursor(30);
				LCD_WriteData(0b00001110);
// 				LCD_WriteData('P');
// 				LCD_WriteData('R');
// 				LCD_WriteData('E');
// 				LCD_WriteData('S');
// 				LCD_WriteData('S');
// 				LCD_WriteData(' ');
// 				LCD_WriteData('R');
// 				LCD_WriteData('E');
// 				LCD_WriteData('S');
// 				LCD_WriteData('E');
// 				LCD_WriteData('T');
				
				
			}
			break;
			case reset_LCD:
			break;
		default:
		break;
	} //Action
	LCD_state = state;
	return state;
}

// End of States /////////////////////////////////////////////////////////





int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xF0; PORTB = 0x0F;
	
	
	
	PWM_on();
	LCD_init();
	
	
	
	// Priority assigned to lower position tasks in array
	unsigned char i=0;
	tasks[i].state = -1;
	tasks[i].period = periodPlayer;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &player_tick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodDisplay;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &display_tick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodProjectile;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &proj_tick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodEnemy;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &enemy_tick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodEDisplay;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &display_Etick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodLCD;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &LCD_tick;
	++i;
	
	tasks[i].state = -1;
	tasks[i].period = periodEnemy2;
	tasks[i].elapsedTime = tasks[i].period;
	///
	tasks[i].TickFct = &enemy_tick2;
	++i;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	while(1)
	{	
		if(reset == 1)
		{
			// global variables
			score = 0;
			hit_enemy = 0;
			hit_player = 0;
			player_pos_x = 0;
			player_pos_y = 0;
			
			enemy_pos_x = 0;
			enemy_pos_y = 0;
			proj_pos_x = 0;
			proj_pos_y = 0;
			proj_count = 0;
			bounce = 0;

			enemy1_death = 0;
			respawn_timer1 = 0;
			
			enemy1_death2 = 0;
			respawn_timer2 = 0;
			enemy_pos_x2 = 0;
			enemy_pos_y2 = 0;
			hit_enemy2 = 0;
			bounce2 = 0;

			startFlag = 0;
			updateFlag = 0;
			game_over = 0;
			gg_menu_flag = 0;
			
// 			player_state = -1;
// 			LCD_state = -1;
// 			display_estate = -1;
// 			display_state = -1;
// 			proj_state = -1;
// 			enemy_state = -1;
			
			
		}
		
		
		
		// For each task, call task tick function if task's period is up
		for (i=0; i < tasksNum; i++) {
			if (tasks[i].elapsedTime >= tasks[i].period){
				// Task is ready to tick, so call its tick function
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0; // Reset the elapsed time
			}
			tasks[i].elapsedTime += tasksPeriodGCD;
		}
		// 		//Sleep();
		while(!TimerFlag);
		TimerFlag = 0;
		

	}
	
	return 0;
	
}
