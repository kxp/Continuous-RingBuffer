#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ring_buffer.h"
#define LOG_PRINT printf



 //buffern len is the number of pointers we want allocate, the ptr size is the maximum size of each ptr.
ring_t*
rb_new(uint32_t buffer_len, uint32_t pointer_size)
{   
    ring_t * rb;
    rb = (ring_t*)malloc(sizeof(ring_t));
    if (!rb)
    	return NULL;

    LOG_PRINT("\nInside of the ring buffer, ptrsize= %d , buffer len= %d", pointer_size, buffer_len);

    rb->size = buffer_len *pointer_size;

    rb->buffer_start = (uint8_t*)malloc(rb->size * sizeof *rb->buffer_start);    
    memset(rb->buffer_start, 0, rb->size);
    
    rb->buffer_end =  (rb->buffer_start + (rb->size ) );
    rb->write_calls = 0;
    rb->read_calls = 0;
    rb->head = rb->buffer_start;
    rb->tail =  rb->buffer_end;
           
    rb->round_complete = 1;
    
    return rb;
}

uint32_t
rb_full(ring_t* rb)
{
    if (rb->head >= rb->tail  && rb->round_complete == 1)
        return 1;//true
    else
        return 0;//false
}

uint32_t
rb_empty(ring_t* rb)
{
    if (rb->head <= rb->tail  && rb->round_complete == 0)
        return 1;//true
    else
        return 0;//false
}

uint32_t
rb_write(ring_t* rb, uint8_t *data, uint32_t size)
{
    if (rb == NULL || rb->buffer_start == NULL || size <= 0)
        return 0;

    if (1 == rb_full(rb))
        return 0;

    else
    {
        rb->write_calls++;//debug       
        int32_t avaiable_space = rb_free_space(rb);
        printf("\nwrite: avaiable_space: %d\n", avaiable_space);
        if ( avaiable_space >= size)
        {
            memcpy(rb->head, data, size);
            rb->head = (rb->head + size);
            if (rb->head == rb->buffer_end)
            {
                rb->head = rb->buffer_start;
                rb->round_complete = 1;
            }
            return size;
        }
        else if (avaiable_space >= 0)
        {
            uint32_t remaining_size = (size - avaiable_space);
            memcpy(rb->head, data, avaiable_space);
            rb->head = rb->buffer_start;

            memcpy(rb->head, (data + avaiable_space), remaining_size);

            rb->head = (rb->head + remaining_size );
            rb->round_complete = 1;
            return size;            
        }
        else
            LOG_PRINT("rb_write:Pointer to read is empty\n");
        return 0;
    }
    return 0;
}

uint32_t
rb_read(ring_t* rb, uint32_t *size, uint8_t* memory)
{
    if (rb == NULL || rb->buffer_start == NULL)
        goto failed_end;   
    if (1 == rb_empty(rb))
    	goto failed_end;

    rb->read_calls++;//if debug?

    int32_t avaiable_space = (int32_t) (rb->buffer_end - rb->tail) ;
	printf("\nread:  avaiable_space: %d",avaiable_space);
    if (avaiable_space < *size && avaiable_space >= 0)
    {
        memcpy(memory, (rb->tail), avaiable_space );
        uint32_t remaining_size = (*size - avaiable_space);       
       
        rb->tail = rb->buffer_start;
       
        uint8_t* temp = (memory + avaiable_space );
        memcpy(temp, rb->buffer_start, remaining_size);

        rb->tail = (rb->buffer_start + remaining_size );
        rb->round_complete = 0;

        return *size;
    }
    else if (avaiable_space > 0)
    {
        memcpy(memory, (rb->tail), *size);
        rb->tail = rb->tail + *size;
        if (rb->tail == rb->buffer_end)
        {
            rb->tail = rb->buffer_start;
            rb->round_complete = 0;
        }
       return *size;
    }    
    
    failed_end:
		*size = 0;

	return 0;
}


uint32_t
rb_free_space(ring_t * rb)
{
    if (rb == NULL)
        return 0;
    if (rb->round_complete == 1)
        return(rb->tail - rb->head) ;
    else
        return (rb->head - rb->tail);
}

uint32_t
rb_filled_space(ring_t * rb)
{
    if (rb == NULL)
        return 0;
    if (rb->round_complete == 1)
        return( (rb->buffer_end -  rb->tail)  +  ( rb->head - rb->buffer_start) );
    
    else
        return(rb->head - rb->tail);
}

void
rb_release(ring_t * rb)
{
    if (rb == NULL)
        return;

    if (rb->buffer_start != NULL)
    	free(rb->buffer_start);
    free(rb);
}
