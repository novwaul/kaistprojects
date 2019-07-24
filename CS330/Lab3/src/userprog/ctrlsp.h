#ifndef USERPROG_CTRLSP_H
#define USERPROG_CTRLSP_H

#define MOV(pointer, type, val) (pointer = (void *) ((type) pointer + (val)))
#define PSH(pointer, type, val) (*((type) pointer) = (val))
#define SET(pointer, type, val) (pointer = (type) val)
#define GET(carrier, type, pointer) (carrier = *(type *) pointer)

#endif /* userprog/ctrlsp.h */
