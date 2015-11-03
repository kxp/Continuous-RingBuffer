#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_chunk.h"


#ifdef __cplusplus
extern "C" {
#endif

    ring_t* //buffern len is the number of pointer we want allocate, the ptr size is the maximum size of each ptr.
        rb_new(unsigned int buffer_len, unsigned int pointer_size)
    {
        if (buffer_len <= 0 /*|| rb== NULL*/)
            return NULL;

        ring_t * rb;
        rb = (ring_t*)malloc(sizeof(ring_t));
        if (!rb)return NULL;

        LOG_PRINT("\nInside of the ring buffer, ptrsize= %d , buffer len= %d", pointer_size, buffer_len);
        unsigned int j = 0;
        //pthread_mutex_init(&(rb->mtx), 0);
        rb->size = buffer_len *pointer_size;

        rb->buffer_start = (uint8_t*)malloc(rb->size * sizeof *rb->buffer_start);
        
        memset(rb->buffer_start, 0, rb->size);
        
        rb->buffer_end =  (rb->buffer_start + (rb->size ) );
        rb->write_calls = 0;
        rb->read_calls = 0;
        rb->head = rb->buffer_start;
        rb->tail =  rb->buffer_start;
               
        rb->round_complete = 0;
        
        return rb;
    }

    uint32_t
    rb_full(ring_t* rb)
    {
        if (rb->head >= rb->tail  && rb->round_complete == 1)//it should be == instead of <=
            return 1;//true
        else
            return 0;//false
    }

    uint32_t
    rb_empty(ring_t* rb)
    {
        if (rb->head <= rb->tail  && rb->round_complete == 0)//it should be == instead of <=
            return 1;//true
        else
            return 0;//false
    }

    uint32_t
    rb_write(ring_t* rb, uint8_t *data, uint32_t size)//avpacket
    {
        LOG_PRINT("\ninside rb write");
        if (rb == NULL || rb->buffer_start == NULL || size <= 0)//just in case
        {
            LOG_PRINT("Pointer to write is null\n");
            return 0;
        }
        if (1 == rb_full(rb))
        {
            LOG_PRINT("Pointer to write is full\n");
            return 0;
        }
        else
        {
            LOG_PRINT("rb_write : %d", size);
            rb->write_calls++;
            int32_t avaiable_space = (int32_t) ( rb->buffer_end - rb->head);
            if ( avaiable_space >= size)
            {
                memcpy(rb->head, data, size);//estoura aki á 21.
                rb->head = (rb->head + size);
                return size;
            }
            else
            {
                uint32_t remaining_size = (size - avaiable_space);
                memcpy(rb->head, data, avaiable_space);
                rb->head = rb->buffer_start;

                //uint32_t* temp = (data + avaiable_space );
                //memcpy(rb->head, temp, remaining_size);
                memcpy(rb->head, (data + remaining_size), avaiable_space);
                rb->head = (rb->head + remaining_size );
                rb->round_complete = 1;
                return avaiable_space;
            }

            return 0;
        }
        return 0;
    }

    //void* //we can change this	
    uint32_t//how it went
        rb_read(ring_t* rb, uint32_t *size, uint8_t* memory)//to avoid local allocations future implementation
    {
        if (rb == NULL || rb->buffer_start == NULL)
        {
            LOG_PRINT("rb_read:Pointer to read is null\n");
            return NULL;
        }
        
        if (1 == rb_empty(rb))
        {
            LOG_PRINT("rb_read:Pointer to read is empty\n");
            *size = 0;
            return NULL;
        }

        //void* memory = malloc(*size);//remove later when we got it rigth
        rb->read_calls++;

        int32_t avaiable_space = (int32_t) (rb->buffer_end - rb->tail) ;
        if (avaiable_space < *size)
        {
            memcpy(memory, (rb->tail), avaiable_space );
            //uint32_t remaining_size = (*size - avaiable_space);
            
            *size = avaiable_space;
            rb->tail = rb->buffer_start;
            //uint32_t* temp = (memory + avaiable_space );
            //memcpy(temp, rb->buffer_start, remaining_size); 

            //rb->tail = (rb->buffer_start + remaining_size );
            rb->round_complete = 0;

            return NULL;
        }
        else
        {
            //memset? nah
            memcpy(memory, (rb->tail), *size);
            rb->tail = rb->tail + *size;
           return NULL;
        }
        return NULL;
    }

    uint32_t
    rb_free_space(ring_t * rb)
    {
        if (rb == NULL)
            return 0;
        if (rb->round_complete == 1)
        {
            uint32_t free_space = (rb->tail - rb->head);//later do a direct return
            return free_space;
        }
        else
        {
            uint32_t free_space = (rb->head - rb->tail);//later do a direct return
            return free_space;
        }
        
    }

    uint32_t
    rb_filled_space(ring_t * rb)
    {
        if (rb == NULL)
            return 0;
        if (rb->round_complete == 1)
        {
            //end part of the buffer               //the initial part of the buffer
            uint32_t filled_space = ( (rb->buffer_end -  rb->tail)  +  ( rb->head - rb->tail) );//later do a direct return
            return filled_space;
        }
        else
        {
            uint32_t filled_space = (rb->head - rb->tail);//later do a direct return
            return filled_space;
        }
    }

    void
     rb_release(ring_t * rb)
    {
        if (rb == NULL)
            return;

        if (rb->buffer_start == NULL)
        {
            free(rb);
            return;
        }
        free(rb->buffer_start);
        free(rb);
    }

#ifdef __cplusplus
}
#endif
