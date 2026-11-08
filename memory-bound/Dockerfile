FROM debian:bookworm-slim AS build
WORKDIR /workspace

RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ cmake make \
 && rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt main.cpp ./
COPY include include
COPY src src
COPY third_party third_party
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)"

FROM debian:bookworm-slim
WORKDIR /app

RUN apt-get update \
 && apt-get install -y --no-install-recommends wget \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --uid 1001 app

COPY --from=build /workspace/build/app /app/app
COPY --chown=app:app data data
USER app

EXPOSE 8080

HEALTHCHECK --interval=10s --timeout=3s --start-period=10s --retries=3 \
  CMD wget -qO- http://localhost:8080/health || exit 1

ENTRYPOINT ["/app/app"]
CMD ["--port", "8080", "--csv", "data/students.csv"]
