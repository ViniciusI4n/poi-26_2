CREATE TABLE links (
    id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code       VARCHAR(16)   NOT NULL UNIQUE,
    url        VARCHAR(2048) NOT NULL,
    clicks     BIGINT        NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ   NOT NULL DEFAULT now()
);

CREATE INDEX idx_links_created_at ON links (created_at DESC);
