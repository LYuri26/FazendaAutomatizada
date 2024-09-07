#ifndef TELA_H
#define TELA_H

// Funções para exibir informações na tela TFT
void inicializarTela();
void atualizarTela();
void exibirHoraAtual(const String &hora);
void exibirInformacoesRede();
void exibirEstadoLuzes(bool luzCasa, bool luzRua, bool luzPasto);
int converterParaMinutos(String hora);
bool isEntreAmanhecerEPorDoSol(String horaAtual, String nascerDoSol, String porDoSol);

#endif
