//#include <mutex>
//#include <pthreads.h>
#include <stdint.h>

typedef struct ring_t
{
    uint8_t *buffer_start;// number of thigs that u wanna store
    uint8_t *buffer_end;
    uint8_t *head;//inicio dos dados
    uint8_t *tail;//fim dos datos
    uint32_t size;//tamanho do buffer
    uint32_t read_calls;//for debugging
    uint32_t write_calls;//for debugging
    //mutex's 
    //pthread_mutex_t mtx;//ok   we wont have this in windows,  a byte will have to work, too kmuch overhead for 
    //std::mutex mtx;           // mutex for critical section
    char round_complete; //0- false; 1- true
}ring_t;

extern "C"
{
    //buffern len is the number of pointer we want allocate, the ptr size is the maximum size of each ptr.
    ring_t* rb_new(uint32_t buffer_len, uint32_t pointer_size);

    uint32_t rb_full(ring_t* rb);

    uint32_t rb_empty(ring_t* rb);

    uint32_t rb_write(ring_t* rb, uint8_t *data, uint32_t size);//avpacket

    uint32_t 	rb_read(ring_t* rb, uint32_t *size, uint8_t* memory);

    void	rb_release(ring_t * rb);

}


