# cpu-bound

Aplicação backend **CPU-bound** para a disciplina de Provisionamento e Operação de Infraestrutura.

Implementa um serviço de **Proof-of-Work**: dado um texto `data` e uma
`difficulty` N, busca por força bruta um `nonce` tal que `SHA-256(data + nonce)` comece com N dígitos
hexadecimais `0`.

## Rodando

```bash
sdk env
./gradlew build
./gradlew bootRun
```

Com Docker:

```bash
docker build -t pow-api .
docker run --rm -p 8080:8080 --cpus=1 --memory=512m pow-api
```

## Endpoints

| Método | Rota | Descrição |
|---|---|---|
| `POST` | `/api/pow` | Corpo: `{"data":"hello","difficulty":5}` |
| `GET` | `/api/pow/verify?data=hello&nonce=123&difficulty=5` | Calcula um único hash e diz se a solução é válida |
| `GET` | `/actuator/health` (`/liveness`, `/readiness`) | Health checks / probes |
| `GET` | `/actuator/prometheus` | Métricas no formato Prometheus |

Exemplo:

```bash
curl -X POST localhost:8080/api/pow -H 'Content-Type: application/json' \
     -d '{"data":"hello","difficulty":6}'
```

## Teste de carga (k6)

```bash
docker run --rm -i --network host -e BASE_URL=http://localhost:8080 -e DIFFICULTY=5 \
  grafana/k6 run - < load-test/pow.js
```
