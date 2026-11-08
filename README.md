# poi-26_2

- [`cpu-bound/`](cpu-bound/): aplicação backend CPU-bound (Proof-of-Work com SHA-256, em Java 21 + Spring Boot)
- [`io-bound/`](io-bound/): aplicação backend IO-bound (encurtador de URLs com PostgreSQL, em Java 21 + Spring Boot)
- [`memory-bound/`](memory-bound/): aplicação backend memory-bound (API de alunos com cadastro em memória e persistência em CSV, em C++17)

## Deploy

Com Docker Compose (as três aplicações + Postgres):

```bash
docker compose up -d --build
docker compose down -v
```

Portas: cpu-bound `8080`, memory-bound `8081`, io-bound `8082`. Para o modo escalado do memory-bound, use `MB_SCALE=5000000 docker compose up -d`.
