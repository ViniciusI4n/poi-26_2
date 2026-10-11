# Memory-bound: API de alunos em C++

Backend HTTP em C++17 que mantém um cadastro de alunos inteiramente em memória.
O CRUD lê e grava um arquivo CSV, e alguns endpoints varrem o conjunto de dados
inteiro, o que dá à aplicação um perfil **memory-bound**: o tempo de resposta e o
consumo de recursos dependem de quantos alunos cabem na RAM.

## Estrutura

```
memory-bound/
├── CMakeLists.txt
├── main.cpp                    # argumentos de linha de comando e inicialização
├── data/students.csv           # 100 alunos de exemplo
├── include/ e src/
│   ├── Student                 # entidade
│   ├── CsvLoader               # leitura, gravação e escala do CSV
│   ├── StudentRepository       # CRUD em memória e operações pesadas
│   └── StudentController       # rotas HTTP e JSON
└── third_party/                # httplib.h e json.hpp (só cabeçalhos)
```

## Requisitos

- g++ com suporte a C++17
- CMake 3.16 ou superior

Linux (Debian/Ubuntu):

```bash
sudo apt install build-essential cmake
```

Windows: MSYS2 (UCRT64) com `mingw-w64-ucrt-x86_64-gcc`, `mingw-w64-ucrt-x86_64-cmake`
e `mingw-w64-ucrt-x86_64-make`. Se o `cmake` não for encontrado, adicione ao PATH:

```bash
export PATH="/c/msys64/ucrt64/bin:$PATH"
```

## Compilar

Dentro da pasta `memory-bound`:

Linux:

```bash
cmake -S . -B build
cmake --build build
```

Windows (MSYS2):

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

O executável fica em `build/app` (Linux) ou `build/app.exe` (Windows).

## Executar

```bash
./build/app --port 8080 --csv data/students.csv
```

| Opção     | Padrão              | O que faz                                              |
|-----------|---------------------|--------------------------------------------------------|
| `--port`  | `8080`              | Porta do servidor                                      |
| `--csv`   | `data/students.csv` | Arquivo CSV de dados                                   |
| `--scale` | desligado           | Replica os alunos até chegar a N registros em memória  |

### Modo normal

Usa os 100 alunos do CSV. Todo `POST`, `PUT` e `DELETE` regrava o arquivo CSV.
Para não alterar o arquivo original durante os testes, trabalhe numa cópia:

```bash
cp data/students.csv /tmp/teste.csv
./build/app --port 8080 --csv /tmp/teste.csv
```

### Modo escalado (experimentos)

```bash
./build/app --port 8080 --scale 5000000
```

O CSV serve só de semente, e o servidor mantém 5 milhões de alunos na RAM
(cerca de 1 GB). Nesse modo as alterações ficam apenas na memória e o CSV **não** é
gravado. Referência medida: 3 milhões de alunos ocupam cerca de 560 MB e 5 milhões
cerca de 985 MB.

## Endpoints

Os exemplos usam `curl` e o servidor em `127.0.0.1:8080`.
No PowerShell use `curl.exe` e escape as aspas do JSON (`\"`).

Um aluno em JSON tem este formato:

```json
{
  "id": 1,
  "name": "Eduarda Oliveira",
  "course": "Physics",
  "age": 19,
  "grade1": 6.0,
  "grade2": 9.4,
  "grade3": 9.6,
  "attendance": 87.5,
  "average": 8.333333333333334
}
```

`average` é calculado pelo servidor. Em `name` e `course` não são aceitos valores
vazios, vírgulas nem quebras de linha (retorna 400).

### Saúde

```bash
curl http://127.0.0.1:8080/health
```

Resposta: `{"status":"ok","students":100}`

### Listar alunos (paginado)

```bash
curl "http://127.0.0.1:8080/students?offset=0&limit=5"
```

`offset` (padrão 0) e `limit` (padrão 20) são opcionais.

### Buscar por id

```bash
curl http://127.0.0.1:8080/students/1
```

Retorna 200 com o aluno, ou 404 se não existir.

### Criar aluno

```bash
curl -i -X POST http://127.0.0.1:8080/students \
  -d '{"name":"Maria Souza","course":"Law","age":22,"grade1":7,"grade2":8,"grade3":9,"attendance":90}'
```

Retorna 201 com o aluno criado (o `id` é definido pelo servidor). Corpo inválido ou
com campo faltando retorna 400.

### Atualizar aluno

```bash
curl -i -X PUT http://127.0.0.1:8080/students/1 \
  -d '{"name":"Maria Souza","course":"Law","age":23,"grade1":8,"grade2":8,"grade3":9,"attendance":92}'
```

Substitui todos os campos. Retorna 200 com o aluno atualizado, 404 se o id não
existir, ou 400 se o corpo for inválido.

### Remover aluno

```bash
curl -i -X DELETE http://127.0.0.1:8080/students/2
```

Retorna 204 sem corpo, ou 404 se não existir.

### Busca por nome (varre todos os alunos)

```bash
curl "http://127.0.0.1:8080/students/search?name=Ana&limit=3"
```

Procura o trecho `name` dentro do nome de cada aluno. `limit` é opcional (padrão 20).
Quando poucos alunos casam, a busca percorre a base inteira, que é o pior caso:

```bash
curl "http://127.0.0.1:8080/students/search?name=zzz"
```

### Estatísticas por curso (varre todos os alunos)

```bash
curl http://127.0.0.1:8080/students/stats
```

Devolve, para cada curso, a quantidade de alunos, a média das notas e a média de
frequência.

### Ranking por média (varre todos os alunos)

```bash
curl "http://127.0.0.1:8080/students/ranking?top=10"
```

Devolve os `top` alunos com maior média (padrão 10), do melhor para o pior.

## Verificar a gravação no CSV

No modo normal, depois de criar, atualizar ou remover alunos:

```bash
head -3 /tmp/teste.csv
tail -2 /tmp/teste.csv
```

## Observações para os experimentos

- As rotas `search`, `stats` e `ranking` percorrem todos os registros, e o custo
  delas cresce com o valor de `--scale`.
- As rotas de leitura simples (`/students/{id}`, `/health`) são baratas e servem de
  comparação.
- O consumo de memória depende quase só de `--scale`, o que facilita variar a RAM da
  VM e observar quando a aplicação começa a degradar (swap).
