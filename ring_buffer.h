#include <stdint.h>

typedef struct ring_t           //some padding should be taking into consideration.
{
    uint8_t *buffer_start;      //ptr to the start address
    uint8_t *buffer_end;        //ptr to the end mem address allocated 
    uint8_t *head;              //ptr to the head memory address
    uint8_t *tail;              //ptr to the tail memory address
    uint32_t size;              //Total buffer size
    uint32_t read_calls;        //for debugging
    uint32_t write_calls;       //for debugging
    uint32_t available_size;    //numero de dados por ler
    char round_complete;        //0- false; 1- true
}ring_t;

ring_t* rb_new(uint32_t buffer_len);

uint32_t rb_full(ring_t* rb);

uint32_t rb_empty(ring_t* rb);

uint32_t rb_write(ring_t* rb, uint8_t *data, uint32_t size);

uint32_t rb_read(ring_t* rb, uint32_t *size, uint8_t* memory);

uint32_t rb_free_space(ring_t * rb);

//uint32_t rb_filled_space(ring_t * rb);

void rb_release(ring_t * rb);



