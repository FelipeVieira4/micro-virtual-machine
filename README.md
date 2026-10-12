# uVM-ESP32

Uma máquina virtual minimalista de 32 bits por instrução, escrita em C, pensada para rodar em um **ESP32** (FreeRTOS / ESP-IDF), mas que também compila e roda em PC para testes.

A VM carrega um programa em bytecode (`.bin`) e o executa instrução por instrução, com registradores, memória, flag de comparação, stack de retorno e syscalls.

> **Status:** em desenvolvimento. A VM e um assembler básico já funcionam; labels e as syscalls de hardware do ESP32 ainda estão sendo construídos.

## Características

- Instruções de largura fixa (4 bytes), fáceis de decodificar
- Registradores de uso geral, memória de dados em bytes e stack de retorno
- Saltos incondicionais e condicionais (via flag)
- `CALL` / `RETURN` com stack própria (estilo `GOSUB`/`RETURN` do BASIC)
- Syscalls para saída de dados e delay
- Validação em tempo de execução: registrador inválido, acesso fora da memória, divisão por zero, stack overflow/underflow, salto fora do programa e opcode desconhecido encerram a VM com uma mensagem de erro, sem travar o chip

## Estrutura do repositório

```
uVM-ESP32/
├── compiler/        # ferramentas para gerar o bytecode
├── utils/           # tipos auxiliares (ex.: mystdint.h)
├── main.c           # ponto de entrada
├── opcode_table.h   # definição dos opcodes
├── vm.c             # implementação da VM
├── vm.h             # estrutura vm_s e API pública
└── LICENSE          # GPL-3.0
```

## Formato da instrução

Cada instrução ocupa **32 bits**, gravados em **little-endian** no arquivo:

```
 31       24 23           12 11            0
┌───────────┬───────────────┬───────────────┐
│  opcode   │      c1       │      c2       │
│  (8 bits) │   (12 bits)   │   (12 bits)   │
└───────────┴───────────────┴───────────────┘
```

- O arquivo do programa precisa ter tamanho **múltiplo de 4** e não pode ser vazio.
- O `pc` é um endereço em bytes; os alvos de `JMP`/`CALL` são **índices de instrução** (o endereço real é `índice × 4`).
- Como `c1` e `c2` têm 12 bits, imediatos e alvos de salto vão de 0 a 4095.

## Conjunto de instruções

`R(x)` = registrador `x`. `MEM[x]` = byte `x` da memória.

### Registradores e memória

| Opcode | Efeito |
|--------|--------|
| `NOP` | Não faz nada |
| `MOV c1, c2` | `R(c1) = R(c2)` |
| `SET c1, imm` | `R(c1) = c2` (imediato de 12 bits) |
| `LOAD c1, c2` | `R(c1) = MEM[R(c2)]` |
| `STORE c1, c2` | `MEM[R(c1)] = R(c2) & 0xFF` |

### Aritmética

| Opcode | Efeito |
|--------|--------|
| `INC c1` | `R(c1)++` |
| `DEC c1` | `R(c1)--` |
| `ADD c1, c2` | `R(c1) += R(c2)` |
| `SUB c1, c2` | `R(c1) -= R(c2)` |
| `MUL c1, c2` | `R(c1) *= R(c2)` |
| `DIV c1, c2` | `R(c1) /= R(c2)` (erro se `R(c2) == 0`) |

### Comparações

Atualizam a flag da VM.

| Opcode | Efeito |
|--------|--------|
| `CMP c1, c2` | `flag = (R(c1) == R(c2))` |
| `GT c1, c2` | `flag = (R(c1) > R(c2))` |
| `LT c1, c2` | `flag = (R(c1) < R(c2))` |

### Controle de fluxo

| Opcode | Efeito |
|--------|--------|
| `JMP alvo, cond` | Salta para a instrução `alvo` conforme `cond` (tabela abaixo) |
| `CALL alvo` | Empilha o endereço da próxima instrução e salta para `alvo` |
| `RETURN` | Desempilha e volta para o endereço salvo |

Condições do `JMP` (campo `c2`):

| `c2` | Significado |
|------|-------------|
| `0` | Sempre |
| `1` | Se `flag` verdadeira |
| `2` | Se `flag` falsa |
| outro | Erro: condição de `JMP` inválida |

### Sistema

| Opcode | Efeito |
|--------|--------|
| `SYS_CALL num, reg` | Executa a syscall `num` com o valor de `R(reg)` |
| `SYS_EXIT` | Encerra a execução da VM |

Syscalls disponíveis:

| `num` | Nome | Comportamento |
|-------|------|---------------|
| `0` | print | Imprime `R(reg)` como número decimal, com quebra de linha |
| `1` | putchar | Imprime o byte baixo de `R(reg)` como caractere |
| `2` | delay | `vTaskDelay` de `R(reg)` ms no ESP32 (sem efeito no PC) |

> Os valores numéricos de cada opcode estão em [`opcode_table.h`](opcode_table.h).

## Exemplo

Conta de 0 a 9 e depois de volta até 6. **Labels ainda não são suportados**: os alvos de `JMP`/`CALL` são o **índice da instrução** (começando em 0).

```
SET R0, 0        ; 0: contador
SET R1, 10       ; 1: limite
SYS_CALL 0, R0   ; 2: imprime R0
INC R0           ; 3
LT R0, R1        ; 4: flag = R0 < 10
JMP 2, 1         ; 5: se flag, volta para a instrução 2

SET R1, 5        ; 6: novo limite
SYS_CALL 0, R0   ; 7: imprime R0
DEC R0           ; 8
GT R0, R1        ; 9: flag = R0 > 5
JMP 7, 1         ; 10: se flag, volta para a instrução 7

SYS_EXIT         ; 11
```

Dica: o índice de uma instrução é a posição dela contando só as instruções (linhas em branco e comentários não contam).

## Limitações conhecidas

- Alvos de `JMP`/`CALL` e imediatos limitados a 12 bits (máx. 4096 instruções por programa)
- `LOAD`/`STORE` operam em bytes
- Sem operadores bit a bit, `MOD` ou comparações com sinal
- Sem syscalls de GPIO/ADC ainda

## Roadmap

- [x] Assembler básico (texto → bytecode)
- [ ] Labels no assembler (`loop:` em vez de índices numéricos)
- [ ] Compilador de uma linguagem estilo BASIC
- [ ] Syscalls de GPIO, ADC e `millis`
- [ ] Instruções bit a bit, `MOD` e comparações `NE`/`LE`/`GE`
- [ ] Constantes de 32 bits
- [ ] Limite máximo de tamanho do programa (`MAX_PROGRAM_SIZE`)

## Licença

Distribuído sob a licença **GPL-3.0**. Veja o arquivo [LICENSE](LICENSE).
