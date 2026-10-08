# OpenGW — RGB30 / ROCKNIX e macOS Apple Silicon

Port do **Open Geometry Wars (OpenGW r42)** para o Powkiddy RGB30 com ROCKNIX e
para Macs com Apple Silicon, incluindo M4. É um jogo de ação com movimento e
mira independentes, grade que se deforma, partículas, bombas e modo cooperativo.

O projeto original foi criado por **Peter Hirschberg** e disponibilizado no
[SourceForge](https://sourceforge.net/projects/opengw/). Este repositório adapta
a janela, entrada, áudio, build e desempenho para as duas plataformas.

## Downloads

Baixe os pacotes em [Releases](https://github.com/felipecomparim/opengw-rgb30-macos/releases).
Não é necessário compilar para jogar.

| Pacote | Plataforma | Conteúdo |
| --- | --- | --- |
| `OpenGW-RGB30-ROCKNIX.zip` | Powkiddy RGB30, Linux ARM64 / ROCKNIX | Launcher PortMaster, executável, GL4ES, imagens e sons |
| `OpenGW-macOS-AppleSilicon.zip` | Mac ARM64, macOS 26 ou posterior | `OpenGW.app`, bibliotecas SDL, imagens e sons |
| `SHA256SUMS` | Ambas | Checksums SHA-256 dos pacotes |

**Estado dos testes:** a versão macOS foi aberta no Mac M4 com macOS 26.4.
O port RGB30 foi jogado no ROCKNIX 20260901 e teve suas otimizações de desempenho
confirmadas no aparelho. O pacote RGB30 atual inclui os menus, recordes e áudio
mais recentes e passou nos testes ARM64 sem janela; essas últimas telas ainda
precisam de uma nova avaliação manual no RGB30. A instalação já existente no
aparelho usado no desenvolvimento continua sendo a versão anterior.

## Instalar no RGB30

1. Tenha o **PortMaster** instalado no ROCKNIX.
2. Extraia `OpenGW-RGB30-ROCKNIX.zip` em `/roms/ports/`.
3. Abra **Ports → OpenGW** no EmulationStation. Atualize a lista de jogos se necessário.

A estrutura deve ser:

```text
/roms/ports/OpenGW.sh
/roms/ports/opengw/opengw
/roms/ports/opengw/gl4es.aarch64/libGL.so.1
/roms/ports/opengw/images/
/roms/ports/opengw/sounds/
```

Ao atualizar, saia do jogo antes de extrair o pacote. Seus arquivos `controls.cfg`
e `highscores.txt` não fazem parte do ZIP e podem ser preservados na pasta `opengw`.
Não é necessário configurar SSH para instalar pelo cartão SD ou pela rede.

O launcher usa temporariamente os governors `performance` nas frequências normais
do aparelho e restaura os anteriores na saída. O jogo não modifica os serviços
do ROCKNIX. O log fica em `/roms/ports/opengw/log.txt`.

[Detalhes do RGB30, compilação e diagnóstico](README-RGB30.md).

## Instalar no Mac M4 / Apple Silicon

1. Extraia `OpenGW-macOS-AppleSilicon.zip`.
2. Arraste **OpenGW.app** para **Applications / Aplicativos**.
3. Abra o aplicativo com duplo clique.

O pacote é nativo **ARM64** e inclui SDL2-compat e SDL3; não requer Homebrew nem
Rosetta para jogar. Abre em uma janela redimensionável de 720×720. O botão verde
da janela permite entrar em tela cheia; para iniciar diretamente em tela cheia:

```sh
/Applications/OpenGW.app/Contents/MacOS/opengw --fullscreen
```

A assinatura é ad hoc, sem Developer ID/notarização. Se o macOS bloquear uma cópia
baixada, use **Ajustes do Sistema → Privacidade e Segurança → Abrir Mesmo Assim**.

[Detalhes do macOS, controles e empacotamento](README-macOS.md).

## Controles

### RGB30 / gamepad

| Controle | Ação |
| --- | --- |
| Analógico esquerdo / direcional | Mover a nave / navegar no menu |
| Analógico direito | Mirar e disparar |
| A/B/X/Y durante a partida | Disparos nas quatro direções |
| A / B nos menus | Confirmar / voltar |
| L1/R1/L2/R2 | Bomba |
| Start | Abrir / fechar a pausa |
| Start + Select no RGB30 | Sair do jogo |

Em **Controls**, os analógicos podem ser trocados: **Move Right - Aim Left** usa
movimento na direita e mira na esquerda. A preferência fica salva.

### Teclado do Mac

| Tecla | Ação |
| --- | --- |
| Setas | Mover a nave / navegar no menu |
| WASD | Mirar e disparar |
| Espaço | Bomba; iniciar cooperativo na seleção de partida |
| Enter | Confirmar / iniciar partida / salvar nome |
| Esc durante a partida | Abrir o menu de pausa |
| Esc nos submenus | Voltar; no menu de pausa, continuar |
| P | Alternativa para pausar / continuar |
| Cmd+Q | Sair do aplicativo |

O teclado também pode ser invertido em **Controls**: WASD move e as setas miram.
Esse ajuste é independente da troca dos analógicos. No Mac, **Esc não fecha o app**;
saia com **Cmd+Q** ou **Quit** no menu principal.

## Menus, pausa e recordes

O menu principal tem **New Game**, **High Scores**, **Controls** e **Quit**.
Em New Game, selecione solo ou cooperativo. A base original suporta até quatro
jogadores locais; a configuração com quatro controles externos não foi testada.

A pausa tem **Resume**, **Controls**, **Music Volume**, **Sound Volume** e
**Quit To Main Menu**. Use cima/baixo para selecionar, esquerda/direita para
ajustar volumes de 0 a 100%, e Enter/A para confirmar. Música e efeitos têm
volumes independentes, salvos automaticamente. A música continua durante a pausa
para ouvir o ajuste. Quit To Main Menu abandona a partida atual sem gravar seu
resultado no ranking.

Ao terminar uma partida **solo**, uma pontuação no **top 10** abre a entrada de
nome. Digite até 12 caracteres (letras, números e espaços) e pressione Enter.
No gamepad, cima/baixo escolhem a letra, esquerda/direita escolhem a posição e
A salva. Após salvar, a tabela aparece. O ranking persiste ao reabrir o jogo.

| Dados | RGB30 | macOS |
| --- | --- | --- |
| Controles e volumes | `/roms/ports/opengw/controls.cfg` | `~/Library/Application Support/OpenGW/OpenGW/controls.cfg` |
| Recordes solo | `/roms/ports/opengw/highscores.txt` | `~/Library/Application Support/OpenGW/OpenGW/highscores.txt` |

A tabela antiga em `scores.sav` é importada quando ainda não existe `highscores.txt`.

## Desempenho no RGB30

O port usa lógica a 60 passos/s, lotes de partículas, uma grade com consultas
espaciais e uma lista de inimigos ativos para colisões. O blur de CPU e a segunda
passagem de glow foram desativados. Isso reduziu o slowdown causado por tiros e
pela deformação da grade.

No teste realizado no RGB30 com disparos contínuos e oito distorções móveis,
600 frames levaram 10,10 segundos: **59,4 FPS**. É uma medição da versão otimizada
anterior aos últimos menus, em um cenário de cerca de dez segundos, e não uma
garantia de 60 FPS em toda fase. Capturas de tela não devem ser usadas para medir FPS.

## Compilar a partir do código

### macOS ARM64

Instale as ferramentas de desenvolvimento Apple, CMake e as bibliotecas SDL:

```sh
xcode-select --install
brew install cmake sdl2-compat sdl3
python3 tools/package_macos.py
```

O script gera `dist/OpenGW.app` e `dist/OpenGW-macOS-AppleSilicon.zip`, incluindo
bibliotecas, ícone e assinatura local. O script de empacotamento usa os caminhos
padrão do Homebrew em Macs ARM64.

### Linux ARM64 / RGB30

Com CMake, g++, SDL2 e OpenGL instalados:

```sh
cmake -S . -B build/aarch64 -DCMAKE_BUILD_TYPE=Release
cmake --build build/aarch64 -j4
```

Ou use o container Debian Bookworm:

```sh
docker build --platform linux/arm64 -t opengw-rgb30-builder -f tools/Dockerfile.aarch64 .
docker run --rm --platform linux/arm64 -v "$PWD:/work" opengw-rgb30-builder \
  sh -c 'cmake -S . -B build/aarch64 -DCMAKE_BUILD_TYPE=Release && cmake --build build/aarch64 -j4'
```

O executável depende de SDL2 do sistema e do GL4ES incluído no pacote para fornecer
OpenGL sobre GLES. O código e o launcher estão neste repositório; builds e ZIPs
gerados ficam fora do histórico Git e são distribuídos nas Releases.

## Verificação

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build/aarch64/opengw --self-test
```

No Mac, substitua o caminho por `dist/OpenGW.app/Contents/MacOS/opengw`.
O teste verifica áudio, partida solo, forças da grade e colisões comparadas com
varreduras completas, controles SDL virtuais, pausa, volumes independentes,
persistência de configurações, nomes e ranking, e transições de fim de partida.

```sh
/roms/ports/OpenGW.sh --benchmark --frames 600
/Applications/OpenGW.app/Contents/MacOS/opengw --benchmark --frames 600
```

## Créditos e licença

- **OpenGW original:** Peter Hirschberg, [SourceForge](https://sourceforge.net/projects/opengw/), base r42.
- **Adaptação RGB30/macOS:** Felipe Correa / [felipecomparim](https://github.com/felipecomparim).
- **SDL2, SDL2-compat e SDL3:** bibliotecas de janela, entrada e áudio.
- **GL4ES:** tradução das chamadas OpenGL para GLES no RGB30.
- **LodePNG e GLEW:** componentes do código original, com seus avisos preservados.

O projeto original declara **GPLv3** na página do SourceForge. A licença está em
[COPYING](COPYING); a descrição das licenças de terceiros está em [LICENSE.md](LICENSE.md).
