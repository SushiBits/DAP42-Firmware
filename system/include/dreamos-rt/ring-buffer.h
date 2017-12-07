/*
 * ring-buffer.h
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_DREAMOS_RT_RING_BUFFER_H_
#define SYSTEM_INCLUDE_DREAMOS_RT_RING_BUFFER_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/cdefs.h>

struct ring_buffer
{
	char *buffer;
	size_t length;
	off_t head;
	off_t tail;
};
typedef struct ring_buffer *ring_buffer_t;

#define RING_BUFFER_INIT_STATIC(buffer, length) \
	static char __rblib__ ## buffer ## _contents[length]; \
	static struct ring_buffer __rblib__ ## buffer ## _object = \
	{ \
		__rblib__ ## buffer ## _contents, \
		length, \
		0, \
		0 \
	}; \
	ring_buffer_t buffer = &__rblib__ ## buffer ## _object

__BEGIN_DECLS

ring_buffer_t ring_buffer_init(size_t length);
void ring_buffer_dealloc(ring_buffer_t buffer);
int ring_buffer_putchar(ring_buffer_t buffer, char ch);
int ring_buffer_getchar(ring_buffer_t buffer);
int ring_buffer_peekchar(ring_buffer_t buffer);
int ring_buffer_drain(ring_buffer_t buffer);
int ring_buffer_getlength(ring_buffer_t buffer);
int ring_buffer_getalloc(ring_buffer_t buffer);
int ring_buffer_getspace(ring_buffer_t buffer);

__END_DECLS

#endif /* SYSTEM_INCLUDE_DREAMOS_RT_RING_BUFFER_H_ */
