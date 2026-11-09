#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

echo "=== 1. Ensuring Docker services are up ==="
docker compose up -d

echo "=== 2. Running Unit Tests (B3 models & parser - no I/O) ==="
docker exec env_mon_api pytest tests/unitTests

echo "=== 3. Running API Tests (B2 FastAPI endpoints) ==="
docker exec env_mon_api pytest tests/api

echo "=== 4. Running Integration Tests (B1+B3 live Mosquitto -> Postgres) ==="
docker exec env_mon_api pytest tests/integrationTests

echo "=== All backend tests (B1-B3) passed! ==="