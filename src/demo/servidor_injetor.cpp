/* O mesmo que o código injetor de 'entradas'. Entretanto, este já usa 
 * o 'servidor' criado. */

// Biblioteca padrão do C++:
#include <array>
#include <string>
#include <thread>
#include <iostream>
#include <chrono>
#include "comunicacao.hpp"
// GNU library of POSIX:
#include <sys/wait.h>
#include <signal.h>

#define Ig std::ignore

static void info_compactada_da_entrada(Entrada& obj); 
static void transmissor_infinito(Entrada& objeto);
static void lancamento_de_entradas_por_varios_processos(void); 
static void transmissao_de_entradas_limitadas
  (Servidor& mensageiro, std::array<Entrada, 5>& In);

int main(void)
{
   lancamento_de_entradas_por_varios_processos();
}

void info_compactada_da_entrada(Entrada& obj) {
   using Cloque = std::chrono::high_resolution_clock;
   using TimePoint = Cloque::time_point;
   using seg = std::chrono::seconds;

   const auto SEP = " ";
   const auto TAB = '\t';
   int id; size_t current, total;
   float percentual;
   TimePoint inicio, fim; bool marcados;
   auto Casting = [](auto d) 
      { return std::chrono::duration_cast<seg>(d); };

   std::tie(current, total, id) = obj.getIntAtributos();
   std::tie(inicio, fim, marcados) = obj.getTPAtributos();
   percentual = static_cast<float>(current) / static_cast<float>(total);

   std::ostringstream tempos_fmt, percentual_fmt, cabecalho_fmt;
   auto agora = Cloque::now();
   auto decorrido_a = Casting(fim - inicio);
   auto decorrido_b = Casting(agora - fim);

   // Formatando todas partes ...
   tempos_fmt << decorrido_a.count() << "seg" << " | " 
              << decorrido_b.count() << "seg";
   percentual_fmt.precision(3);
   percentual_fmt << percentual * 100 << '%';
   cabecalho_fmt << "[" << id << SEP << '|' << SEP 
                 << BoolStr(marcados) << "]"; 

   // Impressão do que foi construído.
   std::cout << cabecalho_fmt.str() << SEP <<  obj.getRotulo() << SEP 
             << TAB << percentual_fmt.str()<< TAB << tempos_fmt.str() 
             << std::endl;
}

void transmissor_infinito(Entrada& objeto)
{
   Servidor mensageiro(&objeto);
   auto inicio = std::chrono::steady_clock::now();
   auto termino = inicio;
   const auto LIMITE = std::chrono::seconds(120);
   const std::chrono::milliseconds PAUSA(600);

   do {
      std::this_thread::sleep_for(PAUSA);
      mensageiro.enviar();
      ++objeto;
      info_compactada_da_entrada(objeto);
      // std::cout << objeto << std::endl;

      termino = std::chrono::steady_clock::now();

   } while((termino - inicio) < LIMITE);
}

void lancamento_de_entradas_por_varios_processos(void) {
   int status;

   // Distribui as entradas criadas entre duas mais forques.
   if (fork() == 0) {
      auto In = Entrada("Morango", 0, 1000);
      transmissor_infinito(In);
      std::terminate();

   } else {
      if (fork() == 0) {
         auto In = Entrada("Melância", 0, 1200);
         transmissor_infinito(In);
         std::terminate();

      } else {
         if (fork() == 0) {
            auto In = Entrada("Abacaxi", 0, 2000);
            transmissor_infinito(In);
            std::terminate();
         } else {
            if (fork() == 0) {
               auto In = Entrada("Laranja", 0, 500);
               transmissor_infinito(In);
               std::terminate();
            }
            else {
               auto In = Entrada("Jacá", 0, 700);
               transmissor_infinito(In);
            }
         }
      }
   }

   wait(&status);
   std::cout << "Tudo foi terminado com sucesso.\n";
   std::cout << "exit code: " << status << std::endl;
}

void transmissao_de_entradas_limitadas
  (Servidor& mensageiro, std::array<Entrada, 5>& In)
{
   using namespace std::chrono;
   using namespace std::this_thread;

   const milliseconds PAUSA(600);
   int cursor = 0;

   mensageiro.adiciona(&In[4]);
   mensageiro.adiciona(&In[2]);
   mensageiro.adiciona(&In[0]);
   mensageiro.adiciona(&In[3]);
   mensageiro.adiciona(&In[1]);

   std::cout << "Total de entradas inicialmente: " << mensageiro.quantidade()
      << std::endl;

   do {
      auto quantia = mensageiro.quantidade();
      sleep_for(PAUSA);
      mensageiro.enviar();
      info_compactada_da_entrada(In[cursor]);
      ++In[cursor];
      cursor = (cursor + 1) % 5;

      if (cursor == 0)
         std::cout << "Total de entradas: " << quantia << std::endl; 

   } while(!mensageiro.sem_entradas());
}

void lancamento_de_entradas_por_unico_processo(void) {
   Servidor mensageiro;
   std::array<Entrada, 5> input = {
      Entrada("Abajur", 0, 500),
      Entrada("Aquário", 0, 3800),
      Entrada("Laterna", 0, 999),
      Entrada("Bola", 0, 200),
      Entrada("Carro", 0, 1700)
   };

   transmissao_de_entradas_limitadas(mensageiro, input);
   std::cout << "Tudo foi terminado com sucesso.\n";
}

