# Grupos de cor (k-means)

k-means com 12 grupos na cor Lab das 102370 peças de dados/pecas_v4.csv, semente 42, início k-means++, parou em 184 rodadas. Os looks não entram nesta conta.

Escala: L/100 e a/100, b/100 (o a e o b do CSV vezes 2,55). Assim a distância é o delta E dividido por 100. O nome é a cor de referência mais perto do centro. Nome repetido ganha número, 1 para o grupo mais escuro.

| grupo | peças | % | L | a | b | distância média ao centro (delta E) |
| --- | --- | --- | --- | --- | --- | --- |
| preto | 26384 | 25,8 | 11,7 | 1,4 | -0,7 | 6,0 |
| branco | 24096 | 23,5 | 88,2 | 0,6 | 2,8 | 6,9 |
| bege | 11400 | 11,1 | 76,1 | 5,0 | 17,6 | 10,3 |
| cinza 2 | 8592 | 8,4 | 61,0 | 0,1 | -4,0 | 12,5 |
| cinza 1 | 8017 | 7,8 | 31,0 | 0,4 | 1,5 | 11,3 |
| marrom | 6990 | 6,8 | 54,9 | 13,2 | 25,4 | 13,3 |
| vermelho | 4440 | 4,3 | 46,8 | 55,1 | 40,7 | 16,8 |
| vinho | 4066 | 4,0 | 25,4 | 25,0 | 13,3 | 14,1 |
| marinho | 2811 | 2,7 | 30,3 | 13,4 | -32,0 | 18,4 |
| amarelo | 2464 | 2,4 | 78,4 | -0,8 | 62,5 | 19,0 |
| rosa | 1679 | 1,6 | 53,4 | 54,2 | 4,0 | 18,7 |
| verde | 1431 | 1,4 | 60,5 | -29,9 | 7,4 | 21,4 |

Cores de referência (RGB):

- preto: (20, 20, 20)
- branco: (245, 245, 245)
- cinza: (128, 128, 128)
- bege: (215, 195, 160)
- marrom: (110, 70, 40)
- jeans: (75, 105, 145)
- marinho: (25, 35, 70)
- vermelho: (190, 30, 40)
- vinho: (110, 25, 45)
- rosa: (230, 160, 180)
- verde: (60, 120, 60)
- amarelo: (235, 205, 60)
- laranja: (230, 120, 40)
- roxo: (110, 60, 130)
