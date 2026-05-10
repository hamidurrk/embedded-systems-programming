/*
 * dotmatrix.h
 *
 * Created: 4/12/2026 3:32:22 AM
 *  Author: Hamidur
 */ 


#ifndef DOTMATRIX_H_
#define DOTMATRIX_H_

void dotmatrix_init(void);
void dotmatrix_clear(void);

void dotmatrix_show_up_arrow(void);
void dotmatrix_show_down_arrow(void);
void dotmatrix_show_red_circle(void);
void dotmatrix_show_checker_pattern(void);
void dotmatrix_show_full_square_pattern(void);

void dotmatrix_print_message(const char *message);




#endif /* DOTMATRIX_H_ */