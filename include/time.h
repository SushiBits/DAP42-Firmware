/*
 * time.h
 *
 *  Created on: Dec 6, 2018
 *      Author: technix
 */

#ifndef INCLUDE_TIME_H_
#define INCLUDE_TIME_H_

#include <stm32f0xx.h>
#include_next <time.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

uint32_t millis(void);

__END_DECLS

#endif /* INCLUDE_TIME_H_ */
