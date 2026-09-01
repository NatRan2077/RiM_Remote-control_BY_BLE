#ifndef __FIFO_H
#define __FIFO_H

//size: 4,8,16,32...128, 256 ...
#define FIFO( name, type, size )\
  struct name{\
    type buf[size];\
    volatile uint32_t tail;\
    volatile uint32_t head;\
  } 

/**
  * @brief	The number of elements in the queue
  */
#define FIFO_COUNT(fifo)     (fifo.head-fifo.tail)

/**
  * @brief FIFO size
  */
#define FIFO_SIZE(fifo)      ( sizeof(fifo.buf)/sizeof(fifo.buf[0]) )

/**
  * @brief
  */
#define FIFO_IS_FULL(fifo)   (FIFO_COUNT(fifo) == FIFO_SIZE(fifo))

/**
  * @brief
  */
#define FIFO_IS_EMPTY(fifo)  (fifo.tail == fifo.head)

/**
  * @brief The amount of free space
  */
#define FIFO_SPACE(fifo)     (FIFO_SIZE(fifo)-FIFO_COUNT(fifo))

/**
  * @brief Insert an element
  */
#define FIFO_PUT(fifo, byte) \
  {\
    fifo.buf[fifo.head & (FIFO_SIZE(fifo)-1)]=byte;\
    fifo.head++;\
	if(FIFO_COUNT(fifo) > FIFO_SIZE(fifo)) fifo.tail++;\
  }

/**
  * @brief Get an element
  */
#define FIFO_GET(fifo) (fifo.buf[fifo.tail & (FIFO_SIZE(fifo)-1)])

/**
  * @brief
  */
#define FIFO_DROP(fifo)   \
  {\
      fifo.tail++; \
  }

/**
  * @brief
  */
#define FIFO_FLUSH(fifo)   \
  {\
    fifo.tail=0;\
    fifo.head=0;\
  } 

#endif //__FIFO_H
