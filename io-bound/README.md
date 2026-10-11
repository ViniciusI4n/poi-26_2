# io-bound

Aplicação backend **IO-bound** para a disciplina de Provisionamento e Operação de Infraestrutura.

É um **encurtador de URLs** com PostgreSQL. Toda requisição vai ao banco,
então o tempo de resposta é dominado pela **espera de IO**.

## Rodando

Com Docker Compose (app + Postgres):

```bash
docker compose up -d --build
docker compose down -v
```

Localmente:

```bash
sdk env
./gradlew build
./gradlew bootRun
```

## Endpoints

| Método | Rota | Descrição |
|---|---|---|
| `POST` | `/api/links` | Corpo `{"url":"https://..."}` → **201** `{code, url, shortUrl, clicks, createdAt}` |
| `GET` | `/{code}` | **302** para a URL original (incrementa `clicks`), ou **404** |
| `GET` | `/api/links/{code}` | Dados e contagem de cliques do link |
| `GET` | `/api/links?page=0&size=20` | Listagem paginada (`size` ≤ 100) |
| `GET` | `/actuator/health` (`/liveness`, `/readiness`) | Probes. A readiness inclui o banco |
| `GET` | `/actuator/prometheus` | Métricas (HTTP, JVM, **pool de conexões Hikari**) |

```bash
curl -X POST localhost:8082/api/links -H 'Content-Type: application/json' -d '{"url":"https://www.ufcg.edu.br"}'
curl -i localhost:8082/<code>
```

## Demonstração IO-bound

```bash
docker compose up -d --build
docker run --rm -i --network host -e BASE_URL=http://localhost:8082 \
  grafana/k6 run --vus 50 --duration 30s - < load-test/shortener.js
```
