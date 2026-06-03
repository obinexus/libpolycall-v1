# syntax=docker/dockerfile:1.7

ARG POLYCALL_VERSION=1.0.0

FROM gcc:12-bookworm AS builder

WORKDIR /src

COPY Makefile Polycallfile ./
COPY include ./include
COPY src ./src

RUN make clean && \
    make static && \
    strip bin/polycall

FROM scratch AS runtime

ARG POLYCALL_VERSION

LABEL org.opencontainers.image.title="LibPolyCall" \
      org.opencontainers.image.description="Minimal package-free LibPolyCall v1 runtime image" \
      org.opencontainers.image.version="${POLYCALL_VERSION}" \
      org.opencontainers.image.vendor="OBINexus" \
      org.opencontainers.image.licenses="MIT"

WORKDIR /app

COPY --from=builder --chmod=0555 /src/bin/polycall /usr/local/bin/polycall
COPY --from=builder --chmod=0444 /src/lib/libpolycall.a /usr/local/lib/libpolycall.a
COPY --from=builder /src/include /usr/local/include/libpolycall
COPY --from=builder --chmod=0444 /src/Polycallfile /etc/polycall/default.conf

USER 65532:65532

ENTRYPOINT ["/usr/local/bin/polycall"]
CMD []
