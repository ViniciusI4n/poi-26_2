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
│   ├── CourseCatalog           # cursos guardados como id inteiro (nome ↔ id)
│   ├── CsvLoader               # leitura, gravação e escala do CSV
│   ├── StudentRepository       # CRUD em memória e operações pesadas
│   ├── StudentController       # rotas HTTP e JSON
│   └── Metrics                 # métricas no formato Prometheus
├── load-test/students.js       # teste de carga (k6)
├── prometheus/prometheus.yml   # configuração de coleta do Prometheus
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
| `--history` | `32`              | Notas guardadas no histórico de cada aluno (8 bytes cada) |

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
(cerca de 2 GB). Nesse modo as alterações ficam apenas na memória e o CSV **não** é
gravado.

Cada aluno ocupa cerca de 150 bytes mais `8 × --history` bytes de histórico de notas.
O histórico é gerado a partir do id (não vai para o CSV) e só é lido pela rota
`/students/{id}/history`, então ele aumenta a RAM sem aumentar o trabalho de CPU das
outras rotas. Referência medida com o padrão `--history 32`:

| `--scale`  | RAM (RSS) |
|------------|-----------|
| 1 milhão   | ~395 MB   |
| 5 milhões  | ~1,9 GB   |
| 10 milhões | ~3,9 GB   |

Com `--history 0`, 10 milhões de alunos ocupam ~1,5 GB.

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

### Histórico de notas

```bash
curl http://127.0.0.1:8080/students/1/history
```

Resposta: `{"id":1,"history":[7.0,9.0,9.1,...]}`, com `--history` notas. Retorna 404
se o aluno não existir.

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

### Métricas (Prometheus)

```bash
curl http://127.0.0.1:8080/metrics
```

Devolve as métricas no formato de texto do Prometheus:

| Métrica                                | Tipo      | O que mede                                                       |
|----------------------------------------|-----------|------------------------------------------------------------------|
| `http_server_requests_seconds`         | histogram | Duração das requisições, por `method`, `route` e `status`        |
| `students_in_memory`                   | gauge     | Quantidade de alunos mantidos na RAM                             |
| `students_history_bytes`               | gauge     | Bytes ocupados pelo histórico de notas                           |
| `process_resident_memory_bytes`        | gauge     | Memória residente (RSS) do processo (só Linux)                   |
| `process_virtual_memory_bytes`         | gauge     | Memória virtual do processo (só Linux)                           |
| `process_cpu_seconds_total`            | counter   | Tempo de CPU (usuário + sistema) consumido (só Linux)            |
| `process_start_time_seconds`           | gauge     | Horário de início do processo (epoch, em segundos)               |

Para coletar com um Prometheus local, com o servidor rodando na porta 8080:

```bash
docker run --rm --network host \
  -v "$PWD/prometheus/prometheus.yml:/etc/prometheus/prometheus.yml:ro" \
  prom/prometheus
```

A interface fica em `http://localhost:9090`. Exemplos de consulta:

```promql
process_resident_memory_bytes{job="memory-bound"}
histogram_quantile(0.95, sum by (le, route) (rate(http_server_requests_seconds_bucket[1m])))
```

## Teste de carga (k6)

Com o servidor rodando em modo escalado:

```bash
docker run --rm -i --network host -e BASE_URL=http://localhost:8080 \
  grafana/k6 run - < load-test/students.js
```

| Variável     | Padrão | O que faz                                                         |
|--------------|--------|-------------------------------------------------------------------|
| `RATE`       | `1000` | Requisições por segundo (taxa de chegada fixa)                    |
| `DURATION`   | `1m`   | Duração do patamar, entre rampas de 30 s                          |
| `SCAN_RATIO` | `0`    | Fração das requisições que varrem a base (`search`/`stats`/`ranking`) |

A mistura padrão é de acessos pontuais (50% aluno, 30% histórico, 20% página), e a
taxa é fixa: o objetivo é manter a RAM ocupada e a CPU ociosa. Referência medida com
10 milhões de alunos (3,8 GB), `RATE=1000`, numa máquina de 16 núcleos:

| `SCAN_RATIO` | CPU da aplicação     | p95 (aluno/histórico) |
|--------------|----------------------|-----------------------|
| `0`          | 0,07 núcleo (0,4%)   | ~0,12 ms              |
| `0.01`       | 0,57 núcleo (3,5%)   | ~0,13 ms              |
| `0.05`       | 1,2 núcleo (7,2%)    | ~1,4 ms, com timeouts |

Com `SCAN_RATIO=0.05` aparecem timeouts: o servidor atende no máximo `núcleos − 1`
conexões ao mesmo tempo (pool de threads do `httplib`), e as varreduras lentas
prendem essas threads.

## Verificar a gravação no CSV

No modo normal, depois de criar, atualizar ou remover alunos:

```bash
head -3 /tmp/teste.csv
tail -2 /tmp/teste.csv
```

## Observações para os experimentos

- As rotas `search`, `stats` e `ranking` percorrem todos os registros, e o custo
  delas cresce com o valor de `--scale`.
- As rotas de leitura simples (`/students/{id}`, `/students/{id}/history`, `/health`)
  são baratas e servem de comparação.
- O consumo de memória depende de `--scale` e `--history`, o que facilita variar a RAM
  da VM e observar quando a aplicação começa a degradar (swap).
- Uma varredura limitada pela memória deixa o núcleo esperando a DRAM, mas o sistema
  operacional conta essa espera como uso de CPU (`top`, `process_cpu_seconds_total`).
  Para ver RAM alta com CPU ociosa, use a mistura padrão do k6 (`SCAN_RATIO=0`).
