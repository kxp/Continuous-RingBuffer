#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ring_buffer.h"
#define LOG_PRINT printf

#ifdef __cplusplus
extern "C" {
#endif

//buffern len is the number of pointers we want allocate, the ptr size is the maximum size of each ptr.
ring_t*
rb_new(uint32_t buffer_len)
{   
    ring_t * rb;
    rb = (ring_t*)malloc(sizeof(ring_t));
    if (!rb)
        return NULL;

    LOG_PRINT("\nRing buffer is being created with the size %d.", buffer_len);

    rb->size = buffer_len;

    rb->buffer_start = (uint8_t*)malloc(rb->size * sizeof *rb->buffer_start);
        memset(rb->buffer_start, 0, rb->size);
    
    rb->buffer_end =  (rb->buffer_start + (rb->size ) );
    rb->write_calls = 0;
    rb->read_calls = 0;
    rb->head = rb->buffer_start;
    rb->tail =  rb->buffer_end;
           
    rb->round_complete = 0;
    
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
        else if (avaiable_space >= size)
        {
            uint32_t remaining_size = (rb->buffer_end - rb->head);
            memcpy(rb->head, data, remaining_size);
            rb->head = rb->buffer_start;
            uint32_t missing_size = size - remaining_size;
            if (missing_size <= 0)
                return 0;

            memcpy(rb->head, (data + remaining_size), missing_size);
            rb->head = (rb->head + missing_size);
            rb->round_complete = 1;
            return size;
        }
        else
            LOG_PRINT("rb_write:No more available memory! You must abort!\n");
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

    uint32_t available_space = rb_free_space(rb);

    //Triple check!
    if ((rb->tail + *size) <= rb->head)
    {
        memcpy(memory, (rb->tail), *size);
        rb->tail = rb->tail + *size;
        if (rb->tail == rb->buffer_end)
        {
            rb->tail = rb->buffer_start;
            rb->round_complete = 0;
        }
        return NULL;
    }
    else if (available_space < rb->size)
    {
        int32_t remaining_space = rb->buffer_end - rb->tail;
       
        if (remaining_space >= *size)//this shouldnt happen!this if should get caugth in the previous one
        {
            memcpy(memory, (rb->tail), *size);
            rb->tail += *size;
            if (rb->tail == rb->buffer_end)
            {
                rb->tail = rb->buffer_start;
                rb->round_complete = 0;
            }
            return 0;
        }
        else
        {
            memcpy(memory, (rb->tail), remaining_space);
            rb->tail += remaining_space;
            //
            if (rb->tail == rb->buffer_end)
                rb->tail = rb->buffer_start;
            else if (rb->tail > rb->buffer_end)
                LOG_PRINT("rb_read:Ups, this is bad!\n");

            //get the missing asked size
            int32_t missing_size = *size - remaining_space;
            //get the ptr
            uint8_t* temp = (memory + remaining_space);
            memcpy(temp, rb->tail, missing_size);//cpy the lefted ptr
            rb->tail += missing_size;
            rb->round_complete = 0;
            return size;
        }
    }
    else
    {
        LOG_PRINT("rb_read:Nothing to Read from the buffer!\n");
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
    {
        int32_t free_space = (int32_t)(rb->tail - rb->head);//later do a direct return
        return free_space;
    }
    else
    {
        int32_t free_space = (int32_t) (rb->head - rb->tail);//later do a direct return
        if (free_space == 0)
        {
            free_space = rb->size;
        }
        return free_space;
    }
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

#ifdef __cplusplus
}
#endif

    
    
