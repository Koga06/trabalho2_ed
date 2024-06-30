#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

typedef struct {
	char codigo_ibge[8];
	char nome[100];
	double latitude;
	double longitude;
	int capital;
	int codigo_uf;
	int siafi_id;
	int ddd;
	char fuso[100];	
} Municipio;

// Estrutura criada para representar os itens da lista encadeada
typedef struct registro{
	Municipio item;
	struct registro *prox; // Aponta para o próximo registro da lista
} registro;

typedef struct tnode{
    char chave[100]; // Chave para os campos específicos (nome, latitude, longitude, codigo_uf ou ddd)
    struct tnode *esq;
    struct tnode *dir;
    struct tnode *pai; // Adicionado o ponteiro para pai
    int h;
    registro *lista_reg; // Lista encadeada para registros com chaves iguais
} tnode;

int max(int a,int b){
    return a>b?a:b;
}

int altura(tnode *arv){
    int ret;
    if (arv==NULL){
        ret = -1;
    }else{
        ret = arv->h;
    }
    return ret;
}

void _rd(tnode **parv){
    tnode *y = *parv; 
    tnode *x = y->esq;
    tnode *A = x->esq;
    tnode *B = x->dir;
    tnode *C = y->dir;

    y->esq = B; 
    x->dir = y;
    *parv  = x;
    y->h = max(altura(B),altura(C)) + 1;
    x->h = max(altura(A),altura(y)) + 1;

    // Novo ponteiro: Pai
    x->pai = y->pai;
    y->pai = x;
    if (B != NULL) {
    	B->pai = y;
    }
}

void _re(tnode **parv){
    tnode * x = *parv; 
    tnode * y = x->dir;
    tnode * A = x->esq;
    tnode * B = y->esq;
    tnode * C = y->dir;

    x->dir = B;
    y->esq = x; 
    *parv  = y;
    x->h = max(altura(A),altura(B)) + 1;
    y->h = max(altura(x),altura(C)) + 1;

    // Novo ponteiro: Pai
    y->pai = x->pai;
    x->pai = y;
    if (B != NULL) {
    	B->pai = x;
    }
}

void _avl_rebalancear(tnode **parv){
    int fb;
    int fbf;
    tnode * filho;
    fb = altura((*parv)->esq) - altura((*parv)->dir);

    if (fb  == -2){
        filho = (*parv)->dir;
        fbf = altura(filho->esq) - altura(filho->dir);
        if (fbf <= 0){ /* Caso 1  --> ->*/
            _re(parv);
        }else{   /* Caso 2  --> <-*/
            _rd(&(*parv)->dir);
            _re(parv);
        }
    }else if (fb == 2){  
        filho = (*parv)->esq;
        fbf = altura(filho->esq) - altura(filho->dir);
        if (fbf >=0){ /* Caso 3  <-- <-*/
            _rd(parv);
        }else{  /* Caso 4  <-- ->*/
            _re(&(*parv)->esq);
            _rd(parv);
        }
    }
}

void avl_insere(tnode ** parv, Municipio item, char chave_busca[]){
    if (*parv == NULL){
        *parv = (tnode *) malloc(sizeof(tnode));
        strcpy((*parv)->chave, chave_busca);
        (*parv)->esq = NULL;
        (*parv)->dir = NULL;
        (*parv)->pai = NULL; // Inicializamos o pai como nulo
        (*parv)->h = 0;
        
        // Inicializa a lista de registros
        (*parv)->lista_reg = (registro *)malloc(sizeof(registro));
        (*parv)->lista_reg->item = item;
        (*parv)->lista_reg->prox = NULL;
    } else {
    	// Vamos definir qual vai ser o item entre as 5 opções
    	int cmp = 0;
    	if (strcmp(chave_busca, "nome") == 0) {
    		cmp = strcmp((*parv)->lista_reg->item.nome, item.nome);
    	} else if (strcmp(chave_busca, "latitude") == 0) {
			if ((*parv)->lista_reg->item.latitude > item.latitude) {
				cmp = 1;
			} else if ((*parv)->lista_reg->item.latitude < item.latitude) {
				cmp = -1;
			}
    	} else if (strcmp(chave_busca, "longitude") == 0) {
    		if ((*parv)->lista_reg->item.longitude > item.longitude) {
				cmp = 1;
			} else if ((*parv)->lista_reg->item.longitude < item.longitude) {
				cmp = -1;
			}
    	} else if (strcmp(chave_busca, "codigo_uf") == 0) {
    		cmp = ((*parv)->lista_reg->item.codigo_uf > item.codigo_uf) - ((*parv)->lista_reg->item.codigo_uf < item.codigo_uf);
    	} else if (strcmp(chave_busca, "ddd") == 0) {
			cmp = ((*parv)->lista_reg->item.ddd > item.ddd) - ((*parv)->lista_reg->item.ddd < item.ddd);
    	}
    
    	if (cmp > 0) {
    		avl_insere(&(*parv)->esq, item, chave_busca);
        	(*parv)->esq->pai = *parv; // Define o pai do filho a esquerda
    	} else if (cmp < 0){
    		avl_insere(&(*parv)->dir, item, chave_busca);
        	(*parv)->dir->pai = *parv; // Define o pai do filho a direita
    	} else { // se a chave já existe/ forem iguais, inserir na lista de registros
    		registro *reg = (registro *)malloc(sizeof(registro));
    		reg->item = item;
    		reg->prox = (*parv)->lista_reg;
    		(*parv)->lista_reg = reg;
    		return; // não há necessidade de rebalanceamento se somente adicionamos na lista
    	}
    }

    (*parv)->h = max(altura((*parv)->esq),altura((*parv)->dir)) + 1;
    _avl_rebalancear(parv);
}

void leitura_insercao(tnode **parv_nome, tnode **parv_latitude, tnode **parv_longitude, tnode **parv_codigo_uf, tnode **parv_ddd){
	FILE *arquivo = fopen("municipios.txt", "r");
	Municipio muni;
	char linha[1024];

	if (arquivo == NULL) {
		printf("Nao foi possível ler o arquivo.\n");
		return;
	} else {
		while(fgets(linha, sizeof(linha), arquivo)){
			sscanf(linha, "%[^,],%[^,],%lf,%lf,%d,%d,%d,%d,%[^\n]", 
				muni.codigo_ibge, muni.nome, &muni.latitude, &muni.longitude, &muni.capital, &muni.codigo_uf, &muni.siafi_id, &muni.ddd, muni.fuso);
			
			avl_insere(parv_nome, muni, "nome");
			avl_insere(parv_latitude, muni, "latitude");
			avl_insere(parv_longitude, muni, "longitude");
			avl_insere(parv_codigo_uf, muni, "codigo_uf");
			avl_insere(parv_ddd, muni, "ddd");
		}
	}

	fclose(arquivo);
}

void imprime_arvore(tnode *parv) {
    if (parv != NULL) {
        imprime_arvore(parv->esq);

        // Registros da lista encadeada, caso existam
        registro *reg = parv->lista_reg;
        while (reg != NULL){
        	printf("Codigo IBGE: %s, Nome: %s\n", reg->item.codigo_ibge, reg->item.nome);
            reg = reg->prox;
        }

        imprime_arvore(parv->dir);
    }
}

// Função para encontrar o sucessor de um nó na árvore AVL, considerando o pai
tnode *sucessor(tnode *node){
	if (node == NULL) {
		return NULL;
	}

	// CASO 1 - Se o nó tem filho a direita, o sucessor é o menor nó dessa subárvore a direita
	if (node->dir != NULL) {
		tnode *atual = node->dir;
		while (atual->esq !=NULL){
			atual = atual->esq;
		}
		return atual;
	}

	// CASO 2 - Se o nó não tem filho a direita, subimos a árvore até encontrar um pai 
	// onde o filho a esquerda seja o proprio nó
	tnode *pai = node->pai;
	while (pai != NULL && node == pai->dir){
		node = pai;
		pai = pai->pai;
	}
	return pai;
}

void busca_latitude(tnode **parv, double latitude1, double latitude2, int *resultado, int *cont, int modo){
    if (*parv == NULL) {
        return;
    }

    switch (modo) {
        case 1: // latitude > ?
            if ((*parv)->lista_reg->item.latitude > latitude1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_latitude(&(*parv)->esq, latitude1, latitude2, resultado, cont, modo);
                busca_latitude(&(*parv)->dir, latitude1, latitude2, resultado, cont, modo);
            } else {
                busca_latitude(&(*parv)->dir, latitude1, latitude2, resultado, cont, modo);
            }
            break;
        case 2: // latitude < ?
            if ((*parv)->lista_reg->item.latitude < latitude1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_latitude(&(*parv)->esq, latitude1, latitude2, resultado, cont, modo);
                busca_latitude(&(*parv)->dir, latitude1, latitude2, resultado, cont, modo);
            } else {
                busca_latitude(&(*parv)->dir, latitude1, latitude2, resultado, cont, modo);
            }
            break;
        case 3: // ? < latitude < ?
            if ((*parv)->lista_reg->item.latitude > latitude1 && (*parv)->lista_reg->item.latitude < latitude2) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
            } 

            busca_latitude(&(*parv)->esq, latitude1, latitude2, resultado, cont, modo);
            busca_latitude(&(*parv)->dir, latitude1, latitude2, resultado, cont, modo);
            break;
    }
}

void busca_longitude(tnode **parv, double longitude1, double longitude2, int *resultado, int *cont, int modo){
    if (*parv == NULL) {
        return;
    }

    switch (modo) {
        case 1: // longitude > ?
            if ((*parv)->lista_reg->item.longitude > longitude1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_longitude(&(*parv)->esq, longitude1, longitude2, resultado, cont, modo);
                busca_longitude(&(*parv)->dir, longitude1, longitude2, resultado, cont, modo);
            } else {
                busca_longitude(&(*parv)->dir, longitude1, longitude2, resultado, cont, modo);
            }
            break;
        case 2: // longitude < ?
            if ((*parv)->lista_reg->item.longitude < longitude1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_longitude(&(*parv)->esq, longitude1, longitude2, resultado, cont, modo);
                busca_longitude(&(*parv)->dir, longitude1, longitude2, resultado, cont, modo);
            } else {
                busca_longitude(&(*parv)->dir, longitude1, longitude2, resultado, cont, modo);
            }
            break;
        case 3: // ? < longitude < ?
            if ((*parv)->lista_reg->item.longitude > longitude1 && (*parv)->lista_reg->item.longitude < longitude2) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
            }
            busca_longitude(&(*parv)->esq, longitude1, longitude2, resultado, cont, modo);
            busca_longitude(&(*parv)->dir, longitude1, longitude2, resultado, cont, modo);
            
            break;
    }
}

void busca_ddd(tnode **parv, double ddd1, double ddd2, int *resultado, int *cont, int modo){
    if (*parv == NULL) {
        return;
    }

    switch (modo) {
        case 1: // ddd > ?
            if ((*parv)->lista_reg->item.ddd > ddd1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_ddd(&(*parv)->esq, ddd1, ddd2, resultado, cont, modo);
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            } else {
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            }
            break;
        case 2: // ddd < ?
            if ((*parv)->lista_reg->item.ddd < ddd1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_ddd(&(*parv)->esq, ddd1, ddd2, resultado, cont, modo);
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            } else {
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            }
            break;
        case 3: // ? < ddd < ?
            if ((*parv)->lista_reg->item.ddd > ddd1 && (*parv)->lista_reg->item.ddd < ddd2) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
                
                busca_ddd(&(*parv)->esq, ddd1, ddd2, resultado, cont, modo);
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            } else {
                busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            }
            break;
        case 4: // ddd = ?
            if ((*parv)->lista_reg->item.ddd == ddd1) {
                registro *reg = (*parv)->lista_reg;
                while (reg != NULL){
                    resultado[(*cont)++] = atoi(reg->item.codigo_ibge);
                    reg = reg->prox;
                }
            }

            busca_ddd(&(*parv)->esq, ddd1, ddd2, resultado, cont, modo);
            busca_ddd(&(*parv)->dir, ddd1, ddd2, resultado, cont, modo);
            break;
    }
}

int menu_modo(){
    int modo = 0;

    printf("\nInforme o modo desejado.\n");
    printf("1. parametro > x\n");
    printf("2. parametro < x\n");
    printf("3. x < parametro < y\n");
    printf("4. parametro == x\n");
    printf("Modo: ");
    scanf("%d", &modo);

    return modo;
}

int *intersecao(int *result1, int cont1, int *result2, int cont2, int *result3, int cont3, int *cont_inter) {
    int *resultado_inter = (int *)malloc(6000 * sizeof(int));
    int contador = 0;

    for (int i = 0; i < cont1; i++) {
        for (int j = 0; j < cont2; j++) {
            if (result1[i] == result2[j]) {
                for (int k = 0; k < cont3; k++) {
                    if (result1[i] == result3[k]) {
                        resultado_inter[contador++] = result1[i];
                        break;
                    }
                }
                break;
            }
        }
    }

    *cont_inter = contador;
    return resultado_inter;
}

tnode *no_do_codigo(tnode *parv, int codigo_ibge){
    if (parv == NULL) {
        return NULL;
    }

    registro *reg = parv->lista_reg;
    while (reg != NULL) {
        if (atoi(reg->item.codigo_ibge) == codigo_ibge) {
            return parv;
        }
        reg = reg->prox;
    }

    tnode *r = no_do_codigo(parv->esq, codigo_ibge);
    if (r != NULL) {
        return r;
    }

    return no_do_codigo(parv->dir, codigo_ibge);
}

void imprime_no(tnode *node){
    if (node == NULL) {
        return;
    }

    registro *reg = node->lista_reg;
    while (reg != NULL) {
        printf("Codigo IBGE: %s\n", reg->item.codigo_ibge);
        printf("Nome: %s\n", reg->item.nome);
        printf("Latitude: %.2f\n", reg->item.latitude);
        printf("Longitude: %.2f\n", reg->item.longitude);
        printf("Capital: %d\n", reg->item.capital);
        printf("Codigo UF: %d\n", reg->item.codigo_uf);
        printf("Siafi ID: %d\n", reg->item.siafi_id);
        printf("DDD: %d\n", reg->item.ddd);
        printf("Fuso: %s\n", reg->item.fuso);
        reg = reg->prox;
    }
}


int main(int argc, char const *argv[]) {
	tnode *parv_nome = NULL;
	tnode *parv_latitude = NULL;
	tnode *parv_longitude = NULL;
	tnode *parv_codigo_uf = NULL;
	tnode *parv_ddd = NULL;

	leitura_insercao(&parv_nome, &parv_latitude, &parv_longitude, &parv_codigo_uf, &parv_ddd);

    int *result_lat = (int *)malloc(6000 * sizeof(int));
    int *result_lon = (int *)malloc(6000 * sizeof(int));
    int *result_ddd = (int *)malloc(6000 * sizeof(int));

    int cont_lat = 0, cont_lon = 0, cont_ddd = 0;
    int modo;
    double range1, range2;

    printf("Informe uma range querie para LATITUDE.\n");
    modo = menu_modo();
    if (modo == 1 || modo == 2 || modo == 4) {
        printf("\nInforme o valor:\n");
        printf("X = ");
        scanf("%lf", &range1);
    } else {
        printf("\nInforme os valores do intervalo:\n");
        printf("X = ");
        scanf("%lf", &range1);
        printf("Y = ");
        scanf("%lf", &range2);
    }
    busca_latitude(&parv_latitude, range1, range2, result_lat, &cont_lat, modo);
    system("cls");

    printf("Informe uma range querie para LONGITUDE.\n");
    modo = menu_modo();
    if (modo == 1 || modo == 2 || modo == 4) {
        printf("\nInforme o valor:\n");
        printf("X = ");
        scanf("%lf", &range1);
    } else {
        printf("\nInforme os valores do intervalo:\n");
        printf("X = ");
        scanf("%lf", &range1);
        printf("Y = ");
        scanf("%lf", &range2);
    }
    busca_longitude(&parv_longitude, range1, range2, result_lon, &cont_lon, modo);
    system("cls");

    printf("Informe uma range querie para DDD.\n");
    modo = menu_modo();
    if (modo == 1 || modo == 2 || modo == 4) {
        printf("\nInforme o valor:\n");
        printf("X = ");
        scanf("%lf", &range1);
    } else {
        printf("\nInforme os valores do intervalo:\n");
        printf("X = ");
        scanf("%lf", &range1);
        printf("Y = ");
        scanf("%lf", &range2);
    }
    busca_ddd(&parv_ddd, range1, range2, result_ddd, &cont_ddd, modo);
    system("cls");

    int cont_inter = 0;
    int *resultado_inter = intersecao(result_lat, cont_lat, result_lon, cont_lon, result_ddd, cont_ddd, &cont_inter);

    // Impressão
    printf("Cidades na intersecao das queries:\n\n");
    if (cont_inter == 0) {
        printf("Nao ha cidades nessa intersecao.\n");
    } else {
        for (int i = 0; i < cont_inter; i++) {
            // Busca o no do codigo ibge do resultado e imprime as informacoes
            tnode *node = no_do_codigo(parv_nome, resultado_inter[i]);
            if (node != NULL) {
                imprime_no(node);
                printf("\n");
            } else {
                printf("Cidade do codigo IBGE %d nao encontrada.\n", resultado_inter[i]);
            }
        }
    }

    // Liberação de Memória
    free(result_lat);
    free(result_lon);
    free(result_ddd);
    free(resultado_inter);
	return 0;
}