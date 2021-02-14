#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "emu.h"

u16 memory[0x8000];
u16 registers[0x8];
u16 stack[0x100];
u8 input_buffer[0xff];

u16 pc = 0x0;
u16 sp = 0x0;
u8 bp = 0x0;
int output_index_x = 0;
int output_index_y = 21;
int print_index = 0;

void execute_opcode(u8 opcode);
u16 decode_arg(u16 arg);
void set_reg(u16 reg, u16 value);
u16 get_reg(u16 reg);
void print_memory(u16 index);
void print_registers();
void print_stack();
void print_gui(u8 opcode);

int main(int argc, char* argv[]){
	initscr();
	start_color();
	curs_set(0);
	
	FILE *fd = fopen("challenge.bin", "r");
	if(fd == NULL){
		perror("File not found\n");
		exit(1);
	}

	int result = 1;
	int index = 0;
	u8 lower;
	u8 higher;
	
	while(result > 0){
		fread(&lower, sizeof(u8), 1, fd);
		result = fread(&higher, sizeof(u8), 1, fd);
		memory[index++] = (higher << 8) | lower;
	}
	fclose(fd);

	while(1){
		u8 opcode = memory[pc];

		//print_gui(opcode);
		
		/*char input = getch(); 
		if(input == 'q'){
			break;
		}*/

		execute_opcode(opcode);
		if(pc >= print_index + 20 * 15 - 2 || pc < print_index){
			print_index = pc;
		}
	}
	endwin();
	return 0;
}

void execute_opcode(u8 opcode){
	u16 a;
	u16 b;
	u16 c;
	u16 value;

	switch(opcode){
		case 0x0:
			clrtobot();
			mvprintw(21, 0, "Program Halted");
			refresh();
			getch();
			endwin();
			exit(0);
			break;
		case 0x1:;
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			set_reg(a, b);
			break;
		case 0x2:
			a = memory[++pc];
			a = decode_arg(a);
			stack[sp++] = a;
			break;
		case 0x3:
			if(sp == 0){
				mvprintw(21, 0, "ERROR: Empty Stack");
				refresh();
				getch();
				endwin();
				exit(0);
			}
			a = memory[++pc];
			set_reg(a, stack[--sp]);
			stack[sp] = 0;
			break;
		case 0x4:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc]; 
			c = decode_arg(c);
			value = b == c ? 1 : 0;
			set_reg(a, value);
			break;
		case 0x5:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc]; 
			c = decode_arg(c);
			value = b > c ? 1 : 0;
			set_reg(a, value);
			break;
		case 0x6:
			a = memory[++pc];
			a = decode_arg(a);
			pc = a - 1;
			break;
		case 0x7:
			a = memory[++pc];
			a = decode_arg(a);
			b = memory[++pc];
			b = decode_arg(b);
			if(a != 0){
				pc = b - 1;
			}
			break;
		case 0x8:
			a = memory[++pc];
			a = decode_arg(a);
			b = memory[++pc];
			b = decode_arg(b);
			if(a == 0){
				pc = b - 1;
			}
			break;
		case 0x9:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc];
			c = decode_arg(c);
			set_reg(a, (b + c) % 32768);
			break;
		case 0xa:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc];
			c = decode_arg(c);
			set_reg(a, (b * c) % 32768);
			break;
		case 0xb:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc];
			c = decode_arg(c);
			set_reg(a, b % c);
			break;
		case 0xc:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc];
			c = decode_arg(c);
			set_reg(a, b & c);
			break;
		case 0xd:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			c = memory[++pc];
			c = decode_arg(c);
			set_reg(a, b | c);
			break;
		case 0xe:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			set_reg(a, (b & 0x7fff) ^ 0x7fff);
			break;
		case 0xf:
			a = memory[++pc];
			b = memory[++pc];
			b = decode_arg(b);
			set_reg(a, memory[b]);
			break;
		case 0x10:
			a = memory[++pc];
			a = decode_arg(a);
			b = memory[++pc];
			b = decode_arg(b);
			memory[a] = b;
			break;
		case 0x11:
			stack[sp++] = pc + 2;
			a = memory[++pc];
			a = decode_arg(a);
			pc = a - 1;
			break;
		case 0x12:
			if(sp == 0){
				mvprintw(21, 0, "SP 0");
				refresh();
				endwin();
				exit(0);
			}
			pc = stack[--sp] - 1;
			stack[sp] = 0;
			break;
		case 0x13:
			a = memory[++pc];
			a = decode_arg(a);
			mvprintw(output_index_y, output_index_x++, "%c", a);
			if(output_index_x > 73){
				output_index_y++;
				output_index_x = 0;
			}
			break;
		case 0x14:
			//TODO
			mvprintw(0, 0, "Input:\n");
			a = memory[++pc];
			a = decode_arg(a);
			memory[a] = getch();
			mvprintw(0, 2, "%c", memory[a]);
			refresh();
			break;
		case 0x15:
			// NO OPERATION
			break;
		default:
			clear();
			mvprintw(21, 0, "Invalid OP-code: %x\nPC: %x", opcode, pc);
			refresh();
			getch();
			endwin();
			exit(0);
	}
	pc++;
}

u16 decode_arg(u16 arg){
	if(arg >= 0x8000){
		return get_reg(arg);
	}else{
		return arg;
	}
}

void set_reg(u16 reg, u16 value){
	registers[reg - 0x8000] = value;
}

u16 get_reg(u16 reg){
	return registers[reg - 0x8000];
}

void print_memory(u16 index){
	move(0, 0);
	int count = 0;
	for(int i = 0; i < 20; i++){
		for(int j = 0; j < 15; j++){ 
			if((index + count) == pc){
				init_pair(1, COLOR_BLACK, COLOR_RED);
				attrset(COLOR_PAIR(1));
				mvprintw(i, j * 5, "%04x", memory[index + count++]);
				attroff(COLOR_PAIR(1));
			}else{
				mvprintw(i, j * 5, "%04x", memory[index + count++]);
			}
		}
	}
}

void print_registers(){
	for(int i = 0; i < 8; i++){
		mvprintw(3 + i, 100, "R%d: 0x%04x\n", i, registers[i]);
	}
}

void print_stack(){
	for(int i = 0; i < 10; i++){
		mvprintw(12 + i, 100, "[%d]: 0x%04x", i, stack[i]);
	}
}

void print_gui(u8 opcode){
	mvprintw(0, 100, "OP: 0x%02x", opcode);
	mvprintw(1, 100, "PC: 0x%04x", pc);
	mvprintw(2, 100, "SP: 0x%x", sp);
	print_registers();
	print_stack();
	print_memory(print_index);
	refresh();
}
