/*
 * ring-buffer.c
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#include <dreamos-rt/ring-buffer.h>
#include <stdlib.h>
#include <string.h>

ring_buffer_t ring_buffer_init(size_t length)
{
	ring_buffer_t buffer = calloc(sizeof(struct ring_buffer), 1);
	if (!buffer)
		return NULL;

	buffer->buffer = calloc(length, 1);
	if (!buffer)
	{
		free(buffer);
		return NULL;
	}

	buffer->length = length;
	buffer->head = 0;
	buffer->tail = 0;

	return buffer;
}

void ring_buffer_dealloc(ring_buffer_t buffer)
{
	if (!buffer)
		return;

	free(buffer->buffer);
	free(buffer);
}

int ring_buffer_putchar(ring_buffer_t buffer, char ch)
{
	if (!buffer)
		return -1;

	buffer->buffer[buffer->head] = ch;
	buffer->head = (buffer->head + 1) % buffer->length;
	if (buffer->head == buffer->tail)
		buffer->tail = (buffer->tail + 1) % buffer->length;
	return ch;
}

int ring_buffer_getchar(ring_buffer_t buffer)
{
	if (!buffer)
		return -1;

	if (buffer->head == buffer->tail)
		return -1;

	char ch = buffer->buffer[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->length;

	return ch;
}

int ring_buffer_peekchar(ring_buffer_t buffer)
{
	if (!buffer)
		return -1;

	if (buffer->head == buffer->tail)
		return -1;

	char ch = buffer->buffer[buffer->tail];

	return ch;
}

int ring_buffer_read(ring_buffer_t buffer, void *buf, size_t len);
int ring_buffer_peek(ring_buffer_t buffer, void *buf, size_t len);
int ring_buffer_write(ring_buffer_t buffer, const void *buf, size_t len);

int ring_buffer_drain(ring_buffer_t buffer)
{
	memset(buffer->buffer, 0, buffer->length);
	buffer->head = 0;
	buffer->tail = 0;

	return 0;
}

int ring_buffer_getlength(ring_buffer_t buffer)
{
	if (!buffer)
		return -1;

	if (buffer->tail > buffer->head)
		return buffer->head + buffer->length - buffer->tail;
	else
		return buffer->head - buffer->tail;
}

int ring_buffer_getalloc(ring_buffer_t buffer)
{
	if (!buffer)
		return -1;

	return buffer->length;
}

int ring_buffer_getspace(ring_buffer_t buffer)
{
	return buffer->length - ring_buffer_getlength(buffer) - 1;
}
