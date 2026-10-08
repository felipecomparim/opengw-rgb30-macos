# OpenGW no Powkiddy RGB30 / ROCKNIX

Port Linux ARM64 do código OpenGW r42, com SDL2 e GL4ES. Testado no RGB30
(RK3566 / Mali-G52), ROCKNIX 20260901, em 720×720.

## Instalação

Baixe `OpenGW-RGB30-ROCKNIX.zip` nas Releases do GitHub e extraia dentro de `/roms/ports/`.
No build local, o pacote fica em `dist/`. Saia do jogo antes de atualizar.
A estrutura deve ficar assim:

```
/roms/ports/OpenGW.sh
/roms/ports/opengw/opengw
/roms/ports/opengw/images/
/roms/ports/opengw/sounds/
/roms/ports/opengw/gl4es.aarch64/libGL.so.1
```

Abra **Ports → OpenGW**. Atualize a lista de jogos se o item ainda não aparecer.
Os erros e o resumo de desempenho ficam em `opengw/log.txt`.

## Controles

- Analógico esquerdo ou direcional: movimento / navegar no menu.
- Analógico direito: direção dos disparos.
- A/B/X/Y: alternativa para disparar nas quatro direções.
- A no menu: confirmar / iniciar; B no menu: voltar.
- L1/R1/L2/R2: bomba.
- Start: pausar.
- Controls no menu principal ou de pausa: trocar os analógicos e o teclado.
  Escolha `Move Right - Aim Left` para movimento no direito e mira no esquerdo.
  Tab/X nessa tela é um atalho para trocar os analógicos.
- Esquerda/direita nas opções Music Volume e Sound Volume da pausa: ajustar de 0 a 100%.
- Start + Select: sair para o menu.

O mapeamento segue SDL GameController e a base do PortMaster. Controles USB
podem ocupar os outros três jogadores. A conexão/desconexão é detectada em execução.

## Menu, recordes e dados

O código atual abre em um menu principal com New Game, High Scores, Controls e
Quit. Start durante a partida abre Resume, Controls, Music Volume, Sound Volume
 e Quit To Main Menu. Os volumes e controles são salvos em `opengw/controls.cfg`.

No fim de uma partida solo, uma pontuação que entra no top 10 abre a entrada
 de nome (até 12 caracteres). No gamepad: cima/baixo escolhem a letra,
esquerda/direita escolhem a posição, A salva e B cancela. O resultado é gravado
imediatamente em `opengw/highscores.txt`. Partidas cooperativas não entram nessa
 tabela. Quit To Main Menu abandona a partida, sem gravar seu resultado.

O pacote de publicação foi atualizado com esses recursos e passou nos testes
ARM64 sem janela. As últimas telas e opções ainda precisam de uma nova avaliação
manual no RGB30; o aparelho de desenvolvimento mantém a instalação anterior.
Preserve `controls.cfg`, `highscores.txt` e, para migração, `scores.sav` ao atualizar.

## Alterações

A entrada e a janela Win32 foram substituídas por SDL2; o projeto agora tem CMake.
Foram corrigidos includes incompatíveis com arquivos Linux, desalocações de arrays,
conversão de áudio e a projeção 2D. O dispositivo de áudio é aberto uma única vez.
Os objetos do jogo são construídos depois da inicialização de SDL, em ordem definida.

A lógica avança a 60 passos por segundo. Grade e partículas são atualizadas na mesma
thread da lógica, evitando acesso simultâneo aos seus arrays. O blur de CPU e a segunda
passagem de glow foram desativados para o RGB30; o restante do desenho vetorial e a
textura original do título são preservados.

As partículas consultam uma lista dos buracos negros ativos, calculada uma vez por
passo, e seus rastros são desenhados em até três lotes. A contagem dos segmentos da
grade foi corrigida para não desenhar vértices sem inicialização. O modo solo começa
com somente uma nave; os construtores não adicionam os outros jogadores.

A grade indexa as posições atuais dos pontos em células espaciais e aplica forças
somente aos pontos dentro do raio de cada distorção. As colisões e interações dos
tiros consultam uma lista de inimigos ativos, atualizada quando mudam de estado.
Isso elimina varreduras repetidas sobre todos os inimigos disponíveis no pool.

O launcher usa os governors `performance` dentro dos limites de frequência do
sistema durante o jogo e restaura os governors anteriores quando o jogo termina.

A biblioteca GL4ES 1.1.7 incluída foi obtida do port AREA2048 já instalado no aparelho.
Sua licença está em `opengw/gl4es.aarch64/LICENSE`.

## Compilação

Em Linux ARM64 com CMake, um compilador C++, SDL2 e OpenGL:

```sh
cmake -S . -B build/aarch64 -DCMAKE_BUILD_TYPE=Release
cmake --build build/aarch64 -j4
```

Também há um ambiente reproduzível em `tools/Dockerfile.aarch64`:

```sh
docker build --platform linux/arm64 -t opengw-rgb30-builder -f tools/Dockerfile.aarch64 .
docker run --rm --platform linux/arm64 -v "$PWD:/work" opengw-rgb30-builder \
  sh -c 'cmake -S . -B build/aarch64 -DCMAKE_BUILD_TYPE=Release && cmake --build build/aarch64 -j4'
```

Os assets são copiados para o diretório do executável. `OPENGW_DATA_DIR` permite
especificar outro diretório antes de iniciar o processo.

## Verificação

O teste sem janela usa os drivers dummy de SDL:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build/aarch64/opengw --self-test
```

Ele inicializa os sons, simula a apresentação, inicia uma partida com uma única
nave e executa 600 passos com movimento e disparos. Compara as consultas espaciais
da grade e as colisões dos inimigos ativos com as varreduras completas originais.
O teste passou nas compilações ARM64/Linux e macOS.

No RGB30, o teste de 600 frames com disparos contínuos e oito distorções móveis
passou de 27,1 para 46,4 FPS após otimizar a grade, e para **59,4 FPS** após otimizar
as consultas de inimigos (10,10 segundos). Os dois intervalos medidos naquela versão
deram 59,0 e 60,0 FPS. O teste usa os governors do launcher e mantém o
EmulationStation suspenso, como durante uma partida iniciada pelo menu.
Esses resultados medem esse cenário de dez segundos; não garantem 60 FPS em todas
as fases ou com qualquer quantidade de inimigos. O usuário confirmou que a versão otimizada medida ficou muito boa, quase perfeita,
jogando no aparelho após as otimizações.

```sh
/roms/ports/OpenGW.sh --start-game --frames 360
/roms/ports/OpenGW.sh --benchmark --frames 600
/roms/ports/OpenGW.sh --frames 30 --screenshot /tmp/opengw.bmp
```

Capturas acrescentam tempo de leitura da GPU; meça FPS sem `--screenshot`.
O controle integrado foi reconhecido e o modo solo foi testado no aparelho.
A configuração com quatro controles externos não foi testada.

## Correção da interface ROCKNIX

Durante a instalação, o Sway carregava EGL Mesa e usava o renderer pixman,
enquanto o EmulationStation falhava com `Could not initialize EGL`.
O menu foi recuperado usando as bibliotecas Mali do próprio sistema.

Foi salvo este override no aparelho:
`/storage/.config/system.d/sway.service.d/90-rgb30-mali.conf`.

```ini
[Service]
Environment=LD_LIBRARY_PATH=/usr/lib/mali
ExecStartPre=/storage/.config/system.d/wait-rgb30-mali.sh
```

O Sway confirmou EGL vendor ARM e renderer Mali-G52 após o reinício.
Esse ajuste é específico desta instalação ROCKNIX; o ZIP do jogo não modifica
os serviços do sistema. `tools/recover_rgb30_display.sh` aplica apenas o ajuste
temporário em `/run`, caso seja necessário diagnosticar novamente.

Após um reinício, foi identificado que o Sway iniciava antes de `/dev/mali0`
aparecer: o log indicava `No mali devices found` e caía no renderer pixman.
O script de espera acima aguarda até 30 segundos pelo dispositivo; se ele não
aparecer, a unidade falha e o systemd repete a tentativa, em vez de iniciar
a interface sem aceleração. `tools/fix_rgb30_boot_remote.sh` instala esse ajuste
específico no aparelho. A espera passou com a GPU disponível; a próxima
inicialização completa ainda precisa ser verificada.
