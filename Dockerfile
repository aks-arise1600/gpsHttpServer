# ------------------------------
# Base image
# ------------------------------
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# ------------------------------
# Enable universe repository (important)
# ------------------------------
RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository universe

# Install Qt 6 + dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    openssl \
    libssl-dev \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-httpserver-dev \
    qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*

# Verify required Qt modules
RUN ls /usr/lib/*/cmake/Qt6HttpServer
RUN ls /usr/lib/*/cmake/Qt6WebSockets

# ------------------------------
# Working directory
# ------------------------------
WORKDIR /app

# ------------------------------
# Copy source
# ------------------------------
COPY . .

# Build your app
RUN mkdir build && cd build && cmake .. && make

# ------------------------------
# Expose HTTP port
# ------------------------------
EXPOSE 8080

# ------------------------------
# Run server
# ------------------------------
CMD ["./build/gpsHttpServerApp"]

