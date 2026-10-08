Primeira publicação dos ports OpenGW r42 para Powkiddy RGB30 / ROCKNIX e macOS Apple Silicon (M4).

Inclui menu principal, tabela de recordes solo com nomes de até 12 caracteres,
menu de pausa, troca independente do teclado/analógicos e volumes separados de
música e efeitos, com persistência das preferências e do ranking.

Pacotes:
- OpenGW-macOS-AppleSilicon.zip: OpenGW.app nativo ARM64, macOS 26+, com SDL e assets incluídos.
- OpenGW-RGB30-ROCKNIX.zip: launcher PortMaster, binário ARM64, GL4ES e assets.
- SHA256SUMS: checksums dos dois ZIPs.

Instalação e controles estão no README principal e nos guias de cada plataforma.

Validação: testes de simulação, colisões, grade, áudio, menus, pausa, controles,
volumes e persistência passaram em macOS ARM64 e Linux ARM64. O port original
foi jogado no RGB30 com melhora de desempenho confirmada; as últimas telas dos
menus e opções do pacote RGB30 atual ainda precisam de nova avaliação manual.
A instalação existente no aparelho de desenvolvimento não foi alterada nesta publicação.
