FROM debian:bullseye

# Install required libraries if any (for example if you used cJSON etc.)
RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy all project files
COPY . .

# Set execute permissions
RUN chmod +x server.out

# Expose port (must match config.json)
EXPOSE 8888

# Start the server
CMD ["./server.out"]

