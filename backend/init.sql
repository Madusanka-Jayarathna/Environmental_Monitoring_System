-- Telemetry readings table (composite PK gives free deduplication on replay)[span_15](start_span)[span_15](end_span)
CREATE TABLE IF NOT EXISTS readings (
    node        TEXT             NOT NULL,
    seq         BIGINT           NOT NULL,
    ts          TIMESTAMPTZ      NOT NULL DEFAULT NOW(),
    temp        REAL             NOT NULL,
    hum         REAL             NOT NULL,
    pressure    REAL             NOT NULL,
    moisture    REAL             NOT NULL,
    anomaly     BOOLEAN          NOT NULL DEFAULT FALSE,
    score       REAL             NOT NULL DEFAULT 0.0,
    PRIMARY KEY (node, seq)
);

-- Index to speed up time-range and latest-sequence queries for REST API (B4)[span_16](start_span)[span_16](end_span)
CREATE INDEX IF NOT EXISTS idx_readings_node_ts ON readings (node, ts DESC);

-- Discrete anomaly / system events table[span_17](start_span)[span_17](end_span)
CREATE TABLE IF NOT EXISTS events (
    id          BIGSERIAL        PRIMARY KEY,
    node        TEXT             NOT NULL,
    ts          TIMESTAMPTZ      NOT NULL DEFAULT NOW(),
    type        TEXT             NOT NULL,
    detail      TEXT
);

CREATE INDEX IF NOT EXISTS idx_events_node_ts ON events (node, ts DESC);
