FROM python:3.9.0-slim

ENV APP_VERSION="5.0.4" \
    APP="platformio-core"

RUN pip install -U platformio==${APP_VERSION} && \
    mkdir -p /workspace && \
    mkdir -p /.platformio && \
    chmod a+rwx /.platformio

WORKDIR /project

