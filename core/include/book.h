#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "errorlib.h"


typedef uint32_t tm_stmp_t;

typedef struct {
    tm_stmp_t timestamp;
    uint32_t order_id;
    uint32_t client_id;
    uint32_t quantity;
    double price;
    char symbol[8];
    char side;
    bool is_valid;
} obk_order_t;

typedef struct obk_book_private_s* obk_book_pt; // Ponteiro opaco

typedef obk_order_t* obk_order_pt;


/**
 * @brief Aloca e inicializa uma nova instância opaca do livro de ordens.
 * @param book Ponteiro duplo a ser atualizado com o endereço do novo livro.
 * @return ERR_NONE em caso de sucesso, ERR_MEM se a alocação falhar.
 */
ret_code_t obk_initialize_book(obk_book_pt* book);

/**
 * @brief Libera de forma segura a memória heap alocada e anula o ponteiro.
 * @param book Ponteiro duplo para a instância do livro a ser limpa.
 * @return ERR_NONE em caso de sucesso, ERR_MEM se o ponteiro já for NULL.
 */
ret_code_t obk_clear_book(obk_book_pt* book);

/**
 * @brief Copia explicitamente os dados de um índice do buffer para uma estrutura de destino.
 * @param cpy Ponteiro de destino que receberá os atributos da ordem.
 * @param buffer Ponteiro para o array de origem contendo os registros ativos.
 * @param idx Índice no array.
 * @return 0 em caso de sucesso.
 */
ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx);

/**
 * @brief Insere uma ordem no lado correto do livro e o ordena.
 * @param book Handle opaco para o livro de negociação.
 * @param cpy Ponteiro para os dados da ordem recebida.
 * @return ERR_NONE em caso de sucesso, ERR_ORD para lado inválido, ERR_MEM se o buffer estiver cheio.
 */
ret_code_t obk_insert_order(obk_book_pt book, obk_order_pt cpy);

/**
 * @brief Modifica o volume (quantidade) da ordem de maior prioridade em um dado lado.
 * @param book Handle opaco para o livro de negociação.
 * @param qty Novo volume a ser definido no índice do topo.
 * @param side Identificador do lado do livro ('A' ou 'B').
 * @return ERR_NONE em caso de sucesso, ERR_ORD para configuração de lado inválida.
 */
ret_code_t obk_change_order(obk_book_pt book, uint32_t qty, char side);

/**
 * @brief Remove a ordem de maior prioridade do topo do heap do lado especificado.
 * @param book Handle opaco para o livro de negociação.
 * @param side Identificador do lado do livro ('A' ou 'B').
 * @return ERR_NONE em caso de sucesso, ERR_ORD se o livro estiver vazio ou o lado for inválido.
 */
ret_code_t obk_remove_order(obk_book_pt book, char side);

/**
 * @brief Retorna uma cópia isolada da ordem de maior prioridade.
 * @param book Handle opaco para o livro de negociação.
 * @param side Identificador do lado do livro ('A' ou 'B').
 * @return Cópia estrutural representando o elemento de maior prioridade.
 */
obk_order_t obk_get_order(obk_book_pt book, char side);

/**
 * @brief Retorna o número de ordens no lado de venda (ask).
 */
int32_t obk_ask_count(obk_book_pt book);

/**
 * @brief Retorna o número de ordens no lado de compra (bid).
 */
int32_t obk_bid_count(obk_book_pt book);
