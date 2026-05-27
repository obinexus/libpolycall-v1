# Build stage
FROM gcc:12-bookworm AS builder

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . .

# Build the library and executable
RUN make release

# Runtime stage
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy built artifacts from builder
COPY --from=builder /app/lib /usr/local/lib
COPY --from=builder /app/bin /usr/local/bin
COPY --from=builder /app/include /usr/local/include

# Run ldconfig to update library cache
RUN ldconfig

# Copy default config
COPY config.Polycallfile /etc/polycall/default.conf

# Set the entrypoint to the polycall executable
ENTRYPOINT ["polycall"]

# Default: run interactive mode; override with -f <config> for non-interactive
CMD []
