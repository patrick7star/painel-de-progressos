#!/bin/python3 -OO
"""
   Cria um linque no repositório "oficial" de linque que o usuário define,
 que é $LINKS. Lá são colocados todos linques de aplicações CLI, que foram
 desenvolvidas por mim.
"""

from os import (getenv, environ as EnvironVars)
from pathlib import (Path)

IDENTIFICADOR = "LINKS"
NOME = "painel-progresso"
# Caminho ao executável em relação ao projeto.
if __debug__:
   EXECUTAVEL = Path("./bin/debug").resolve()
else:
   EXECUTAVEL = Path("./bin/release").resolve()


def cria_caminho_do_linque() -> Path:
   global NOME

   "Retorna caminho onde o linque será criado baseado na constante NOME."
   caminho = Path(getenv(IDENTIFICADOR))
   caminho = caminho.joinpath(NOME)

   return caminho

def verificacao_basica() -> None:
   if "LINKS" in EnvironVars:
      print("Variável 'LINKS' está definida.")
   else:
      if __debug__:
         print("Variável 'LINKS' não foi definda.")

   caminho = cria_caminho_do_linque()

   if (not caminho.exists(follow_symlinks=False)):
      print("O linque de '%s' não existe." % NOME)

def cria_linques() -> None:
   caminho = cria_caminho_do_linque()

   try:
      caminho.symlink_to(EXECUTAVEL)
   except FileExistsError:
      caminho.unlink()
      print("Removido linque já lá.")
      caminho.symlink_to(EXECUTAVEL)
   finally:
      assert (caminho.exists(follow_symlinks=False))
      print("Linque criado com sucesso.")

def cria_linque_do_link_em_ingles() -> None:
   NOME_EN = "panel-progress"
   BASE = getenv("LINKS")
   target = Path(BASE).joinpath(NOME)
   novo = Path(BASE).joinpath(NOME_EN)

   if target.exists():
      print("Como existe {}, criando algo dele ...".format(NOME))

      try:
         novo.symlink_to(target)
      except FileExistsError:
         print("Já existe um linque com o nome em Inglês.")
      else:
         print("Linque(en_US) criado com sucesso.")
      finally:
         pass
   else:
      print("Não existe um {}".format(NOME), file=sys.stderr)


print("Repositório dos linques: '{}'".format(getenv("LINKS")))
verificacao_basica()
cria_linques()
cria_linque_do_link_em_ingles()
