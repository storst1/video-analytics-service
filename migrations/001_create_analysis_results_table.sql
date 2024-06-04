CREATE TABLE IF NOT EXISTS analysis_results (
    id VARCHAR(255) PRIMARY KEY,
    result JSONB,
    video_status VARCHAR(255)
);
