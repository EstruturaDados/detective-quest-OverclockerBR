#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- CONSTANTES ---
#define TAMANHO_TABELA_HASH 10
#define MAX_SUSPEITOS 50

// --- I. Estruturas de Dados ---

// 1. Elemento da Lista Encadeada da Tabela Hash (Suspeito e Contagem)
typedef struct SuspeitoNode {
    char nomeSuspeito[50];
    int contagemPistas; // Para contar quantas pistas apontam para ele
    struct SuspeitoNode *proximo;
} SuspeitoNode;

// 2. Tabela Hash (Array de ponteiros para SuspeitoNode)
typedef SuspeitoNode* TabelaHash[TAMANHO_TABELA_HASH];

// 3. Nó da Árvore de Busca (BST) para Pistas
typedef struct PistaNode {
    char pista[100];
    char suspeito[50]; // Nova: O suspeito ligado a esta pista
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// 4. Nó da Árvore Binária para Salas
typedef struct Sala {
    char nome[50];
    char pistaEncontrada[100];
    char suspeitoAssociado[50]; // Nova: Suspeito que o jogador associa a pista desta sala
    int pistaColetada;
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;


// --- II. Funções da Tabela Hash ---

/**
 * @brief Função de Hashing simples: Soma dos valores ASCII dos 3 primeiros caracteres (ignorando case).
 * @param chave A string (nome do suspeito) a ser hasheada.
 * @return O índice na tabela hash.
 */
int funcaoHash(const char* chave) {
    if (chave == NULL || chave[0] == '\0') return 0;
    
    int soma = 0;
    for (int i = 0; i < 3 && chave[i] != '\0'; i++) {
        // Converte para maiúsculo para uniformizar a chave
        soma += toupper(chave[i]); 
    }
    return soma % TAMANHO_TABELA_HASH;
}

/**
 * @brief Insere ou atualiza um suspeito na Tabela Hash.
 * Se o suspeito já existir, incrementa a contagem. Caso contrário, cria um novo nó.
 */
void inserirOuAtualizarNaHash(TabelaHash tabela, const char* suspeito) {
    int indice = funcaoHash(suspeito);
    SuspeitoNode* atual = tabela[indice];
    
    // 1. Tenta encontrar o suspeito na lista encadeada (colisão)
    while (atual != NULL) {
        if (strcmp(atual->nomeSuspeito, suspeito) == 0) {
            atual->contagemPistas++; // Suspeito encontrado, incrementa
            return;
        }
        atual = atual->proximo;
    }

    // 2. Se não encontrou, cria um novo nó (inserção no início da lista)
    SuspeitoNode* novoNode = (SuspeitoNode*)malloc(sizeof(SuspeitoNode));
    if (novoNode == NULL) {
        perror("Erro ao alocar memoria para o no do suspeito");
        exit(EXIT_FAILURE);
    }
    strncpy(novoNode->nomeSuspeito, suspeito, sizeof(novoNode->nomeSuspeito) - 1);
    novoNode->nomeSuspeito[sizeof(novoNode->nomeSuspeito) - 1] = '\0';
    novoNode->contagemPistas = 1; // Primeira pista associada
    
    // Insere no início da lista (encadeamento)
    novoNode->proximo = tabela[indice];
    tabela[indice] = novoNode;
}

/**
 * @brief Exibe todos os suspeitos com suas respectivas contagens.
 */
void exibirSuspeitos(TabelaHash tabela) {
    printf("\n--- Associacoes Suspeito | Total de Pistas ---\n");
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        SuspeitoNode* atual = tabela[i];
        while (atual != NULL) {
            printf("  > %s: %d pistas\n", atual->nomeSuspeito, atual->contagemPistas);
            atual = atual->proximo;
        }
    }
    printf("------------------------------------------------\n");
}

/**
 * @brief Identifica e exibe o suspeito com o maior número de pistas associadas.
 */
void encontrarSuspeitoMaisCitado(TabelaHash tabela) {
    char culpado[50] = "Nenhum";
    int maxPistas = 0;
    int houveSuspeito = 0;

    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        SuspeitoNode* atual = tabela[i];
        while (atual != NULL) {
            houveSuspeito = 1;
            if (atual->contagemPistas > maxPistas) {
                maxPistas = atual->contagemPistas;
                strncpy(culpado, atual->nomeSuspeito, sizeof(culpado) - 1);
                culpado[sizeof(culpado) - 1] = '\0';
            }
            atual = atual->proximo;
        }
    }
    
    printf("\n--- DEDUZINDO O CULPADO ---\n");
    if (!houveSuspeito) {
        printf("Nenhuma associacao de pista a suspeito foi registrada.\n");
    } else {
        printf("A maior evidencia aponta para: **%s** com %d pistas relacionadas.\n", culpado, maxPistas);
    }
    printf("---------------------------\n");
}

/**
 * @brief Libera a memória da Tabela Hash.
 */
void liberarHash(TabelaHash tabela) {
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        SuspeitoNode* atual = tabela[i];
        while (atual != NULL) {
            SuspeitoNode* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
        tabela[i] = NULL; // Garantir que o ponteiro na tabela seja NULL
    }
}


// --- III. Funções da Árvore de Busca (BST) para Pistas (Atualizadas) ---

/**
 * @brief Cria e inicializa um novo nó de pista.
 */
PistaNode* criarPistaNode(const char* pista, const char* suspeito) {
    PistaNode* novoNode = (PistaNode*)malloc(sizeof(PistaNode));
    if (novoNode == NULL) {
        perror("Erro ao alocar memoria para o no da pista");
        exit(EXIT_FAILURE);
    }
    strncpy(novoNode->pista, pista, sizeof(novoNode->pista) - 1);
    novoNode->pista[sizeof(novoNode->pista) - 1] = '\0';
    
    strncpy(novoNode->suspeito, suspeito, sizeof(novoNode->suspeito) - 1);
    novoNode->suspeito[sizeof(novoNode->suspeito) - 1] = '\0';
    
    novoNode->esquerda = NULL;
    novoNode->direita = NULL;
    return novoNode;
}

/**
 * @brief Insere uma nova pista na BST (agora associada a um suspeito).
 */
PistaNode* inserirPista(PistaNode* raiz, const char* pista, const char* suspeito) {
    if (raiz == NULL) {
        return criarPistaNode(pista, suspeito);
    }

    int comparacao = strcmp(pista, raiz->pista);

    if (comparacao < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, pista, suspeito);
    } else if (comparacao > 0) {
        raiz->direita = inserirPista(raiz->direita, pista, suspeito);
    }
    // Não insere se a pista já existe

    return raiz;
}

/**
 * @brief Realiza a travessia Inorder na BST para exibir pistas e suspeitos associados.
 */
void exibirPistasEmOrdem(PistaNode* raiz) {
    if (raiz != NULL) {
        exibirPistasEmOrdem(raiz->esquerda);
        printf("* %s -> Suspeito: %s\n", raiz->pista, raiz->suspeito);
        exibirPistasEmOrdem(raiz->direita);
    }
}

/**
 * @brief Libera a memória alocada para a BST de pistas.
 */
void liberarPistas(PistaNode* raiz) {
    if (raiz == NULL) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}


// --- IV. Funções da Árvore Binária de Salas (Construção) ---

/**
 * @brief Cria e inicializa um novo nó (Sala).
 */
Sala* criarSala(const char* nome, const char* pista, const char* suspeito) {
    Sala* novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Erro ao alocar memoria para a sala");
        exit(EXIT_FAILURE);
    }
    
    strncpy(novaSala->nome, nome, sizeof(novaSala->nome) - 1);
    novaSala->nome[sizeof(novaSala->nome) - 1] = '\0';
    
    if (pista != NULL) {
        strncpy(novaSala->pistaEncontrada, pista, sizeof(novaSala->pistaEncontrada) - 1);
        strncpy(novaSala->suspeitoAssociado, suspeito, sizeof(novaSala->suspeitoAssociado) - 1);
        novaSala->pistaEncontrada[sizeof(novaSala->pistaEncontrada) - 1] = '\0';
        novaSala->suspeitoAssociado[sizeof(novaSala->suspeitoAssociado) - 1] = '\0';
    } else {
        novaSala->pistaEncontrada[0] = '\0';
        novaSala->suspeitoAssociado[0] = '\0';
    }
    
    novaSala->pistaColetada = 0;
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

/**
 * @brief Constrói a estrutura da árvore da mansão com pistas e suspeitos.
 */
Sala* construirMapaCompleto() {
    // Sala* criarSala(nome, pista, suspeito);
    
    // Nível 0: Raiz
    Sala* hallEntrada = criarSala("Hall de Entrada", NULL, NULL);

    // Nível 1
    hallEntrada->esquerda = criarSala("Sala de Jantar", NULL, NULL);
    hallEntrada->direita = criarSala("Biblioteca", NULL, NULL);

    // Nível 2
    // Suspeitos: D. Clara (Dona da Casa), Sr. Bartolomeu (Mordomo)
    hallEntrada->esquerda->esquerda = criarSala("Cozinha", "Faca de prata no armario", "Sr. Bartolomeu");
    hallEntrada->esquerda->direita = criarSala("Despensa", NULL, NULL); 

    hallEntrada->direita->esquerda = criarSala("Escritorio", "Carta confidencial", "Sr. Bartolomeu");
    hallEntrada->direita->direita = criarSala("Sala de Trofeus", "Luva de couro preta", "Dona Clara"); 

    // Nível 3 (Folhas)
    // Suspeitos: Dr. Victor (Medico), Sr. Bartolomeu
    hallEntrada->esquerda->esquerda->esquerda = criarSala("Jardim de Inverno", "Perfume Floral", "Dr. Victor"); 
    hallEntrada->esquerda->esquerda->direita = criarSala("Porão", "Diário rasgado", "Dona Clara");

    // Suspeitos: Dr. Victor, Sr. Bartolomeu
    hallEntrada->direita->esquerda->esquerda = criarSala("Quarto Principal", NULL, NULL);
    hallEntrada->direita->esquerda->direita = criarSala("Sotão", "Botão de casaco azul", "Sr. Bartolomeu");
    
    return hallEntrada;
}

/**
 * @brief Libera a memória alocada para a árvore da mansão.
 */
void liberarMapa(Sala* raiz) {
    if (raiz == NULL) return;
    liberarMapa(raiz->esquerda);
    liberarMapa(raiz->direita);
    free(raiz);
}


// --- V. Lógica de Exploração e Coleta de Pistas (Atualizada) ---

/**
 * @brief Tenta coletar a pista de uma sala e a insere na BST e na Tabela Hash.
 */
void coletarPista(Sala* sala, PistaNode** pistasRaiz, TabelaHash tabela) {
    if (sala->pistaEncontrada[0] != '\0' && sala->pistaColetada == 0) {
        const char* pista = sala->pistaEncontrada;
        const char* suspeito = sala->suspeitoAssociado;
        
        printf("\n✨ **PISTA ENCONTRADA!** Voce encontrou: **%s**.\n", pista);
        printf("Associando a pista ao suspeito: **%s**.\n", suspeito);
        
        // 1. Insere na BST (Nível Aventureiro)
        *pistasRaiz = inserirPista(*pistasRaiz, pista, suspeito);
        
        // 2. Insere/Atualiza na Tabela Hash (Nível Mestre)
        inserirOuAtualizarNaHash(tabela, suspeito);
        
        // 3. Marca como coletada
        sala->pistaColetada = 1; 
    } else if (sala->pistaColetada == 1) {
        printf("\n(Ja coletou a pista: %s)\n", sala->pistaEncontrada);
    } else {
        printf("\nNenhuma pista importante foi encontrada nesta sala.\n");
    }
}

/**
 * @brief Permite a exploração interativa (Menu Principal).
 */
void explorarSalas(Sala* raiz, PistaNode** pistasRaiz, TabelaHash tabela) {
    Sala* atual = raiz;
    char escolha;

    printf("\n--- Explorando a Mansão e Coletando Pistas ---\n");
    printf("Voce esta em: **%s**\n", atual->nome);

    while (1) {
        coletarPista(atual, pistasRaiz, tabela);

        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("\n🚨 **Fim do Caminho!** Nao ha mais saidas nesta sala (**%s**).\n", atual->nome);
            break; 
        }

        printf("\nOpcoes de Acao:\n");
        if (atual->esquerda != NULL) {
            printf(" (e) Esquerda -> %s\n", atual->esquerda->nome);
        }
        if (atual->direita != NULL) {
            printf(" (d) Direita -> %s\n", atual->direita->nome);
        }
        printf(" (v) Visualizar Pistas e Associacoes (BST)\n");
        printf(" (h) Analisar Suspeitos (Tabela Hash)\n");
        printf(" (s) Sair da Exploracao\n");

        printf("Escolha ('e', 'd', 'v', 'h', 's'): ");
        if (scanf(" %c", &escolha) != 1) {
            while(getchar() != '\n'); 
            continue; 
        }
        escolha = tolower(escolha);

        if (escolha == 's') {
            printf("\nSaindo da exploracao da mansao.\n");
            break;
        } else if (escolha == 'v') {
            printf("\n--- Caderno de Investigacao (Pistas e Suspeitos) ---\n");
            if (*pistasRaiz == NULL) {
                printf("Nenhuma pista coletada ainda.\n");
            } else {
                exibirPistasEmOrdem(*pistasRaiz);
            }
            printf("----------------------------------------------------\n");
            continue; 
        } else if (escolha == 'h') {
            exibirSuspeitos(tabela);
            continue; 
        } else if (escolha == 'e') {
            if (atual->esquerda != NULL) {
                atual = atual->esquerda;
                printf("\nVoce avanca para: **%s**\n", atual->nome);
            } else {
                printf("🚫 Nao ha caminho para a esquerda.\n");
            }
        } else if (escolha == 'd') {
            if (atual->direita != NULL) {
                atual = atual->direita;
                printf("\nVoce avanca para: **%s**\n", atual->nome);
            } else {
                printf("🚫 Nao ha caminho para a direita.\n");
            }
        } else {
            printf("❌ Opcao invalida. Tente novamente.\n");
        }
    }
}


// --- VI. Função Principal (Unificada) ---
int main() {
    // Inicialização das Estruturas
    PistaNode* cadernoDePistas = NULL;
    TabelaHash registroSuspeitos;
    // Inicializa a Tabela Hash com NULL
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        registroSuspeitos[i] = NULL;
    }
    
    printf("==================================================\n");
    printf("    ENIGMA STUDIOS: Nivel Mestre (C) \n");
    printf("    Integrando Arvore Binaria, BST e Tabela Hash\n");
    printf("==================================================\n");

    // 1. Cria a arvore binaria (Mapa da Mansão) com pistas e suspeitos
    Sala* mapaDaMansao = construirMapaCompleto();

    // 2. Inicia a interacao de exploracao, coleta e associação
    explorarSalas(mapaDaMansao, &cadernoDePistas, registroSuspeitos);
    
    // 3. Resultado Final (Análise de Evidências)
    printf("\n\n=============== FIM DA INVESTIGACAO ==============\n");
    
    // A) Exibe a lista final de suspeitos e contagens (Hash)
    exibirSuspeitos(registroSuspeitos);
    
    // B) Determina o culpado (Hash)
    encontrarSuspeitoMaisCitado(registroSuspeitos);
    
    // 4. Libera a memoria
    liberarMapa(mapaDaMansao);
    liberarPistas(cadernoDePistas);
    liberarHash(registroSuspeitos);
    
    printf("\nPrograma encerrado. Memoria liberada com sucesso.\n\n");
    
    return 0;
}