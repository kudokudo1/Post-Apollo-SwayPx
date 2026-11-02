# CI-ONLY; disposable real Chromium process tree, no production image.
# Debian's chromium package retains its own sandbox; do not use --no-sandbox.
FROM debian:bookworm-slim
RUN apt-get update -qq \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y -qq --no-install-recommends \
    chromium chromium-sandbox procps ca-certificates \
 && rm -rf /var/lib/apt/lists/*
RUN useradd --uid 10001 --create-home --home-dir /home/stage-watch --shell /usr/sbin/nologin stage-watch
USER 10001:10001
WORKDIR /home/stage-watch
ENV HOME=/home/stage-watch
ENTRYPOINT ["/usr/bin/chromium"]
CMD ["--headless=new","--disable-gpu","--no-first-run","--disable-background-networking","--disable-component-update","--disable-sync","--disable-extensions","--disable-default-apps","--disable-features=MediaRouter","--no-default-browser-check","--user-data-dir=/tmp/stage-director-browser-profile","about:blank"]
