# OpenGW para Mac com Apple Silicon

Aplicativo nativo ARM64, preparado e testado em um Mac M4 com macOS 26.4.
Inclui SDL2-compat, SDL3, imagens e sons; não requer Homebrew nem Rosetta.

Abra `dist/OpenGW.app` com duplo clique. O ZIP para copiar para outro Mac é
`dist/OpenGW-macOS-AppleSilicon.zip`; extraia e abra `OpenGW.app`.
O aplicativo abre em uma janela de 720×720, que pode ser redimensionada.

## Menu principal e recordes

O aplicativo abre no menu principal: **New Game**, **High Scores**, **Controls**
e **Quit**. Use as setas e Enter (ou direcional/analógico e A no gamepad).

Ao terminar uma partida solo, uma pontuação que entre no top 10 abre a tela
**New High Score**. Digite seu nome com até 12 caracteres (letras, números e
espaços), use Backspace para corrigir e Enter para salvar. No gamepad, cima/baixo
escolhem a letra, esquerda/direita escolhem a posição e A salva. Esc/B cancela.
Após salvar, a tabela aparece; Enter/Esc/A/B retornam ao menu principal.

Os recordes são gravados imediatamente em `highscores.txt` e ficam disponíveis
mesmo após fechar e reabrir o jogo. A tabela é exclusiva das partidas solo;
partidas cooperativas retornam ao menu principal. Recordes antigos em
`scores.sav` são importados quando ainda não existe a nova tabela.

## Controles

- Enter: confirmar a opção do menu / iniciar uma partida solo.
- Setas: movimento / navegar no menu.
- WASD: direção dos disparos.
- Espaço: bomba; no modo cooperativo, iniciar a partida.
- Esc: abrir o menu de pausa durante a partida; pressionar novamente continua.
- P: alternativa para pausar ou continuar.
- Backspace: voltar no menu.
- Tab na seleção de partida ou durante a pausa: trocar os analógicos do gamepad.
- Esc: voltar nas telas de recordes e controles; não fecha o aplicativo.
- Cmd+Q: sair do aplicativo; também há Quit no menu principal.

Gamepad: analógico esquerdo move, direito mira; A confirma, B volta,
gatilhos ou ombros usam bomba, Start pausa, Start + Select sai.
X no menu ou durante a pausa troca os analógicos. Escolha
`Move Right - Aim Left` para movimento na direita e mira na esquerda.
O teclado começa com setas para movimento e WASD para mira. Em Controls,
é possível trocar para WASD mover e setas mirar, independentemente do gamepad.

As preferências e os recordes são salvos em
`~/Library/Application Support/OpenGW/OpenGW/`, fora do aplicativo.

## Menu de pausa e áudio

Esc durante a partida abre **Resume**, **Controls**, **Music Volume**,
**Sound Volume** e **Quit To Main Menu**. Use cima/baixo para selecionar,
esquerda/direita para ajustar os volumes de 0 a 100%, e Enter/A para confirmar.
A música continua durante a pausa para ouvir os ajustes; efeitos de jogo ficam
pausados e os sons do menu permitem conferir o volume de efeitos.

Controls permite trocar o teclado e os analógicos separadamente. Esc/B nessa
tela retorna ao menu de pausa; Esc no menu de pausa continua a partida.
P ou Start também alternam a pausa. As preferências são salvas automaticamente.
Quit To Main Menu abandona a partida atual e retorna ao menu, sem fechar o app
nem registrar aquela partida interrompida no ranking. Cmd+Q ou Quit no menu
principal fecham o aplicativo. Start+Select não fecha a versão do Mac.

## Compilar e empacotar

Com CMake, ferramentas de desenvolvimento Apple e SDL2-compat/SDL3 disponíveis:

```sh
python3 tools/package_macos.py
```

O script cria o bundle, inclui as bibliotecas, corrige os caminhos de carregamento,
gera o ícone original do jogo e aplica uma assinatura ad hoc local.
A versão exige macOS 26 ou posterior. Não possui assinatura Developer ID nem
notarização Apple; em outro Mac, uma cópia baixada pode precisar ser liberada em
Ajustes do Sistema → Privacidade e Segurança → Abrir Mesmo Assim.

Para executar verificações ou escolher tela cheia:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dist/OpenGW.app/Contents/MacOS/opengw --self-test
dist/OpenGW.app/Contents/MacOS/opengw --fullscreen
dist/OpenGW.app/Contents/MacOS/opengw --benchmark --frames 600
```

## Verificações realizadas

O bundle ARM64 passou na verificação de assinatura local e o ZIP foi validado.
As dependências do executável usam o diretório Frameworks do aplicativo e os
frameworks do macOS. O SDL3 é incluído junto ao SDL2-compat para carregamento
dinâmico. O teste de simulação, áudio, colisões, forças da grade, troca dos
analógicos e persistência passou usando o executável do bundle.
A janela com o título original do jogo foi aberta e conferida visualmente neste
Mac. Ainda não foi feita uma medição de FPS durante uma partida manual no Mac.

Também passaram os testes de entrada e correção do nome, ordenação do top 10,
reabertura do arquivo salvo, falha de gravação sem perda do resultado pendente,
navegação pelo menu e transição real de fim de partida para os recordes.

Passaram também os testes de congelamento da partida na pausa, navegação de
controles, retomada, retorno ao menu principal, Cmd+Q, limites de volume,
ganhos independentes de música/efeitos e persistência de todas as preferências.
