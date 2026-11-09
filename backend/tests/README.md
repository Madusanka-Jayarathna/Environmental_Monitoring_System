How to run tests

Option A: Run inside the running Docker container
    cd backend
    docker composer up -d --build api
    docker exec -it env_mon_api pytest -v

Option B: Run directly from host
    cd backend
    DATABASE_URL="postgresql://env_user:env_password@localhost:5432/env_monitoring" MQTT_BROKER_HOST="localhost" PYTHONPATH=. pytest -v

Option C: via Script
    ./run_tests.sh



Code Testing (no need for running)
    Test A  : Start containers & Check status
        docker compose up -d --build
        docker compose ps

    Test B  : Verify MQTT pub/sbu
        (terminal 1) docker exec -it env_mon_mosquitto mosquitto_sub -h localhost -t test -v     
        (terminal 2) docker exec -it env_mon_mosquitto mosquitto_pub -h localhost -t test -m hi

    Test C  : Verfy PostgreSQL Connection & Scheme
        docker exec -it env_mon_postgres psql -U env_user -d env_monitoring
        \dt
        \d readings
        \d events
        \q

    Test D  : FastAPI endpoint
        curl http://localhost:8000/health

    Test E  : Verify Ingestion
        (terminal 1) docker compose logs -f api
        (terminal 2) docker exec -it env_mon_mosquitto mosquitto_pub -h localhost -t "farm/node1/readings" -q 1 -m '{"node":"node1","seq":1,"temp":26.4,"hum":58.2,"pressure":1014.0,"moisture":42.5,"anomaly":false,"score":0.12}'
        (terminal 2) docker exec -it env_mon_postgres psql -U env_user -d env_monitoring -c "SELECT * FROM readings;"
        (terminal 2) docker exec -it env_mon_mosquitto mosquitto_pub -h localhost -t "farm/node1/readings" -q 1 -m '{"node":"node1","seq":1,"temp":26.4,"hum":58.2,"pressure":1014.0,"moisture":42.5,"anomaly":false,"score":0.12}'
        (terminal 2) docker exec -it env_mon_postgres psql -U env_user -d env_monitoring -c "SELECT COUNT(*) FROM readings WHERE node='node1' AND seq=1;"






