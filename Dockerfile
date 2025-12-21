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
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --target BookNest -j

# ---- Runtime stage ----
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    tzdata \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Create runtime directories for volumes
RUN mkdir -p /app/data /app/import

# Copy the built binary
COPY --from=builder /src/cmake-build-release/BookNest /app/BookNest

# Default command: interactive console app
ENTRYPOINT ["/app/BookNest"]
