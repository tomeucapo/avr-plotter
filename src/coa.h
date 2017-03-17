#ifndef COA_H
#define COA_H

#define QUEUE_SIZE 16

struct coa {
       unsigned char buffer[QUEUE_SIZE];
       signed char capsalera, darrer;
};

void coa_nova(struct coa *q);
void afegeix_element(struct coa *q, signed char c);
signed char treu_element(struct coa *q);
unsigned char coa_buida(struct coa *q);

#endif
