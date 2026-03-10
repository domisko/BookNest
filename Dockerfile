## Multi-stage Dockerfile for BookNest (C++17, console app)

# ---- Builder stage ----
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy project files
COPY . /src

# Configure & build Release
# WICHTIG: Wir nutzen den Ordner "build" konsistent
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --target BookNest -j $(nproc)

# ---- Runtime stage ----
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    tzdata \
    libstdc++6 \
 && rm -rf /var/lib/apt/lists/*

RUN groupadd -r booknest && useradd -r -g booknest booknest

WORKDIR /app

# Erstelle die notwendigen Verzeichnisse
RUN mkdir -p /app/data /app/import && chown -R booknest:booknest /app

# Kopiere die ausführbare Datei aus dem Builder
COPY --from=builder /src/build/BookNest /app/BookNest

# WICHTIG: Kopiere den INHALT des lokalen import-Ordners in den /app/import Ordner im Container
COPY import/ /app/import/

USER booknest

# Default command
ENTRYPOINT ["/app/BookNest"]