// Bibliotecas do próprio projeto:
#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <iostream>
#include <string>
#include <array>
#include <chrono>
#include <thread>
#include <memory>
#include <ranges>
// Biblioteca legada do C:
#include <cerrno>
#include <cstring>
// Biblioteca do API Unix:
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;
using namespace chrono;
using namespace this_thread;

const int NUMERO_DE_MSG = 10;
const string_view PATHNAME = "/tubulação";

auto create_pipeline(void) -> mqd_t {
   mqd_t mensageiro;
   int bandeiras = O_RDONLY;
   array<uint8_t, MAX_SERIAL> buffer{0};
   struct mq_attr atributos;
   int permissoes = 0600;
   
   atributos.mq_flags = 0;
   atributos.mq_maxmsg = NUMERO_DE_MSG;
   atributos.mq_msgsize = MAX_SERIAL;
   atributos.mq_curmsgs = 0;
   mensageiro = mq_open(PATHNAME.data(), bandeiras, permissoes, &atributos);

   if (mensageiro == -1)
   {
      cerr << "Erro ao tentar abrir \"" << PATHNAME << "\"." << endl
           << "[erro]" << strerror(errno) << endl;
      std::terminate();
   }
   return mensageiro;
}

void pausa_esperada(void)
   { sleep_for(milliseconds(800)); }

int main(void)
{
   mqd_t mensageiro;
   array<uint8_t, MAX_SERIAL> buffer{0};
   
   mensageiro = create_pipeline();

   for (auto X: std::views::iota(1, NUMERO_DE_MSG))
   {
      auto bufferptr = buffer.data();
      auto ptr = reinterpret_cast<char*>(bufferptr);

      if (mq_receive(mensageiro, ptr, MAX_SERIAL, NULL) != -1)
      {
         auto objeto = Entrada::deserializa(buffer); 
         cout << objeto << endl;
      } else {
         cerr << "Não foi possível ler os bytes.\n"
              << "[erro]" << strerror(errno) << endl;
         std::terminate();
      }
      pausa_esperada();
   }

   if (mq_close(mensageiro) != -1)
      cout << "Mensageiro fechado com sucesso.\n";
}
