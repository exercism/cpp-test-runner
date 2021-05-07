FROM alpine:3.11

RUN apk add --no-cache coreutils g++ libc-dev cmake make python3 git
RUN pip3 install --upgrade pip
RUN pip3 install junitparser

# Build Catch as a library directly inside the Docker image
# since it takes a bit of time to compile it,this way we speed
# compilation of tests when running the test suite.
RUN git clone https://github.com/catchorg/Catch2.git --depth 1 --branch v2.13.6 && \
    cd Catch2 && \
    cmake -Bbuild -H. -DCMAKE_CXX_STANDARD=17 -DCATCH_BUILD_STATIC_LIBRARY=ON -DBUILD_TESTING=OFF -DCATCH_INSTALL_DOCS=OFF && \
    cmake --build build/ --target install && \
    cd .. && \
    rm -rf Catch2

WORKDIR /opt/test-runner
COPY . .
ENTRYPOINT ["/opt/test-runner/bin/run.sh"]
