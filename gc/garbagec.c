// Feito para a disciplina DCC065 - Sistemas Operacionais (UFMG)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "garbagec.h"


// Strategia de alocação como variável global. Feio mas funciona dado o
// esqueleto do TP.
static char *STRATEGY = NULL;

// Caso você utilize a lista fora da memória que vamos alocar a mesma
// terá que ser acessada como uma var global. Global vars in C.
free_list_t *HEAP = NULL;

/*
 * Sempre aloca até estourar o limite, não implementei libera
 */
void *alwaysGrow(size_t size) {
  assert(HEAP->lastAlloca != NULL);
  free_node_t *lastAlloca = HEAP->lastAlloca;
  //printf("Ultimo free %lu\n", lastAlloca->free);
   
  // Temos espaço para alocar + o espaço da lista?
  if (lastAlloca->free < sizeof(free_node_t) + size) {
    return NULL;
  }
  // Sim!
  // Novo nó logo após o último aloca.
  // Posicao da alocacao + tamanho alocado + tamanho do cabeçalho
  free_node_t *newNode = (void*)lastAlloca + lastAlloca->size + sizeof(free_node_t);
  
  newNode->next = NULL;
  newNode->size = size;
  newNode->free = lastAlloca->free - sizeof(free_node_t) - size;

  // Só crescemos, o espaço anterior não tem memória livre mais.
  lastAlloca->free = 0;
  lastAlloca->next = newNode;
  HEAP->lastAlloca = newNode; // Atualiza ponteiro para última operação

  // Retornamos o inicio do espaço que alocamos tirando o cabeçalho
  return (void*)newNode + sizeof(free_node_t);
}

void *ff(size_t size) {
	free_node_t *aux = HEAP->head;

	while(aux != NULL) {
		if(aux->free >= sizeof(free_node_t) + size)
        {

			free_node_t *newNode = (void*)aux + size + sizeof(free_node_t);
			newNode->free = aux->free - sizeof(free_node_t) - size;
			newNode->size = size;
			newNode->next = aux->next;
			aux->next = newNode;
			
			aux->free = 0;
		
			if(newNode -> next == NULL)
				HEAP -> lastAlloca = newNode;
				
			return (void*)newNode + sizeof(free_node_t);
		}else{
			if(aux -> next == NULL)
				HEAP -> lastAlloca = aux;
			
			aux = aux->next;
		}

	}
	return NULL;
}

void *bf(size_t size) {
	free_node_t *aux = HEAP -> head;
	free_node_t *fim = HEAP -> lastAlloca;
	free_node_t *best = HEAP -> head;
	int primeiro = 0;
	size_t dif = size;
	size_t melhor;

	while(aux != NULL) {

		if(aux->free >= sizeof(free_node_t) + size)
        {  
            if(primeiro == 0){
                melhor = aux -> free - sizeof(free_node_t) - size;
                best = aux;
                primeiro = 1;
            }else{
                dif = aux -> free - sizeof(free_node_t) - size;
                if(dif < melhor){
                    melhor = dif;
                    best = aux;
                }
            }            
         			
		}
		aux = aux->next;
	}
	          
    free_node_t *newNode = (void*)best + size + sizeof(free_node_t);
    newNode->free = best->free - sizeof(free_node_t) - size;
    newNode->size = size;
    newNode->next = best->next;
    best->next = newNode;
			
    best->free = 0;
	return (void*)newNode + sizeof(free_node_t);

}

void *wf(size_t size) {
	free_node_t *aux = HEAP -> head;
	free_node_t *fim = HEAP -> lastAlloca;
	free_node_t *best = HEAP -> head;
	int primeiro = 0;
	size_t dif = size;
	size_t melhor;

	while(aux != NULL) {

		if(aux->free >= sizeof(free_node_t) + size)
        {  
            if(primeiro == 0){
                melhor = aux -> free - sizeof(free_node_t) - size;
                best = aux;
                primeiro = 1;
            }else{
                dif = aux -> free - sizeof(free_node_t) - size;
                if(dif > melhor){
                    melhor = dif;
                    best = aux;
                }
            }            
         			
		}
		aux = aux->next;
	}
	          
    free_node_t *newNode = (void*)best + size + sizeof(free_node_t);
    newNode->free = best->free - sizeof(free_node_t) - size;
    newNode->size = size;
    newNode->next = best->next;
    best->next = newNode;
	
    best->free = 0;
	return (void*)newNode + sizeof(free_node_t);
}

void *nf(size_t size) {

free_node_t *aux = HEAP->lastAlloca;
        free_node_t *newNode;
 
        while(aux != NULL) {
                if(aux->free >= sizeof(free_node_t) + size)
                {
 
                        newNode = (void*)aux + size + sizeof(free_node_t);
                        newNode->free = aux->free - sizeof(free_node_t) - size;
                        newNode->size = size;
                        newNode->next = aux->next;
                        aux->next = newNode;
 
                        aux->free = 0;
 
                        printf("mem: %lu\n", newNode->free);
 
                        if(newNode->next == NULL)
                                HEAP->lastAlloca = newNode;
 
                        return (void*)newNode + sizeof(free_node_t);
                }else{
                        if(aux->next == NULL)
                                HEAP->lastAlloca = aux;
 
                        aux = aux->next;
                }
 
        }
        return (void*)newNode + sizeof(free_node_t);
}


void *aloca(size_t size) {

  if (strcmp(STRATEGY, "ag") == 0) {
    return alwaysGrow(size);
  }

  if (strcmp(STRATEGY, "ff") == 0) {
    return ff(size);
  }

  if (strcmp(STRATEGY, "bf") == 0) {
    return bf(size);
  }

  if (strcmp(STRATEGY, "wf") == 0) {
    return wf(size);
  }

  if (strcmp(STRATEGY, "nf") == 0) {
    return nf(size);
  }

  return NULL;
}

void libera(void *ptr) {
	free_node_t *anterior = HEAP -> head;

	free_node_t *metaData = (void*)ptr - sizeof(free_node_t);
	
	while(anterior -> next != metaData){
		anterior = anterior -> next;
		//printf("To no while!\n");
	}
	if(HEAP -> lastAlloca == metaData){
		HEAP -> lastAlloca = anterior;
	}
	
	anterior -> free += metaData -> free + metaData -> size;
	anterior -> next = metaData -> next;
}

void referencia(size_t pos){
        int i = 0;
        free_node_t *aux = HEAP->head;
        while(aux != NULL) {
                if(i = pos)
                        aux->size += 4;
                i++;
                aux = aux->next;
        }
}

void desReferencia(size_t pos){
        int i = 0;
        free_node_t *aux = HEAP->head;
        while(aux != NULL) {
                if(i = pos)
                        aux->size -= 4;
                i++;
                aux = aux->next;
        }
}

void run(void **variables, int* referencias, void ** variables2) {
  // Vamos iniciar alocando todo o MEMSIZE. Vamos split e merges depois.
  // Vou iniciar o HEAP usando NULL, deixa o SO decidir. Podemos usar sbrk(0)
  // também para sugerir o local inicial.
  HEAP = mmap(NULL, MEMSIZE,
              PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
              -1, 0);
  assert(HEAP != NULL);
  assert(HEAP != MAP_FAILED);

  HEAP->head = (void*) HEAP + sizeof(free_list_t);
  HEAP->lastAlloca = HEAP->head;
  HEAP->head->size = 0;
  HEAP->head->free = MEMSIZE - sizeof(free_list_t) - sizeof(free_node_t);
  HEAP->head->next = NULL;

  int opid;    // ID da operação
  int memsize; // Tamanho da alocação
  char optype; // Tipo da operação
  void *addr;
  while (scanf("%d", &opid) == 1) {
    getchar();
    scanf("%d", &memsize);
    getchar();
    scanf("%c", &optype);
    getchar();
    //printf("Alocando %d; %d; %c\n", opid, memsize, optype);
    if (optype == 'a') {         // Aloca!
      addr = aloca(memsize);
      if (addr == NULL) {
        //printf("mem full\n");
        munmap(HEAP, MEMSIZE);
        exit(1);
      }
      variables[opid] = addr;
      referencias[opid]++;
    }else if (optype == 'r') {  // Referencia!
      referencia(memsize);      
      //addr = aloca(4);
      if (addr == NULL) {
        //printf("mem full\n");
        munmap(HEAP, MEMSIZE);
        exit(1);
      }
      variables[opid] =  variables[memsize];
      //variables2[opid] =  addr;
      referencias[memsize]++;
    }else if (optype == 'f') {  // Free!
      if (referencias[opid] == 0) {        
         //printf("referencias [%d] - %d \n",  referencias[opid]);
        addr = variables[opid];        
        libera(addr);
        desReferencia(opid);
      //  if (variables2[opid] != NULL) {
        //  addr =  variables2[opid];
          //libera(addr);
        //}
    }
      else{
        referencias[opid]--;
        // printf("referencias [%d] - %d \n",  referencias[opid]);
      }    
    } else {
      printf("Erro na entrada");
      munmap(HEAP, MEMSIZE);
      exit(1);
    }
  }
  
  
  //soma
  
  int soma_total = 0;
        free_node_t *aux = HEAP->head;
        while(aux != NULL) {
                soma_total += aux->size;
                aux = aux->next;
        }
  
  printf("%d\n", soma_total);
  
  munmap(HEAP, MEMSIZE);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    //printf("Usage %s <algorithm>\n", argv[0]);
    exit(1);
  }
  STRATEGY = argv[1];

  int nops;
  scanf("%d\n", &nops);
  //printf("%d\n", nops);

  char *algorithms[] = {"ff", "bf", "wf", "nf", "ag"};
  int n_alg = 5;
  int valid = 0;
  for (int i = 0; i < n_alg; i++) {
    if (strcmp(STRATEGY, algorithms[i]) == 0) {
      valid = 1;
      break;
    }
  }
  if (valid == 0) {
    printf("Algoritmo inválido: Usage %s <algorithm>\n", argv[0]);
    printf("--onde o algoritmo vem das opções: {ff, bf, wf, nf, ag}\n");
    exit(1);
  }

  // O vetor variables mantem os endereços de ids alocados.
  // É lido ao executarmos uma operação 'f'
  void **variables = (void **) malloc(nops * sizeof(void**));
  assert(variables != NULL);
  
   void **variables2 = (void **) malloc(nops * sizeof(void**));
  assert(variables2 != NULL);

  for (int i = 0; i < nops; i++)
    variables[i] = NULL;
  
    for (int i = 0; i < nops; i++)
    variables2[i] = NULL;
  
  int *referencias = (int *) malloc(nops * sizeof(int*));
  assert(referencias != NULL);
  
  for (int i = 0; i < nops; i++)
    referencias[i] = 0;
  
  run(variables, referencias, variables2);
  //free(variables, referencias, variables2);
}
