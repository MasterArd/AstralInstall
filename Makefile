PROJECT_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BACKEND_DIR := $(PROJECT_ROOT)/src/Backend
FRONTEND_DIR := $(PROJECT_ROOT)/src/Frontend
BUILD_DIR ?= $(PROJECT_ROOT)/build
BACKEND_BUILD_DIR ?= $(BUILD_DIR)/backend
FRONTEND_BUILD_DIR ?= $(BUILD_DIR)/frontend

CMAKE ?= cmake
GO ?= go
ZIG ?= zig
BUILD_TYPE ?= Release
QT6_DIR ?=

NATIVE_LIBRARY := $(BACKEND_DIR)/Common/libformat.a
NATIVE_OBJECT := $(BACKEND_DIR)/Common/libformat.a.o
NATIVE_SOURCE := $(BACKEND_DIR)/Common/Convert.zig
BACKEND_BINARY := $(BACKEND_BUILD_DIR)/Backend
FRONTEND_BINARY := $(FRONTEND_BUILD_DIR)/MyAppFrontend

ifeq ($(OS),Windows_NT)
BACKEND_BINARY := $(BACKEND_BINARY).exe
FRONTEND_BINARY := $(FRONTEND_BINARY).exe
endif

ifeq ($(strip $(QT6_DIR)),)
QT6_OPTION :=
else
QT6_OPTION := -DQt6_DIR=$(QT6_DIR)
endif

.PHONY: all configure backend frontend run backend-run clean help

all: frontend

configure:
	$(CMAKE) -S "$(FRONTEND_DIR)" -B "$(FRONTEND_BUILD_DIR)" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(QT6_OPTION)

$(NATIVE_LIBRARY): $(NATIVE_SOURCE)
	$(ZIG) build-lib -static -O ReleaseFast -femit-bin="$@" "$<"

backend: $(BACKEND_BINARY)

$(BACKEND_BINARY): $(NATIVE_LIBRARY)
	$(CMAKE) -E make_directory "$(BACKEND_BUILD_DIR)"
	$(CMAKE) -E env CGO_ENABLED=1 "$(GO)" -C "$(BACKEND_DIR)" build -o "$@" .

frontend: backend configure
	$(CMAKE) --build "$(FRONTEND_BUILD_DIR)" --config $(BUILD_TYPE)

run: frontend
	$(CMAKE) -E chdir "$(FRONTEND_BUILD_DIR)" "$(FRONTEND_BINARY)"

backend-run: backend
	"$(BACKEND_BINARY)" --debug

clean:
	$(CMAKE) -E rm -rf "$(BUILD_DIR)"
	$(CMAKE) -E rm -f "$(NATIVE_LIBRARY)" "$(NATIVE_OBJECT)"

help:
	@echo "Targets:"
	@echo "  all         Build backend and frontend (default)"
	@echo "  backend     Build the cgo backend and Zig native library"
	@echo "  frontend    Configure and build the Qt frontend"
	@echo "  run         Build and launch the frontend"
	@echo "  backend-run Build and run backend self-tests"
	@echo "  clean       Remove generated build files"
	@echo ""
	@echo "Options: BUILD_TYPE=Debug|Release QT6_DIR=/path/to/Qt6"