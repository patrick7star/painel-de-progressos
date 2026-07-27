#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <iostream>
#include <new>
#include <tuple>
#include <memory>
// Antiga biblioteca do C:
#include <climits>
// GNU library of POSIX:
#include <sys/types.h>
#include <unistd.h>

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
using namespace std;

const string ESPACO("  ");
// Comprimento da barra de progresso.
const int COMPRIMENTO = 28;
#define ig std::ignore
// Ignora blocos de declarações.
#define passar ;


static int digitos_necessarios(size_t X)
// Computa o total de dígitos que o valor passado ocupa.
   {  return (int)(floor(log10(X)) + 1); }

bool Entrada::esgotado(void) {
/* Diz que o progresso de uma 'entrada' está esgotado, quando a 'atual' taxa
 * excede ou igualá o 'total'. */ 
   this->registra_o_termimo(); 

   return (*this).atual == (*this).total && (*this).marcados; 
}
      
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                     Construturos e Destrutores
 * == == == == == == == == == == == == == == == == == == == == == == == == */
Entrada::Entrada(const string rotulo, size_t atual, size_t total, pid_t ID,
  TimePoint inicio, TimePoint termino, bool marcados) 
{ 
   this->rotulo = rotulo; 
   this->atual = atual; 
   this->total = total; 
   this->id = ID;
   this->inicio = inicio;
   this->termino = termino;
   this->marcados = marcados;
}

static string comando_fortune_string(void) {
/* Faz da string do programa 'fortunate' um rótulo de amostra para testar
 * o programa que está se criando. */
   const int MAX = 500;
   FILE* saida;
   char buffer[MAX];
   int lido;

   saida = popen("fortune -n 170", "r");
   lido = fread(buffer, sizeof(char), MAX, saida);
   pclose(saida);

   // Substituindo quebras de linhas por motivo de compatibilidade.
   for (int q = 0; q < lido; q++) {
      char a = buffer[q];

      if (a == '\n' || a == '\t' || a == '\b' || a == '\r')
         buffer[q] = '*';
   }

   return string(buffer);
}

static tuple<size_t, size_t> 
gera_valores_aleatorios_debug(void) {
   using Distribuicao = uniform_int_distribution<size_t>;

   random_device motor;
   Distribuicao seletor(0, 300);
   string rotulo;
   size_t total, atual;

   rotulo = comando_fortune_string();
   total = seletor(motor);
   atual = seletor(motor);

   // Repara números incorretos, que podem ter sido sorteados.
   while (atual > total)
      atual = seletor(motor);

   return make_tuple(atual, total);
}

Entrada::Entrada(void) {
/* Usa o construtor 'default' para criar uma amostra, isso, ao menos na
 * versão de 'debug' deste programa. */
   this->rotulo = comando_fortune_string();
   this->id = getpid();
   this->inicio = Cloque::now();
   this->termino = Cloque::now();
   tie(this->atual, this->total) = gera_valores_aleatorios_debug();
}


/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                     Métodos Auxiliares Internos 
 * == == == == == == == == == == == == == == == == == == == == == == == == */
float Entrada::percentual(void) const {
/* Computa percentual já crescido da entrada. */
   float a = this->atual;
   float t = this->total;

   return a / t;
}

void Entrada::taxa_de_crescimento_a(void) {
/* Está taxa de crescimento leva em conta, intervalos de progressos definidos
 * para acionar uma nova velocidade de crescimento, que provavelmente é 
 * menor que a anterior. */
   // Percentual que ainda falta para terminar.
   float p = this->percentual();
   float P = 1.0 - p;
   // Quantia que tal "percentual faltante" significa.
   size_t T       = this->total;
   size_t R       = (size_t)(P * T);
   size_t um      = (size_t)(T * 0.01);
   size_t meio    = (size_t)(T * 0.005);
   size_t decimo  = (size_t)(R * 0.10);
   size_t quinto  = (size_t)(R * 0.20);
   size_t quarto  = (size_t)(R * 0.25);
   size_t terco   = (size_t)(R * 0.333);

   if (p < 0.20 && terco > 0)
      this->atual += terco;
   else if (p >= 0.20 && p < 0.40 && quarto > 0)
      this->atual += quarto;
   else if (p >= 0.40 && p < 0.60 && quinto > 0)
      this->atual += quinto;
   else if (p >= 0.60 && p < 0.90 && decimo > 0)
      this->atual += decimo;
   else if (p >= 0.90 && p < 0.95 && um > 0)
      this->atual += um;
   else if (p >= 0.95 && p < 0.98 && meio > 0)
      this->atual += meio;
   else 
      this->atual++;
}

void Entrada::taxa_de_crescimento_b(void) {
/* Está derivada aplica a maior taxa de crescimento que pode haver, e se ela
 * se esgotar, aplicar uma de menor intensidade. */
   // Percentual que ainda falta para terminar.
   float p = this->percentual();
   float P = 1.0 - p;
   // Quantia que tal "percentual faltante" significa.
   size_t T       = this->total;
   size_t R       = (size_t)(P * T);
   size_t um      = (size_t)(T * 0.01);
   size_t meio    = (size_t)(T * 0.005);
   size_t decimo  = (size_t)(R * 0.10);
   size_t quinto  = (size_t)(R * 0.20);
   size_t quarto  = (size_t)(R * 0.25);
   size_t terco   = (size_t)(R * 0.333);

   if (terco > 0)
      this->atual += terco;
   else if (quarto > 0)
      this->atual += quarto;
   else if (quinto > 0)
      this->atual += quinto;
   else if (decimo > 0)
      this->atual += decimo;
   else if (um > 0)
      this->atual += um;
   else if (meio > 0)
      this->atual += meio;
   else 
      this->atual++;
}

static void desenha_progresso_colorido(float p)
{
// Desenha um progresso colorido onde o cursor já está na janela.
   // Comprimento da barra de progresso.
   const char BARRA = 'o';
   const char VAZIO = '.';
   // Total desta barra que foi carregado.
   int QUANTIA = (int)(COMPRIMENTO * p);
   int cor = 0, n = 0;
   const char DELIMITADORES[3] = "[]";
   stringstream fmt;

   /* Cor definida no nível do atual progresso. As cores estão atribuidas
    * as seguintes porcentagens. 
    *       85% para cima, fica tudo azul.
    *       65% para cima, fica tudo verde.
    *       25% para cima, fica tudo amarelo.
    *       25 prá baxio, o restante, fica em vermelho.
    */
   if      (p >= 0.85)              cor = 96;
   else if (p >= 0.65 && p < 0.85)  cor = 98;
   else if (p >= 0.25 && p < 0.65)  cor = 97;
   else                             cor = 99;

   /* Criando uma formatação de barra de progresso em sí. */
   // Parte branca bem destacada para os delimitadores.
   attron(COLOR_PAIR(93) | A_BOLD);
   addch(DELIMITADORES[0]);
   attroff(COLOR_PAIR(93) | A_BOLD);
   
   for (n = 1; n < COMPRIMENTO; n++) 
   {
      if (n < QUANTIA) {
         attron(COLOR_PAIR(cor) | A_BOLD);
         addch(BARRA);
         attroff(COLOR_PAIR(cor) | A_BOLD);
      } else {
         attron(COLOR_PAIR(93) | A_NORMAL);
         addch(VAZIO);
         attroff(COLOR_PAIR(93) | A_NORMAL);
      }
   }
   // O demilitador de fechamento tem um branco bem forte.
   attron(A_BOLD | COLOR_PAIR(93));
   addch(DELIMITADORES[1]);
   attroff(A_BOLD | COLOR_PAIR(93));
   // Percentual é pintado conforme ...
   fmt.precision(3);
   fmt.width(5);
   fmt << p * 100.0 << '%'; 
   attron(COLOR_PAIR(93) | A_NORMAL);
   addstr(fmt.str().c_str());
   attroff(COLOR_PAIR(93) | A_NORMAL);
}

static void desenha_contadores_colorido(size_t a, size_t t)
{
   int total_digitos = digitos_necessarios(t);
   int atributos = COLOR_PAIR(93) | A_UNDERLINE | A_BOLD;
   ostringstream format;
   
   attron(atributos);
   format.width(total_digitos);
   format << (int)a;
   addstr(format.str().c_str());

   format.str("");
   format << '/';
   addstr(format.str().c_str());

   format.str("");
   format << (int)t;
   addstr(format.str().c_str());
   attroff(atributos);
   addstr(ESPACO.c_str());
}

static void desenha_rotulo_colorido(string& rotulo)
{
   attron(COLOR_PAIR(97) | A_BOLD);
   addstr(rotulo.c_str());
   attroff(COLOR_PAIR(97) | A_BOLD);
}

void Entrada::desenha(WINDOW* janela, int linha)
{
   int total_digitos = digitos_necessarios(this->total);
   int contadores_width = 2 * total_digitos + ESPACO.length();
   int progresso_width = COMPRIMENTO + 2;
   /* Os cincos contando o máximo de 3 digitos(inclui a vírgula), e um dígito 
    * da fração, então o adicional referente ao símbolo de percentual. */
   int percentual_width = 3 + 2 + 1;
   int largura = contadores_width + progresso_width + percentual_width;
   const int MARGEM = 3;

   // Desenha tudo formatado acima na telinha do ncurses.
   move(linha, 2);
   desenha_rotulo_colorido(this->rotulo);
   move(linha, COLS - largura - MARGEM);
   desenha_contadores_colorido(this->atual, this->total);
   desenha_progresso_colorido(this->percentual());
}

void Entrada::registra_o_termimo(void) {
/* Apenas registra o 'termino' se, e somente se, o valor 'atual' alcançou o
 * 'total'. Também se 'marcados' está indicando que tal termino já foi
 * marcado antes. */
   auto atual = this->atual;
   auto total = this->total;
   bool termino_nao_marcado = !(this->marcados);

   if (atual < total)
      passar
   else {
      // Apenas marcar uma única vez.
      if (termino_nao_marcado)
         this->marcados = true; 
      else
         passar
   }
   
   if (termino_nao_marcado)
      this->termino = Clock::now();
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Operadores de Sobrecarga 
 * == == == == == == == == == == == == == == == == == == == == == == == == */
Entrada& Entrada::operator++(){
/* Aumenta uma fração do percentual do que ainda é restante. Uma ferramenta
 * feita especialmente para debug, fazer tais entradas aumentarem de forma
 * arbitraria para que possa debuggar sua interface gráfica. */
   // Abandona se já estiver finalizado o progresso.
   if (this->percentual() > 1.0) return *this;

   (*this).taxa_de_crescimento_b();

   // Correção de excedências, se houve um acrescimo acima do máximo(cem).
   if ((*this).atual > (*this).total)
      (*this).atual = (*this).total;

   (*this).registra_o_termimo();
   return *this;
}

Entrada& Entrada::operator++(int){
   ++(*this);
   return *this;
}

string Entrada::to_string(void) const {
// Transforma a formatação numa string.
   ostringstream fmt_number, fmt_bar, fmt_label;
   // Total desta barra que foi carregado.
   int QUANTIA = (int)(COMPRIMENTO * this->percentual());
   // int total_digitos = (int)(floor(log10(this->total)) + 1);
   int total_digitos = digitos_necessarios(this->total);

   // Criando a formatação da parte que diz sobre a quantia total/e a atual.
   fmt_number.width(total_digitos);
   fmt_number << (int)this->atual;
   fmt_number << "/";
   fmt_number << (int)this->total; 
   fmt_number << ESPACO.c_str();
   /* Criando uma formatação de barra de progresso em sí. */
   fmt_bar << '[';
   for (int n = 1; n < COMPRIMENTO; n++) {
      if (n < QUANTIA)
         fmt_bar << 'o';
      else
         fmt_bar << '.';
   }
   fmt_bar << ']';
   fmt_bar.precision(4);
   fmt_bar.width(4);
   fmt_bar << this->percentual() * 100.0; 
   fmt_bar << '%';
   // Rótulo. Tenta encurtar se for longo demais.
   if (this->rotulo.length() > 50) {
      int count = 1;
      for (const char& letra: this->rotulo) {
         if (count++ > 50)
            break;
         fmt_label << letra;
      }
      fmt_label << ESPACO;
      fmt_label << "...";
      fmt_label << ESPACO;
   } else 
      fmt_label << this->rotulo;
   
   string Output(fmt_label.str());

   Output += ESPACO;
   Output += fmt_number.str();
   Output += fmt_bar.str();
   return Output;
}

Entrada& Entrada::operator=(size_t atual) {
/* Apenas coloca novo valor se, e somente se, ela exceder o antigo, e claro,
 * não exceder o limite. */
   const auto ATUAL = (*this).atual;
   const auto TOTAL = (*this).total;

   if (atual > ATUAL && atual < TOTAL)
      this->atual += ATUAL;

   (*this).registra_o_termimo();
   return *this;
}

Entrada& Entrada::operator+=(size_t incremento) {
/*   Adiciona a quantia desejada, se ela estiver no que é faltante para 
 * completar o progresso em andamento. */
   auto atual = (*this).atual;

   if (atual + incremento <= total)
      *this = (*this).atual + incremento;

   (*this).registra_o_termimo();
   return *this;
}

bool operator>=(Entrada& obj_a, Entrada& obj_b) {
   size_t atual_a, atual_b;

   tie(atual_a, ig, ig) = obj_a.getIntAtributos();
   tie(atual_b, ig, ig) = obj_b.getIntAtributos();

   bool entradas_iguais = (obj_a == obj_b);
   bool mais_atualizado = (atual_a > atual_b);

   return (entradas_iguais && mais_atualizado);
}

bool operator==(Entrada& lhv, Entrada& rhv) {
/* Verifica se duas 'entradas' são iguais. Aqui, leva-se em conta apenas 
 * o valor 'total' e 'pid'. O campo 'atual' fica para a operação relacional.
 */
   size_t atual_a, total_a, atual_b, total_b;
   pid_t id_a, id_b;

   tie(atual_a, total_a, id_a) = lhv.getIntAtributos();
   tie(atual_b, total_b, id_b) = rhv.getIntAtributos();

   return (total_a == total_b && id_a == id_b);
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Operadores de Sobrecarga 
 *                         e Formatação
 * == == == == == == == == == == == == == == == == == == == == == == == == */
static string extrai_progresso(const Entrada& obj) {
   const string auxiliar = obj.to_string();
   auto p = auxiliar.find("/", 0);
   auto rotulo = auxiliar.substr(0, p - 6);
   auto q = auxiliar.find("[", 0);
   auto t = auxiliar.find("]", q);
   auto barra = auxiliar.substr(q, t + 5);

   return barra;
}

static string debug_entrada(const Entrada& obj)
{
   pid_t id; size_t atual, total;
   ostringstream Fmt("Entrada");
   auto barra = extrai_progresso(obj);
   TimePoint a, b;
   bool marcado;
   auto Cast = [](auto X) { return duration_cast<chrono::seconds>(X); };
   auto agora = Clock::now();

   tie(atual, total, id) = obj.getIntAtributos();
   tie(a, b, marcado) = obj.getTPAtributos();

   Fmt << "Entrada [" << id << "]" << endl;
   Fmt << "\tRótulo: \"" << obj.getRotulo() << '\"'<< endl;
   Fmt << "\tBarra: " << barra << endl;
   Fmt << "\tInfo: " << atual << " e " << total << endl;
   Fmt << "\tPassados(" << BoolStr(marcado) <<  "): " 
       << Cast(b - a).count() << "seg" << " | " 
       << Cast(agora - b).count() << "seg" << endl;
      
   return Fmt.str();
}

ostream& operator<<(ostream& Output, const Entrada& obj) {
// Formatação do tipo de dado na saída padrão, ou em que está acoplada a ela.
   pid_t id; size_t atual, total;
   const string auxiliar = obj.to_string();
   auto p = auxiliar.find("/", 0);
   auto rotulo = auxiliar.substr(0, p - 6);
   auto q = auxiliar.find("[", 0);
   auto t = auxiliar.find("]", q);
   auto barra = auxiliar.substr(q, t + 5);
   auto m = auxiliar.find("/");
   tie(ig, total, ig) = obj.getIntAtributos();
   auto n = digitos_necessarios(total);
   auto l = ESPACO.length();
   auto info = auxiliar.substr(m - n, l + 2 * n);
   tie(atual, total, id) = obj.getIntAtributos();

   Output << "PID: " << id << endl;
   Output << "Rótulo: \"" << rotulo  << '\"'<< endl;
   Output << "Barra: " << barra << endl;
   Output << "Info: " << info << ESPACO << atual << " e " << total 
          << endl << endl;

   return Output;
}

ostream& operator<<(ostream& Output, Entrada& obj) {
// Formatação do tipo de dado na saída padrão, ou em que está acoplada a ela.
   Output << debug_entrada(obj);
   return Output;
}


/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                         Encapsulamento
 * == == == == == == == == == == == == == == == == == == == == == == == == */
const string& Entrada::getRotulo(void) const
   { return this->rotulo; }

size_t Entrada::getTotal(void) const
   { return this->total; }

size_t Entrada::getAtual(void) const
   { return this->atual; }

tuple<size_t, size_t, pid_t>
Entrada::getIntAtributos(void) const {
   auto atual = this->atual;
   auto total = this->total;
   auto id = this->id;

   return make_tuple(atual, total, id);
}

auto Entrada::getTPAtributos(void) const -> AtributosTP
   { return make_tuple(this->inicio, this->termino, this->marcados); }

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Serialização e Deserialização
 * Ordem de serialização e deserialização segue a seguinte ordem:
 *    - atual        (8 bytes)
 *    - total        (8 bytes)
 *    - id           (4 bytes)
 *    - inicio       (8 bytes)
 *    - termino      (8 bytes)
 *    - marcados     (1 byte)
 *    - rótulo       (restante do MAX_SERIAL)
 * == == == == == == == == == == == == == == == == == == == == == == == == */
time_t selo_de_tempo(TimePoint x) { 
/* Retorna o 'timestamp', em segundos, desde o começo do UNIX em 1970. */
   using namespace std::chrono;

   auto duracao = x.time_since_epoch();
   auto segundos = duration_cast<seconds>(duracao);

   return segundos.count(); 
}

template<typename T>
static void escreve_na_array
  (array<uint8_t, MAX_SERIAL>& fita, T* dados, int& cursor)
{
/* Terceirização de escreve os bytes do tipo na gigante array, que será 
 * transmitida. Como o padrão do código é muito simples, reduz bastante
 * escreve a função que faz isso, e marca onde e, o que já foi escrito. */
   // Preenche com zeros, principalmente para a string que será escrita.
   if (cursor == 0)
      uninitialized_fill(fita.begin(), fita.end(), 0x0);

   uint8_t* fonte = reinterpret_cast<uint8_t*>(dados);
   uint8_t* destino = fita.data() + cursor;

   uninitialized_copy_n(fonte, sizeof(T), destino);
   cursor += sizeof(T);
}

static void escreve_string_na_array
  (array<uint8_t, MAX_SERIAL>& fita, string& dados, int* cursor)
{
/* O mesmo que o acima, porém específico pra strings. */
   auto length = dados.length();
   const int size = sizeof(char);
   const char* pointer = dados.c_str();
   char* ptr = const_cast<char*>(pointer);
   uint8_t* fonte = reinterpret_cast<uint8_t*>(ptr);
   uint8_t* destino = fita.data() + *cursor;

   uninitialized_copy_n(fonte, size *length, destino);
   *cursor += length;
}

Bytes Entrada::serializa(void) {
/*   Sem sucesso ainda com um tipo serialização dinâmica, tentarei ao invés 
 * a versão estática. Assim, o retorno ao invés de uma fila, será uma array
 * fixa, com os bytes todos enfileirados. A ordem de inserção, ainda continua
 * a mesma que a anterior: em primeiro, os bytes dos valores com tamanhos 
 * fixos em tempo de compilação; depois os tipos de dados variaveis, que 
 * neste caso é essencialmente a string. O tamanho de tal array é definida
 * pela constante 'MAX_SERIALIZACAO'.
 *   Então a organização dos dados serializados fica: na parte estática, 
 * atributos 'atual' e 'total', nesta ordem; já a parte dinâmica fica o 
 * restante, como é a única, tal fase é extremamente fácil, o que sobra
 * do buffer, apenas é preenchidos com caractéres nulos. */
   array<uint8_t, MAX_SERIAL> Out;
   int escrito = 0;
   // Converte em 'selos de tempos', estes, medidos em segundos.
   time_t inicio = selo_de_tempo(this->inicio);
   time_t fim = selo_de_tempo(this->termino);

   escreve_na_array(Out, &this->atual, escrito);
   escreve_na_array(Out, &this->total, escrito);
   escreve_na_array(Out, &this->id, escrito);
   escreve_na_array(Out, &inicio, escrito);
   escreve_na_array(Out, &fim, escrito);
   escreve_na_array(Out, &this->marcados, escrito);
   escreve_string_na_array(Out, this->rotulo, &escrito);

   return Out;
}

static size_t deserializa_sizet(Bytes& In, int* cursor) {
   auto bytes = In.data();
   size_t* pointer = (size_t*)(bytes + *cursor);
   auto valor = *pointer;
   const int sz = sizeof(size_t);

   (*cursor) += sz;
   return valor;
}

static string deserializa_string(Bytes& In, int* cursor) {
   auto restante = In.size() - (*cursor + 1); 
   auto rawptr = In.data();
   auto str = reinterpret_cast<char*>(rawptr);
   auto buffer = new char[restante];
   string Output;
   auto fonte = str + *cursor;

   uninitialized_copy_n(fonte, restante, buffer);
   Output = string(buffer);
   delete[] buffer;
   (*cursor) += restante;

   return Output;
}

static pid_t deserializa_pidt (Bytes& In, int* cursor) {
   auto bytes = In.data();
   pid_t* pointer = (pid_t*)(bytes + *cursor);
   auto valor = *pointer;
   const int sz = sizeof(pid_t);

   (*cursor) += sz;
   return valor;
}

static bool deserializa_bool(Bytes& In, int* cursor) {
   auto bytes = In.data();
   auto valor = (*(bytes + *cursor));

   (*cursor) += sizeof(uint8_t);
   return static_cast<bool>(valor);
}

Entrada Entrada::deserializa(Bytes& In) { 
   int lido = 0;
   auto atual = deserializa_sizet(In, &lido);
   auto total = deserializa_sizet(In, &lido);
   auto id = deserializa_pidt(In, &lido);
   auto inicio = deserializa_sizet(In, &lido);
   auto termino = deserializa_sizet(In, &lido);
   auto marcados = deserializa_bool(In, &lido);
   auto rotulo = deserializa_string(In, &lido);

   if (total < 0)
      throw invalid_argument("bytes inválidos");

   return Entrada(
      rotulo, atual, total, id, 
      Clock::from_time_t(inicio),
      Clock::from_time_t(termino),
      marcados
   );
}


#ifdef __unit_tests__
#ifdef __linux__
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                   Testes Unitários de Entrada
 * == == == == == == == == == == == == == == == == == == == == == == == == */
 #include <iostream>
 #include <queue>
 #include <thread>
 #include <cassert>

template <typename T>
static void print_queue(queue<T>& fila) {
/* Realiza a impressão dos itens de tipo 'T' na 'fila'. Isso não é 'TS',
 * porque ele modifica o objeto para realizar tal tarefa, então cuidado. */
   int total = fila.size();

   cout << '(' << fila.size() << ") " << '[';
   while (total-- > 0) {
      auto X = fila.front();

      fila.pop();
      cout << (int)X << ", ";
      fila.push(X);
   }
   cout << "\b\b" << ']' << endl;
}

template <typename T>
static void print_array(array<T, MAX_SERIAL>& seq) {
   cout << '(' << seq.size() << ") " << '[';
   for (T& item: seq) 
      cout << (int)item << ", ";
   cout << "\b\b" << ']' << endl;
}

void serializacao_e_deserializacao_simples(void) {
   Entrada a, b;

   cout << "pid_t: " << sizeof(pid_t) << " bytes\n";
   cout << a << endl << b << endl;

   auto bytes_a = a.serializa();
   auto bytes_b = b.serializa();

   print_array(bytes_a);
   print_array(bytes_b);

   auto A = Entrada::deserializa(bytes_a);
   auto B = Entrada::deserializa(bytes_b);

   cout << endl << A << endl << B << endl;
}

void no_decorrer_do_tempo(void) {
   using std::this_thread::sleep_for;
   using seg = chrono::seconds;

   Entrada x, y;

   for (int n = 1; n <= 17; n++) {
      ++x; ++y;
      cout << x << endl << y << endl;
      sleep_for(seg{1});
   }
}

int main(int qtd, char* args[], char* vars[]) 
{
   serializacao_e_deserializacao_simples();
   no_decorrer_do_tempo();
}
#endif
#endif
