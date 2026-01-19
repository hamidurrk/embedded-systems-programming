/*
 * bit_ops.h
 *
 * Created: 19.1.2026 11:31:00
 *  Author: x144622
 */ 


#ifndef BIT_OPS_H_
#define BIT_OPS_H_

# define SET_BIT(val, bit)		(val |= ((( __typeof__ ( val ))1) << bit ))# define CLEAR_BIT(val, bit)	(val &= ~((( __typeof__ ( val))1) << bit ) )
# define READ_BIT(val, bit)	((val & ((( __typeof__ ( val))1) << bit ) ) ? 1 : 0)

#endif /* BIT_OPS_H_ */