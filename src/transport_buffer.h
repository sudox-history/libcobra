#ifndef COBRA_TRANSPORT_BUFFER_H
#define COBRA_TRANSPORT_BUFFER_H

typedef struct transport_buffer_t transport_buffer_t;

struct transport_buffer_t {
    unsigned char *data;
#ifdef COBRA_TRANSPORT_BUFFER_PRIVATE
    int position;
    int length;
#endif
};

transport_buffer_t* transport_buffer_create(int initial_size);

void transport_buffer_insert(transport_buffer_t *buffer, void *data, int len);

void transport_buffer_rewind(transport_buffer_t *buffer);

void transport_buffer_destroy(transport_buffer_t *buffer);

int transport_buffer_read_int(transport_buffer_t *buffer, int count);

int transport_buffer_position(transport_buffer_t *buffer);

#endif //COBRA_TRANSPORT_BUFFER_H