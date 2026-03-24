set windows-shell := ["pwsh.exe", "-NoLogo", "-c"]

alias b := build
alias br := build-release
alias ba := build-all
alias bm := benchmark
alias c := clean
alias t := test
alias tr := test-release
alias ta := test-all
alias tc := test-client
alias ts := test-server
alias pc := performance-client
alias ps := performance-server

buildDir := if os() == "windows" {
    "tmpwin"
} else {
    "tmplin"
}

build:
    cmake . -B ./{{buildDir}}/debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DDSVEOSCOSIM_BUILD_TESTS=ON
    cmake --build ./{{buildDir}}/debug --config Debug

build-release:
    cmake . -B ./{{buildDir}}/release -GNinja -DCMAKE_BUILD_TYPE=Release -DDSVEOSCOSIM_BUILD_TESTS=ON
    cmake --build ./{{buildDir}}/release --config Release

build-all: build build-release

benchmark:
    ./{{buildDir}}/release/tests/benchmark/DsVeosCoSimBenchmark

[windows]
clean:
    Remove-Item -Path ./{{buildDir}} -Recurse -Force -ErrorAction SilentlyContinue

[linux]
clean:
    rm -rf ./{{buildDir}} >/dev/null

test: build
    ./{{buildDir}}/debug/tests/unit/DsVeosCoSimTest

test-release: build-release
    ./{{buildDir}}/release/tests/unit/DsVeosCoSimTest

test-all: test test-release

test-client:
    ./{{buildDir}}/release/tests/TestClient/TestClient

test-server:
    ./{{buildDir}}/release/tests/TestServer/TestServer

performance-client:
    ./{{buildDir}}/release/tests/PerformanceTestClient/PerformanceTestClient

performance-server:
    ./{{buildDir}}/release/tests/PerformanceTestServer/PerformanceTestServer
