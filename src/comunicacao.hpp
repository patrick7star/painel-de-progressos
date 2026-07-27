#ifndef __COMUNICACAO_H__
#define __COMUNICACAO_H__
#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <chrono>
#include <queue>
#include <vector>
#include <filesystem>
// Biblioteca do Unix:
#include <mqueue.h>


class Servidor {
/*   Gerenciamento o envio de dados referentes à uma ou mais 'entradas'. Sabe
 * quando enviar, e parar de enviar, até pausar o processo de transmissão.
 * Também registra quantas vezes foi enviado, e o tanto de bytes que foi 
 * acumulado no total deste envio.
 */

 protected:
   using Clock = std::chrono::steady_clock;
   using Queue = std::queue<Entrada*>;
   using TimePoint = Clock::time_point;

   // Referência da 'entrada' que sempre transmite seu sinal.
   Queue fila;   
   // Cronômetro que registra o tempo necessário prá próxima remessa.
   mutable TimePoint inicio;

   bool pronto_pra_envio(void) const;
   void remove_entradas_expiradas(void);

 private:
   // 'file descriptor' do 'named pipe', onde os dados são enviados.
   int tubulacao;

   void envia_uma_entrada(Entrada& obj) const;
   bool entrada_pertencente(Entrada& obj);

 public:
   // Todos construtores que existem.
   Servidor(void);
   Servidor(Entrada* pointer);
   ~Servidor(void); 

   // Envia todas as 'entradas' no 'pool'.
   void enviar(void); 
   // Adiciona 'entrada' pra ser transmitida, se não estiver finalizada.
   bool adiciona(Entrada* pointer); 
   /*   O mesmo que dizer que está vázia. Informa que servidor não tem 
    * qualquer 'entrada' prá envio. */
   bool sem_entradas(void) const;
   /*   Informa a quantia atual de 'entradas' que estão ativamente sendo 
    * transmitidas pelo o 'servidor'.
    *   Não é apenas quantas você adiciona, porque um tempo após o termino
    * do progresso de uma, ela apenas expira, e é removida da fila interna
    * de transmissão. */
   int quantidade(void) const;
};


class ServidorMessageQueue: public Servidor 
{
 using ServidorMQ = ServidorMessageQueue;
 using path = std::filesystem::path;

 private:
   // Caminho da tubulação:
   path caminho;
   mqd_t tubulacao;

   void envia_uma_entrada(Entrada& input);

 public:
   // Métodos construtores:
   ServidorMessageQueue  (void); 
   ServidorMessageQueue  (Entrada* pointer);
   ~ServidorMessageQueue (void); 

   // Envia todas as 'entradas' via uma named message-queue.
   void enviar(void); 
};


class Cliente {
/*   Receberá as 'entradas' que são enviadas via 'pipe'. Entretanto, ela tem
 * uma lista interna, que aceita apenas 'entradas' não repetidas, e que 
 * sejam atualizadas. */

 using Lista = std::vector<Entrada>;
 using Clock = std::chrono::system_clock;
 using TimePoint = std::chrono::time_point<Clock>;

 private:
   // Viaduto(named pipe) de 'entradas' recebidas.
   int tubo;
   // Referência mutavel a lista de 'Entradas' externa.
   Lista& colecao;
   Lista expiradas;
   // Cronômetro prá contar até a próxima inserção.
   TimePoint inicio;
   TimePoint tempo;

   void insercao_controlada(Entrada& obj);
   void tenta_ler_entrada(Bytes& Out);
   bool permicao_de_leitura(void);
   void remocao_de_entradas_expiradas(void);

 public:
   /* O construtor deve receber uma lista, pra que se aloque todas entradas
    * externas que foram recebidas, mesmo que ela esteja incialmente vázia. 
    */
   Cliente(Lista& lista);
   ~Cliente(void); 

   /* Lê alguns bytes, e tenta decodifica-lô numa 'entrada'. Se for possível,
    * insere nova, ou apenas atualiza se necessário. */
   bool receber(void); 
   /* Quantidades de 'Entradas' que já foram expiradas. */
   int quantidade(void) const;
};
#endif
