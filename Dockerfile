FROM alpine:3.11

RUN apk add --no-cache coreutils g++ libc-dev cmake make python3
RUN pip3 install --upgrade pip
RUN pip3 install junitparser

WORKDIR /opt/test-runner
COPY . .
ENTRYPOINT ["/opt/test-runner/bin/run.sh"]
