/*
 * time.h
 *
 *  Created on: Sep 27, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_DREAMOS_RT_TIME_H_
#define SYSTEM_INCLUDE_DREAMOS_RT_TIME_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

void yield(void);
uint32_t millis(void);
uint32_t micros(void);

__END_DECLS

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#endif /* SYSTEM_INCLUDE_DREAMOS_RT_TIME_H_ */
